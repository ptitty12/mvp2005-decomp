// Online play on the page (E5): the lobby (create a game, share the invite
// link, join one), WebRTC signaling through the server's WebSocket, and the
// data channel to the other player. The channel carries the engine's netplay
// datagrams (engine/net/protocol.h) between this page and the Worker; the
// lockstep itself runs inside the engine. Nothing game-derived is sent: only
// controller inputs, acknowledgements and state hashes.
//
// Only shown when the page is served by the netplay server (engine/server,
// which answers /config.json); the local dev server has no lobby.
//
// URL parameters (tests, invite links):
//   room=ID         join this game (the invite link)
//   online=create   create a game at once
//   delay=N|auto    the input delay to create the game with (frames; default auto: measured, Q5)
//   netsim=L,J,P    tests only: delay this page's outgoing datagrams by L ms plus
//                   up to J ms of jitter, and drop a fraction P of them

import type { ClientMsg, IceServer, ServerMsg, SignalData } from '../../server/src/lobby_protocol.js';
import { chooseDelay, lowerDelay, type DelayChoice } from './delay.js';
import type { PaceReport } from './pacer.js';
import type { NetOptions, NetStatus, PaceState } from './protocol.js';

export interface OnlineState {
  available: boolean;
  room?: string;
  player?: 1 | 2;
  invite?: string;
  delay?: number;
  peer: boolean;
  channel: string;     // the data channel's readyState, or 'none'
  route?: string;      // 'direct' or 'relay' once connected
  // The WebRTC transport's own counters (DTLS and SCTP included, IP/UDP not).
  transport?: { bytesSent: number; bytesReceived: number; packetsSent: number; packetsReceived: number; at: number };
  controller?: { connected: boolean; name?: string; ready: boolean };  // this player's controller (Q3)
  peerController?: { connected: boolean; ready: boolean };  // the other player's, as they announced it (Q3)
  link?: DelayChoice & { pings: number };  // Q5: the link as measured before an Auto game, and the delay chosen
  pace?: PaceState;    // Q5: the pace, as this page's Worker reports it
  peerStats?: Record<string, unknown>;  // Q5: the other player's statistics (netstats.ts), as they sent them
  status?: NetStatus;  // the engine's session, as the Worker reports it
  windows?: string;
  diagnostic?: string;
  error?: string;
}

const HASH_EVERY = 60;
// Q5: the link probe before an Auto game: this many pings, this far apart, and how long to wait for the last.
const PINGS = 20, PING_ROUNDS = 2, PING_MS = 50, PING_WAIT_MS = 600;

export class Online {
  readonly state: OnlineState = { available: false, peer: false, channel: 'none' };
  private ws: WebSocket | null = null;
  private ice: IceServer[] = [];
  private pc: RTCPeerConnection | null = null;
  private dc: RTCDataChannel | null = null;
  private pendingCandidates: RTCIceCandidateInit[] = [];
  private readonly el: HTMLElement;
  private readonly statusEl: HTMLElement;
  private readonly padEl: HTMLElement;
  private readonly peerPadEl: HTMLElement;
  private padTimer = 0;
  private readonly inviteEl: HTMLInputElement;
  private readonly createBtn: HTMLButtonElement;
  private readonly delayEl: HTMLSelectElement;
  private openWaiters: ((delay?: number) => void)[] = [];
  private chosenDelay: number | undefined;
  private ready = false;
  private pings = new Map<number, number>();  // Q5: ping id -> when sent
  private rtts: number[][] = [];  // per round (Q5b)
  // A simulated slow network for tests (?netsim=latency,jitter,loss).
  private readonly sim: { latency: number; jitter: number; loss: number } | null;

  /** Datagrams from the other player (to the Worker). */
  onDatagram: (data: ArrayBuffer) => void = () => {};
  /** Q5: the other player's pace report (to the Worker). */
  onPace: (report: PaceReport) => void = () => {};
  /** Q5: the other player's statistics (to the overlay). */
  onPeerStats: (stats: Record<string, unknown>) => void = () => {};
  /** Something changed (the page refreshes its own status line). */
  onChange: () => void = () => {};

  constructor(container: HTMLElement, private readonly params: URLSearchParams) {
    const sim = params.get('netsim');
    const [latency = 0, jitter = 0, loss = 0] = (sim ?? '').split(',').map(Number);
    this.sim = sim ? { latency, jitter, loss } : null;
    this.el = container;
    this.el.replaceChildren();
    const h = document.createElement('h2');
    h.textContent = 'Play online';
    const row = document.createElement('div');
    row.className = 'online-row';
    this.createBtn = document.createElement('button');
    this.createBtn.type = 'button';
    this.createBtn.textContent = 'Create a game';
    this.delayEl = document.createElement('select');
    this.delayEl.title = 'Input delay in frames. Auto measures the connection when the game starts and picks the ' +
                         'smallest delay that keeps the game smooth; or choose one (3 suits a round trip up to about 90 ms).';
    for (const d of [0, 2, 3, 4, 5, 6]) {
      const o = document.createElement('option');
      o.value = String(d);
      o.textContent = d ? `delay ${d}` : 'delay: auto';
      this.delayEl.append(o);
    }
    const asked = params.get('delay');
    this.delayEl.value = asked && asked !== 'auto' && [2, 3, 4, 5, 6].includes(Number(asked)) ? asked : '0';
    this.inviteEl = document.createElement('input');
    this.inviteEl.readOnly = true;
    this.inviteEl.placeholder = 'invite link';
    this.inviteEl.className = 'invite';
    const copy = document.createElement('button');
    copy.type = 'button';
    copy.textContent = 'Copy link';
    copy.addEventListener('click', () => {
      if (this.inviteEl.value) navigator.clipboard?.writeText(this.inviteEl.value).catch(() => {});
    });
    row.append(this.createBtn, this.delayEl, this.inviteEl, copy);
    this.statusEl = document.createElement('p');
    this.statusEl.className = 'online-status';
    this.statusEl.setAttribute('role', 'status');
    this.padEl = document.createElement('p');
    this.padEl.className = 'online-pad';
    this.padEl.id = 'online-pad';
    this.peerPadEl = document.createElement('p');
    this.peerPadEl.className = 'online-pad';
    this.peerPadEl.id = 'online-peer-pad';
    this.peerPadEl.hidden = true;
    this.el.append(h, row, this.statusEl, this.padEl, this.peerPadEl);
    this.createBtn.addEventListener('click', () => this.create());
  }

  /** Shows the panel when the netplay server is there; joins or creates per the URL. */
  async init(): Promise<void> {
    try {
      const r = await fetch('/config.json', { cache: 'no-store' });
      if (!r.ok) return;
    } catch {
      return;
    }
    this.state.available = true;
    this.el.hidden = false;
    this.say('Create a game and send the link, or open the link you got.');
    const room = this.params.get('room');
    if (room) this.connect({ t: 'join', room });
    else if (this.params.get('online') === 'create') this.create();
  }

  /**
   * The lobby's controller readiness (Q3): whether this player's controller is connected and used.
   * Display only: the pad reaches the game through the input queue like any other input.
   */
  setController(c: { connected: boolean; name?: string; ready: boolean }): void {
    const player = this.state.player;
    const where = player ? ` It plays as player ${player}.` : '';
    this.padEl.textContent = !c.connected
      ? 'Your input: keyboard only. To use a controller, connect it and press a button on it.'
      : c.ready ? `Your controller: ${c.name} is ready.${where}`
                : `Your controller: ${c.name} is connected. Press a button on it to use it.${where}`;
    this.padEl.dataset.state = !c.connected ? 'keyboard' : c.ready ? 'ready' : 'idle';
    const changed = this.state.controller?.connected !== c.connected || this.state.controller?.ready !== c.ready;
    this.state.controller = { connected: c.connected, name: c.name, ready: c.ready };
    if (changed) this.announcePad();
  }

  // Whether this player has a controller in use, told to the other player over the data channel as text
  // (the engine's datagrams are binary and never see it). Display only; sent on change and every few seconds,
  // since the channel drops packets.
  private announcePad(): void {
    const c = this.state.controller;
    if (c && this.dc?.readyState === 'open') this.dc.send(JSON.stringify({ mvp: 'pad', v: 1, connected: c.connected, ready: c.ready }));
  }

  // Text messages to the other player (Q3, Q5): small JSON objects on the data channel next to the engine's
  // binary datagrams, which the engine never sees. Display and host timing only.
  sendText(m: Record<string, unknown>): void {
    const text = JSON.stringify(m);
    const sim = this.sim;
    if (!sim) {
      if (this.dc?.readyState === 'open') this.dc.send(text);
      return;
    }
    if (Math.random() < sim.loss) return;
    setTimeout(() => {
      if (this.dc?.readyState === 'open') this.dc.send(text);
    }, sim.latency + Math.random() * sim.jitter);
  }

  /** Q5: this page's pace report (from its Worker), for the other player. */
  sendPace(report: PaceReport, state: PaceState): void {
    this.state.pace = state;
    this.sendText({ mvp: 'pace', ...report });
  }

  /** Q5: this page's statistics (netstats.ts), for the other player's overlay. */
  sendStats(stats: Record<string, unknown>): void {
    this.sendText({ mvp: 'stats', v: 1, ...stats });
  }

  private onPeerText(text: string): void {
    let m: { mvp?: unknown; v?: unknown; connected?: unknown; ready?: unknown; id?: unknown };
    try {
      m = JSON.parse(text);
    } catch {
      return;
    }
    if (m?.v !== 1) return;
    if (m.mvp === 'ping' && typeof m.id === 'number') {  // Q5: the other page measures the link
      this.sendText({ mvp: 'pong', v: 1, id: m.id });
      return;
    }
    if (m.mvp === 'pong' && typeof m.id === 'number') {
      const at = this.pings.get(m.id);
      if (at !== undefined) {
        this.pings.delete(m.id);
        (this.rtts[Math.floor(m.id / PINGS)] ??= []).push(performance.now() - at);
      }
      return;
    }
    if (m.mvp === 'pace') {
      this.onPace(m as unknown as PaceReport);
      return;
    }
    if (m.mvp === 'stats') {
      this.state.peerStats = m as Record<string, unknown>;
      this.onPeerStats(this.state.peerStats);
      return;
    }
    if (m.mvp !== 'pad' || typeof m.connected !== 'boolean' || typeof m.ready !== 'boolean') return;
    this.state.peerController = { connected: m.connected, ready: m.ready };
    const who = this.state.player ? `Player ${this.state.player === 1 ? 2 : 1}` : 'The other player';
    this.peerPadEl.hidden = false;
    this.peerPadEl.textContent = m.ready ? `${who}'s controller: ready.`
      : m.connected ? `${who}'s controller: connected, not used yet.` : `${who} is on the keyboard.`;
    this.peerPadEl.dataset.state = m.ready ? 'ready' : m.connected ? 'idle' : 'keyboard';
  }

  /** The netplay options for the Worker, once this page is in a room. */
  netOptions(): NetOptions | undefined {
    if (!this.state.player) return undefined;
    return { player: this.state.player, delay: this.state.delay ?? 3, hashEvery: HASH_EVERY };
  }

  /**
   * Resolves when the data channel is open and the game may start, with the input delay this page proposes
   * when the game is Auto (Q5: measured on the link first), else undefined (the room's delay).
   */
  whenOpen(): Promise<number | undefined> {
    if (this.ready) return Promise.resolve(this.chosenDelay);
    return new Promise((resolve) => this.openWaiters.push(resolve));
  }

  // Q5: an Auto game: about a second of pings over the data channel (the other page answers each at once),
  // then the delay for the round trips measured (delay.ts). Without answers it takes 3.
  private async measureLink(): Promise<number> {
    for (let k = 0; k < PINGS * PING_ROUNDS; ++k) {
      this.pings.set(k, performance.now());
      this.sendText({ mvp: 'ping', v: 1, id: k });
      await new Promise((r) => setTimeout(r, PING_MS));
    }
    await new Promise((r) => setTimeout(r, PING_WAIT_MS));
    // Q5b: the lower of the two rounds' delays (delay.ts): a moment's stall on either page
    // shows in one round, the link's own latency and jitter in both.
    const choice = lowerDelay(Array.from({ length: PING_ROUNDS }, (_, r) => chooseDelay(this.rtts[r] ?? [])));
    if (!choice) {
      this.say('Could not measure the connection; using input delay 3.');
      return 3;
    }
    this.state.link = { ...choice, pings: PINGS * PING_ROUNDS };
    this.say(`Measured the connection: round trip ${choice.rttMs.toFixed(0)} ms, jitter ${choice.jitterMs.toFixed(0)} ms: ` +
             `input delay ${choice.delay}.`);
    return choice.delay;
  }

  private channelReady(): void {
    if (this.ready || this.measuring) return;
    const done = (delay?: number) => {
      this.ready = true;
      this.chosenDelay = delay;
      for (const w of this.openWaiters.splice(0)) w(delay);
    };
    if (this.state.delay === 0) {
      this.measuring = true;
      this.say('Connected. Measuring the connection...');
      this.measureLink().then(done, () => done(3));
    } else {
      done(undefined);
    }
  }
  private measuring = false;

  /** A datagram from the Worker to the other player. */
  send(data: ArrayBuffer): void {
    const sim = this.sim;
    if (!sim) {
      if (this.dc?.readyState === 'open') this.dc.send(data);
      return;
    }
    if (Math.random() < sim.loss) return;
    setTimeout(() => {
      if (this.dc?.readyState === 'open') this.dc.send(data);
    }, sim.latency + Math.random() * sim.jitter);
  }


  /** The engine's session status (from the Worker). */
  engineStatus(status: NetStatus, windows?: string, diagnostic?: string): void {
    this.state.status = status;
    if (windows !== undefined) this.state.windows = windows;
    if (diagnostic !== undefined) this.state.diagnostic = diagnostic;
    const s = status;
    if (s.state === 'running') {
      // Q5: below full speed, which computer sets the pace.
      const p = this.state.pace, other = s.player === 1 ? 2 : 1;
      const pace = p && p.rate < 0.995
        ? ` Both games run at ${p.rate.toFixed(2)}x speed to match ${p.peerProp !== null && p.peerProp < p.prop
            ? `player ${other}'s computer` : 'this computer'}.` : '';
      this.say(`Playing online as player ${s.player} (delay ${s.delay}, ${this.state.route ?? 'connected'}): ` +
               `frame ${s.frame}, round trip ${(s.rttRecent ?? s.rttAvg).toFixed(0)} ms, waited ${s.stallMs} ms, ` +
               `${s.hashesCompared} sync checks passed.${pace}`);
    } else if (s.state === 'handshake') {
      this.say('Checking that both players have the same game and settings...');
    } else {
      this.say(s.state === 'desynced'
        ? `The two games went out of sync, so the game stopped. ${s.reason}`
        : `Online game over: ${s.reason}`, true);
      if (s.state === 'desynced' && diagnostic) this.offerDiagnostic(diagnostic);
    }
  }

  // The desync diagnostic as a text file to save (numbers only, no game data;
  // it stays on this computer unless the player chooses to share it).
  private offerDiagnostic(text: string): void {
    if (this.el.querySelector('a.diag')) return;
    const a = document.createElement('a');
    a.className = 'diag';
    a.href = URL.createObjectURL(new Blob([text], { type: 'text/plain' }));
    a.download = `mvp-netplay-diagnostic-player${this.state.player ?? 0}.txt`;
    a.textContent = "Save the diagnostic (compare it with the other player's to see what differed)";
    this.el.append(a);
  }

  private say(text: string, error = false): void {
    this.statusEl.textContent = text;
    this.statusEl.classList.toggle('err', error);
    this.onChange();
  }

  // Q5: delay 0 is Auto (the server keeps the room's number, 0 to 10; both pages then measure the link).
  private create(): void {
    this.connect({ t: 'create', delay: Number(this.delayEl.value) || 0 });
  }

  private connect(first: ClientMsg): void {
    if (this.ws) {
      this.ws.send(JSON.stringify(first));
      return;
    }
    this.createBtn.disabled = true;
    const ws = new WebSocket(`${location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.host}/ws`);
    this.ws = ws;
    ws.addEventListener('open', () => ws.send(JSON.stringify(first)));
    ws.addEventListener('message', (ev) => this.onServer(JSON.parse(String(ev.data)) as ServerMsg));
    ws.addEventListener('close', () => {
      this.ws = null;
      if (this.state.channel !== 'open') {
        this.createBtn.disabled = false;
        this.say('Lost the connection to the lobby.', true);
      }
    });
  }

  private signal(data: SignalData): void {
    this.ws?.send(JSON.stringify({ t: 'signal', data } satisfies ClientMsg));
  }

  private onServer(m: ServerMsg): void {
    switch (m.t) {
      case 'welcome':
        this.ice = m.iceServers;
        break;
      case 'room':
        this.state.room = m.room;
        this.state.player = m.player;
        this.state.invite = m.invite;
        this.state.delay = m.delay;
        this.state.peer = m.peer;
        this.inviteEl.value = m.invite;
        this.delayEl.value = [0, 2, 3, 4, 5, 6].includes(m.delay) ? String(m.delay) : '0';
        this.delayEl.disabled = true;
        this.say(m.peer ? 'The other player is here. Connecting...' :
                 'Game created. Send the invite link to the other player.');
        if (m.peer) this.startPeer();
        break;
      case 'peer':
        this.state.peer = m.present;
        if (m.present) {
          this.say('The other player joined. Connecting...');
          this.startPeer();
        } else if (this.state.channel !== 'open') {
          this.say('The other player left the lobby.', true);
          this.resetPeer();
        }
        break;
      case 'signal':
        this.onSignal(m.data).catch((e: unknown) => this.say(`Connection failed: ${String(e)}`, true));
        break;
      case 'error':
        this.state.error = m.message;
        this.createBtn.disabled = false;
        this.say(m.message, true);
        break;
      case 'rooms':
        break;
    }
  }

  private resetPeer(): void {
    this.dc?.close();
    this.pc?.close();
    this.dc = null;
    this.pc = null;
    this.state.channel = 'none';
    this.pendingCandidates = [];
  }

  // Both sides create the same negotiated channel; player 1 makes the offer.
  private startPeer(): void {
    if (this.pc) return;
    const pc = new RTCPeerConnection({ iceServers: this.ice });
    this.pc = pc;
    const dc = pc.createDataChannel('mvp', { negotiated: true, id: 0, ordered: false, maxRetransmits: 0 });
    dc.binaryType = 'arraybuffer';
    this.dc = dc;
    this.state.channel = dc.readyState;
    dc.addEventListener('open', () => {
      this.state.channel = 'open';
      this.say('Connected to the other player.');
      this.findRoute().catch(() => {});
      setInterval(() => this.findRoute().catch(() => {}), 2000);
      this.announcePad();
      window.clearInterval(this.padTimer);
      this.padTimer = window.setInterval(() => this.announcePad(), 3000);
      this.channelReady();
    });
    dc.addEventListener('close', () => {
      this.state.channel = 'closed';
      this.onChange();
    });
    dc.addEventListener('message', (ev: MessageEvent<ArrayBuffer | string>) => {
      if (typeof ev.data === 'string') this.onPeerText(ev.data);  // the other player's controller line (Q3)
      else this.onDatagram(ev.data);
    });
    pc.addEventListener('icecandidate', (ev) => {
      const c = ev.candidate;
      this.signal({ kind: 'candidate', candidate: c ? { candidate: c.candidate, sdpMid: c.sdpMid, sdpMLineIndex: c.sdpMLineIndex } : null });
    });
    pc.addEventListener('connectionstatechange', () => {
      if (pc.connectionState === 'failed') this.say('Could not connect to the other player (no route, even through the relay).', true);
    });
    if (this.state.player === 1) {
      (async () => {
        await pc.setLocalDescription(await pc.createOffer());
        this.signal({ kind: 'offer', sdp: pc.localDescription!.sdp });
      })().catch((e: unknown) => this.say(`Connection failed: ${String(e)}`, true));
    }
  }

  private async onSignal(d: SignalData): Promise<void> {
    if (!this.pc) this.startPeer();
    const pc = this.pc!;
    if (d.kind === 'candidate') {
      if (!d.candidate) return;
      if (pc.remoteDescription) await pc.addIceCandidate(d.candidate);
      else this.pendingCandidates.push(d.candidate);
      return;
    }
    await pc.setRemoteDescription({ type: d.kind, sdp: d.sdp });
    for (const c of this.pendingCandidates.splice(0)) await pc.addIceCandidate(c);
    if (d.kind === 'offer') {
      await pc.setLocalDescription(await pc.createAnswer());
      this.signal({ kind: 'answer', sdp: pc.localDescription!.sdp });
    }
  }

  // Whether the connection is direct or goes through the TURN relay.
  private async findRoute(): Promise<void> {
    if (!this.pc) return;
    const stats = await this.pc.getStats();
    stats.forEach((s: { type: string; bytesSent?: number; bytesReceived?: number; packetsSent?: number; packetsReceived?: number }) => {
      if (s.type === 'transport' && s.bytesSent !== undefined)
        this.state.transport = { bytesSent: s.bytesSent, bytesReceived: s.bytesReceived ?? 0, packetsSent: s.packetsSent ?? 0,
                                 packetsReceived: s.packetsReceived ?? 0, at: performance.now() };
    });
    let pair: { localCandidateId?: string } | undefined;
    stats.forEach((s: { type: string; state?: string; nominated?: boolean; selected?: boolean; localCandidateId?: string }) => {
      if (s.type === 'candidate-pair' && s.state === 'succeeded' && (s.nominated || s.selected)) pair = s;
    });
    if (!pair?.localCandidateId) return;
    const local = stats.get(pair.localCandidateId) as { candidateType?: string } | undefined;
    this.state.route = local?.candidateType === 'relay' ? 'relay' : 'direct';
    this.onChange();
  }
}
