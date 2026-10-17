# Netplay (E5)

How two players on different networks play each other: each runs the game in
their own browser, the two engines exchange controller inputs in lockstep over
a WebRTC data channel, and the owner's VPS serves the site behind a password,
runs the lobby and signaling, and relays traffic for players whose networks
need it. Design: `docs/ARCHITECTURE.md` §8. Working log and evidence:
`docs/engine/E5_LOG.md`.

```
 player 1's browser                                         player 2's browser
 page: lobby UI, RTCPeerConnection  <--- WebRTC data channel --->  page
   | datagrams (postMessage)              (direct, or via TURN)      |
 Worker: engine (wasm) + engine/net session                       Worker
                         \                                    /
                          \--- WebSocket: lobby + signaling ---/
                                  owner's VPS (engine/server):
                                  Caddy (TLS) -> mvp-server (Node), coturn (TURN)
```

Only controller inputs, acknowledgements and state hashes cross the network.
Nothing game-derived does: each browser reads its own disc (or, on the owner's
private deployment only, the owner's image, downloaded once from the
password-protected disc route into that player's own browser storage: P1,
`docs/engine/WEB.md`), translates the game itself, and keeps any diagnostic dump
local. On the deployment a page opens the lobby only once its copy of the disc is
complete, so a player still downloading never makes the other one wait past the
session's 10 s timeout.

## 1. Lockstep

- **Frames** are the game's frame polls (the entry of fn_8014865C, E0c-IN1),
  numbered from power-on. Both browsers boot the game identically and play from
  power-on: menus, intro and all.
- **Ports.** Player 1 drives controller port 1, player 2 port 2; ports 3 and 4 are
  empty. Both player ports are connected from power-on (the boot input is
  `neutral_input(0x3)` on both sides). Each browser's own controller (or keyboard)
  drives *its* player's port: the page sends one pad, as its port 0, and the engine's
  session puts it on port 1 or 2 by the room role; the page's other ports are held
  disconnected online (`docs/engine/Q3_LOG.md` section 5). The lobby shows whether
  your controller is ready and, over the data channel as small text messages the
  engine never sees, whether the other player's is.
- **Input delay D.** The local pad sampled when the game reaches frame F becomes
  the input of frame F + D. Frames 0 .. D-1 have neutral input on both ports. Each
  player proposes D and the session uses the larger proposal. The lobby's choice
  (the creator's, shown to the joiner) is **Auto** by default (Q5): once the data
  channel is open, each page sends two rounds of 20 small text pings (50 ms apart,
  answered at once by the other page) and picks D from each round's round trips
  (`web/src/delay.ts`): D x 18 ms must cover half the median round trip, about 10
  ms for the Worker hop and a field's run, and the one-way jitter (half the spread
  from the fastest to the 90th-percentile round trip), with D between 2 and 6. It
  proposes the lower of the two rounds' D (Q5b: a page busy for a moment while it
  measured once chose 6 on a local link; a stall shows in one round, a real link
  in both). A local link gives 2, E5's simulated 40-50 ms one way gives 4. A
  manual choice (2 to 6) overrides it.
  D is agreed before the game starts (HELLO), so how it was chosen cannot change
  what the guest computes.
- **The gate.** A frame runs only when both ports' inputs for it are known. The
  engine's run loop is unchanged except for this check (`net/driver.h`):
  `rt::run()` returns at every VI field (host yields, `runtime/host_yield.h`), and
  the host loop calls it only while `NetplayDriver::may_run()` is true. A frame poll
  whose input is missing stops the run right there, before it consumes anything
  (`NetplayDriver::before_poll`, `YieldingHooks::stop_before`, E5c): `rt::run()`
  returns, the host loop waits, and the guest makes the same poll again when the
  input is there, exactly as if it had never stopped. After each poll whose
  successor's input is still missing, the driver also asks the run to yield at its
  next stopping safe point, so the host loop pumps the network meanwhile. A poll that
  cannot stop (inside an interrupt handler) and has no input would stop the game
  ("gate missed") rather than guess; it has never happened. (Before E5c the gate
  relied on that yield alone, which waits for a safe point where an event is due; at
  the match's loading frames come faster than events, and the game stopped there.)
- **Sampling.** With D > 0 the local input of frame F + D is sampled when frame F
  can run: a player who waits for the other samples the pad when the wait ends, so
  both players' input latency is D frames. With D = 0 the input is sampled before
  waiting (otherwise both would wait forever).
- **Bounded lead.** A peer at frame F has sent its inputs up to F + D - 1, so neither
  peer can get more than D frames ahead of the other, and they never deadlock.
- **Desync detection.** Each side hashes MEM1 in a striped way (`net/state_hash.h`):
  at frame f it hashes slice `f mod K` of MEM1 (K slices) with the frame number and
  the instruction count, chained into a window hash that closes at `f mod K = K - 1`
  (K = 60: one window per second of play, 400 KB of hashing per frame). Window
  hashes are exchanged and compared; a mismatch stops both games at once. Each side
  can then show (and save locally) a text diagnostic: the session state, the recent
  inputs, the window hashes and the 60 slice hashes of the last window (two players'
  diagnostics show which 400 KB of MEM1 differ). Diagnostics hold numbers only, never
  memory contents, and are never uploaded.
- **Disconnects.** A peer silent for 10 s ends the session ("stopped answering").
  Leaving the page sends a goodbye. A running game cannot be rejoined (that needs
  save states, K5).

Host time decides only *when* datagrams are sent and when a frame becomes ready,
never what a frame's input is: both peers' guests see exactly the same inputs at
the same frame polls, whatever the network does.

### Pacing between unequal machines (Q5)

Lockstep holds the faster machine to the slower one's pace. Before Q5 each page
paced itself to the wall clock at full speed: the slower machine ran flat out and
chased (two-field batches, every other frame shown), and the faster one ran
ahead, waited for the other's input (35 times a second, a third of the time, at
a simulated 1.6x slower partner), ran again: stop and go. Now (`web/src/pacer.ts`,
the Worker's paced loop):

- **Capacity.** Each side measures what a field costs its machine (the Worker's
  busy time per field: the run, its waits for the render thread, the present),
  averaged per 15 fields and smoothed over about 30 fields (the game's scenes
  change their cost within seconds; the pace follows them both ways); its
  capacity is a field's time (16.7 ms) over that cost.
- **One pace.** Each side proposes its capacity times 0.98, in steps of 0.01, at
  most full speed (lowered at once, raised once it has had room for two steps for
  half a second); both run at the lower of the two proposals. The proposals travel four times
  a second as text messages on the data channel (`{"mvp":"pace", ...}`), which
  the page relays to its Worker; the engine never sees them. When the slower
  machine keeps up again its proposal rises and both return to full speed.
- **Even fields.** The paced loop's schedule runs at that pace: one field per
  slot, started at the slot's own time (a timer, not the next animation frame),
  presented as it ends. A wait for the other player's input moves the
  schedule on rather than being caught up; a side that fell behind (a heavy
  field) catches up one field at a time, at most two fields' worth, so no batch
  runs two fields and every frame is shown.
- **Phase.** Each side measures its *input lead* (`net/driver.h`: how long before
  it needed the other's input of a frame that input arrived, the median of the
  last 24 frames; negative when it waited) and reports it with its proposal. The
  side with the smaller lead runs early; each side moves its schedule by half the
  difference (60 % of it a second, at most 3 % of the elapsed time), so both leads
  settle equal: as far from waiting as the delay allows on both sides.
- **Sound.** The Worker tells the AudioWorklet the pace, so Q2's time-stretch
  plays at it from the start instead of finding it from a buffer that runs low.
- **Roles (Q5b).** Even pacing at 98 % of the slower machine's measured
  capacity left that machine idle part of the time, behind a schedule it could
  not quite keep, or ahead of one it could have beaten. Now, once one machine's
  capacity is 3 % below the other's for half a second, it *sets the pace*: its
  schedule runs 15 % past its capacity, so it is always a little behind and runs
  each field as soon as the last is done (still one field at a time, every frame
  shown), as fast as its machine can. The other *follows* it: its schedule runs
  at the capacity the setter reports, it moves its schedule toward equal input
  leads by itself (the setter does not), and it trims its pace by the phase
  difference that remains (an integral term, at most 4 %, held at full speed).
  Player 1 decides the roles from both capacities and reports its own in its
  `pace` message (`role`); player 2 takes the opposite one, so the two always
  agree. Machines within 3 % of each other keep the common pace above; the roles
  last through stretches both run at full speed. `paceTune=roles:0` (a page
  parameter for development) restores Q5's common pace.

All of it is host timing: it decides when fields run, never what they compute.
The two games' guest lines and state hashes stay identical (Q5_LOG.md).

## 2. Wire protocol (version 1)

Datagrams over an unreliable, unordered channel (`RTCDataChannel` with
`ordered: false, maxRetransmits: 0`, negotiated id 0; UDP in native tests). All
integers big-endian. Every datagram starts with a 4-byte header:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 1 | `0x4D` (`M`) |
| 1 | 1 | `0x4E` (`N`) |
| 2 | 1 | protocol major version: 1 |
| 3 | 1 | type: 1 HELLO, 2 INPUT, 3 BYE |

A datagram that does not parse exactly (wrong magic or major version, wrong
length, reserved bits set, an unknown button, a disconnected pad with controls,
hash records out of order) is dropped and counted; it never changes the session.
`engine/net/protocol.{h,cpp}` is the reference implementation.

**HELLO** (handshake), repeated every 100 ms until the peer has seen it:

| Size | Field |
|---:|---|
| 1 | minor version (0) |
| 1 | player: 1 or 2 |
| 1 | flags: bit 0 = "I have received your HELLO"; other bits 0 |
| 1 | proposed input delay D |
| 2 | hash window K in frames (0: no desync check) |
| 2 | reserved, 0 |
| 8 | nonce (random per game instance) |
| 8 | seed proposal (player 1's is used; reserved for engine-side randomness, unused today) |
| 20 | disc identity (SHA-1, §3) |
| 32 | pack-set hash (SHA-256, `docs/MODDING.md` §10) |
| 4 | CLK2: CPU cycles per guest instruction, in thousandths (1,250; E0b-CLK1 builds sent whole cycles, 2, so the two refuse each other) |
| 4 | DVD1: command latency in time-base ticks (40,500) |
| 4 | DVD1: transfer time in time-base ticks per KiB (13,184) |
| 1 + n | engine build id (n <= 64 bytes) |
| 2 + n | canonical settings string (n <= 512 bytes) |

The game starts only if the two HELLOs agree on the build id, disc identity,
pack-set hash, CLK2/DVD1 constants, settings string and K, and the players are 1
and 2. Otherwise both sides stop with the reason (for example "different disc
identities" or "both players chose player 1"). A side is running once it has the
peer's HELLO with flag bit 0 set, or any INPUT from it. A second HELLO with another
nonce means the peer restarted: the session ends.

**INPUT**, sent when a new local input is sampled, and after 20 ms without any send
(retransmission and keepalive):

| Size | Field |
|---:|---|
| 4 | ack_frame: the sender has every peer input before this frame |
| 4 | ack_hash: the sender has every peer hash record before this frame |
| 4 | cur_frame: the sender's next frame poll |
| 4 | send_ms: the sender's clock (wraps) |
| 4 | echo_ms: the latest send_ms received from the peer (0: none yet) |
| 4 | echo_hold_ms: how long the sender held echo_ms before this send |
| 4 | first_frame: frame of the first pad below |
| 1 | pad count n (<= 64; the sender uses at most 32) |
| ... | n pads: 11 bytes each (flags: bit 0 connected; buttons 2; stick x, y; C-stick x, y; trigger L, R; analog A, B: `runtime/input.h` PadState v1), or the single byte `0x80` when equal to the previous pad of this datagram (never for the first) |
| 1 | hash record count m (<= 8) |
| 12 m | records, ascending frames: frame (4), window hash (8) |

The pads are every input the peer has not acknowledged, oldest first, so a lost
datagram costs at most one resend interval. Round-trip time = now - echo_ms -
echo_hold_ms (it includes the time a datagram waits while the guest runs a field).

**BYE**, sent three times when a side stops:

| Size | Field |
|---:|---|
| 1 | reason: 0 quit, 1 desync, 2 rejected, 3 timeout, 4 error |
| 4 | desync: the window that differed; otherwise the sender's next frame |
| 1 + n | short text for the other player (n <= 200) |

**Versioning.** The protocol is a versioned engine API (CLAUDE.md). A change an
older peer cannot parse gets a new major version (and a migration note in
DECISIONS); minor versions may only add what older peers can ignore. Since the
build ids must match anyway, peers always run the same version today.

## 3. Identity

- **Engine build id** (browser): `web-` + the first 12 bytes (hex) of SHA-256 over
  `mvp-web.wasm`, `mvp-web.js` and `symbols.txt` as served (function boundaries are
  safe points, so the symbol map is part of the build). Native tools use their own
  id, so a native peer never matches a browser peer.
- **Disc identity v1**: SHA-1(`"mvp2005-disc-id/1\n"` + boot.bin (0x440 bytes: game id,
  revision) + BI2 (0x2000 bytes) + the FST bytes + the hex SHA-1 of `main.dol`). That
  is everything the boot environment copies into memory plus the code, and the same
  for ISO and CISO dumps of the disc. Other file contents are not hashed (that would
  read the whole 1.4 GB image); a damaged file would show up as a desync.
- **Pack set** (E6): the ordered pack-set hash of `docs/MODDING.md` §10 of the roster
  packs the player picked on the page (the Roster packs panel, `docs/engine/WEB.md`),
  computed by the engine's pack code (`engine/mods`) after it mounted them on the virtual
  disc; without packs, the empty set's hash
  (`f5e1298911db13adf522a1b34d4d4f9291b42fa7a77f63a5f6f07bc322dcc66e`). The disc identity
  above is taken when the disc is checked, before the packs are mounted (mounting rewrites
  the FST), so a pack mismatch is reported as one: "the players chose different roster
  packs (player 1: pack set b9a04e6578e789b4, player 2: f5e1298911db13ad); both must pick
  the same packs in the same order". The same disc and the same packs give the same
  virtual disc on both sides, byte for byte, so the guests stay in lockstep
  (`docs/engine/E6_LOG.md`).
- **Settings**: sorted `key=value` pairs joined by `;`: `aram_size`, `ax_voices`,
  `bus_clock`, `console_type`, `cpu_clock`, `pad` (PadState version), `ports`,
  `rtc_seconds`, `sram_flags`, `sram_language`, `tv_mode` (`net/identity.cpp`).

## 4. Lobby and signaling (JSON over WebSocket, version 1)

The page opens `wss://<site>/ws` (the session cookie and the site's Origin are
required). Types: `engine/server/src/lobby_protocol.d.ts` (shared with the page).

| Direction | Message |
|---|---|
| server -> page | `{t:"welcome", v:1, iceServers:[{urls:[...]}, {urls:[...], username, credential}]}` |
| page -> server | `{t:"create", name?, delay?}`: a new room, the creator is player 1 (Q5: `delay` 0 is Auto, measured by both pages) |
| page -> server | `{t:"join", room}`: join as player 2 (the invite link is `/?room=<id>`) |
| server -> page | `{t:"room", room, player, invite, delay, peer}` |
| server -> page | `{t:"peer", present}`: the other player joined or left the lobby |
| both | `{t:"signal", data}`: offer, answer or ICE candidate, relayed to the other player |
| page -> server | `{t:"list"}` / `{t:"leave"}`; server -> page `{t:"rooms", rooms}`, `{t:"error", code, message}` |

Player 1 makes the WebRTC offer when player 2 is present; both create the same
negotiated data channel. Once the channel is open the lobby is no longer needed.
Besides the engine's binary datagrams, the pages send each other small JSON text
messages on that channel, which the engine never sees: `pad` (controller
readiness, Q3), `ping`/`pong` (the link measured for an Auto delay), `pace` (four
times a second: capacity, proposed pace, input lead; Q5b: the sender's role) and
`stats` (once a second: the overlay's numbers) (Q5).

## 5. The server (`engine/server/`)

TypeScript on Node 24 (D3), one dependency (`ws`), pinned in `package-lock.json`.

| Route | |
|---|---|
| `GET /login`, `POST /login`, `POST /logout` | the password gate: one shared password, an HttpOnly `SameSite=Lax` session cookie (`Secure` behind TLS), in-memory sessions (30 days), failed logins limited per address (10 per 15 min), no open redirects |
| `GET /setup`, `POST /setup` | first-run password setup (below): the form while the server has no password and its window is open (every page request is sent here); 403 once the window has closed; **404 forever once a password exists** |
| `GET /healthz` | `ok`, without a login (monitoring) |
| `GET /config.json` | `{"disc":"/api/disc/image"}` when a disc image is configured, else `{}`; with a disc also `discHash` (the image hash), `discSize` and `discManifest`, or `discPreparing` while the server hashes the image (P1); `discPage` with uploads |
| `GET`/`HEAD /api/disc/image` | the owner's image: `Accept-Ranges: bytes`, single ranges (206, or 416), `Cache-Control: private, no-store` |
| `GET /api/disc/manifest` | the image's 8 MiB pieces' SHA-256 and the image hash (SHA-256 over those digests), for the pages' one-time download (`src/disc_hash.ts`); 503 while hashing, `private, no-store` |
| `GET /api/rooms` | open rooms |
| `/ws` | lobby and signaling (above) |
| anything else | the engine site (static, `Cache-Control: private, no-cache` + ETag) |

Without a session, a page load is sent to the login form and anything else gets
401. Every response has `X-Content-Type-Options`, `Referrer-Policy: no-referrer`
(invite links never leak through referrers), `X-Frame-Options: DENY` and (P2)
`Cross-Origin-Opener-Policy: same-origin`, `Cross-Origin-Embedder-Policy: require-corp`
and `Cross-Origin-Resource-Policy: same-origin`, so the page is cross-origin isolated
and loads the threaded engine (WEB.md section 2); engine pages have a Content-Security-Policy (same origin only, plus `wasm-unsafe-eval` to
compile the translated game). Secrets come only from the environment or from files
outside the repository: `MVP_TURN_SECRET`/`MVP_TURN_SECRET_FILE`, and the site password
(created in the browser on the first run, below, or `MVP_PASSWORD`/`MVP_PASSWORD_FILE`).
Everything else: `MVP_SERVER_CONFIG` (JSON, `deploy/server.json.example`) or
`MVP_*` variables (`src/config.ts`).

### First-run password setup (E5b)

The agents never handle the site password, so a server that has none waits for the
owner to choose it in a browser.

**Where the password comes from**, in this order (the first that exists wins):

1. `MVP_PASSWORD` / `MVP_PASSWORD_FILE` (at least 8 characters): for tests and CI.
   When set, first-run setup is off (`/setup` is 404), the state file is neither read
   nor written, and any stored password is ignored. An empty `MVP_PASSWORD_FILE` is
   an error, never "no password".
2. The hash in the state file (`stateFile` in `server.json`, or `MVP_STATE_FILE`).
3. Neither: **setup mode**. `stateFile` is then required (the server refuses to
   start without a password source).

**Setup mode.** Every page request (`/`, `/login`, anything that is not `/healthz`
or `/setup`) is redirected to `/setup`; API calls and sockets get 503 and 401.
`/setup` is a small form: password and confirmation (at least 12 and at most 128
characters, NFKC-normalized, counted in characters) and a live strength hint (a rough
guide only: the server enforces the length). On a good submission the server hashes
the password, writes the state file, logs the owner in (the usual session cookie)
and leaves setup mode. The password is never logged, never echoed back into a page,
and never written anywhere but as the hash.

**The state file** (`/var/lib/mvp/state.json` on the VPS): JSON, `version: 1`,
`setupOpenedAt` (ms since the epoch: when the server first started without a
password) and, after setup, `password`: `{kdf: "scrypt", N, r, p, keylen, salt, hash,
setAt}` (salt and hash base64). Created on the first start without a password (mode
0600, its directory 0700, where the OS has modes; on Windows the profile's ACLs
apply) and replaced atomically (temporary file, fsync, rename). A file that exists
but cannot be parsed or validated stops the server: it never falls back to "no
password", which would reopen setup.

**Hash.** scrypt (RFC 7914, Node's `crypto.scrypt`): N = 2^17, r = 8, p = 1 (OWASP's
first scrypt recommendation: 128 MiB, about 0.2 s per hash), a 64-byte key, a random
32-byte salt per password, verified in constant time. Each record carries its own
parameters, so the cost can rise later without breaking old records; records with
absurd parameters (a state file edited into a denial of service) are refused. Password
checks and hashes run one at a time (a queue of eight; beyond that, 503) so a burst
cannot exhaust a small VPS's memory. With the environment password, login compares
SHA-256 digests as before.

**Strangers cannot (easily) claim it.**

- *A window.* Setup is open for `setupHours` (default 48; `MVP_SETUP_HOURS`) after the
  server **first** starts without a password. The start is kept in the state file, so
  restarts do not extend it. After it, `/setup` answers 403, pages answer 503 "not set
  up", and the log says so once: `setup window closed ...: the server has no password
  and needs an operator`.
- *Rate limits* on the setup endpoints, per sliding 15 minutes (a refused request is
  not counted): 30 requests and 10 form submissions per client address, and 100
  submissions in all (429 with `Retry-After`). Behind Caddy the address is the
  proxy's last `X-Forwarded-For` entry (`trustProxy`).
- *CSRF.* The form carries a token bound to an HttpOnly `SameSite=Strict` cookie
  (`mvp_setup`, path `/setup`): `<expiry>.<HMAC-SHA256(per-process key, id.expiry)>`,
  valid 30 minutes, stateless. A submission also has to be same-origin: a present
  `Sec-Fetch-Site` must say `same-origin`, a present `Origin` must be the site's own
  (the setup pages send `Referrer-Policy: same-origin`, because under the site-wide
  `no-referrer` browsers send `Origin: null` for form posts). The page's CSP pins its
  one style and one script by hash.
- *Exactly one winner.* Two simultaneous submissions: one is stored, the other gets
  503 or 404.

**Residual risk.** Setup is first come, first served: whoever submits a valid form
first while the window is open owns the password, and with it the site, the lobby and
(on the private deployment) the owner's disc image. The limits and the window make
abuse harder; they do not prove the visitor is the owner. The deploy URL is
unpublished, but the host name appears in the public certificate-transparency logs as
soon as Caddy obtains its certificate, and scanners watch those logs, so assume a
stranger can find the form within hours. Therefore: make the first visit right after
installing; if it shows the **login** form instead of the setup form, someone else
created the password: reset (below) and treat the VPS as exposed; copy the disc image
to the VPS only after the password exists (§6); keep `setupHours` short where the
window can be. The password travels over HTTPS (Caddy); on plain HTTP it would not be
protected, so use setup only behind TLS (`secureCookies`).

**Changing the password later** is an operator action, there is no admin UI: stop
`mvp-server`, delete the state file (`/var/lib/mvp/state.json`), start it. The server
is then in setup mode again with a fresh window (and every session is gone); the owner
makes a new first visit. The same reopens setup after a window that closed unused.
The server reads the file at start, so it must be restarted.

**TURN.** coturn runs in its "TURN REST API" mode (`use-auth-secret`): the server
hands each page credentials derived from the shared secret (username
`<expiry>:mvp`, password base64 HMAC-SHA1), valid for 24 hours. No accounts are
stored, and leaked credentials expire.

Tests: `npm test` in `engine/server` (64 tests, `test/disc.test.ts` for the disc upload, the image hash and the Docker settings: the gate, sessions and limits, the
static site, the disc route, the lobby, signaling, TURN credentials, and first-run
setup: setup mode redirects, form validation and CSRF, the stored hash and its file,
the window, restarts, the environment override, limits), also run by ctest as
`server_node_tests`.

## 6. Deploying on a VPS

**Docker / Dokploy (current):** `deploy/Dockerfile` and `deploy/docker-compose.yml`
(services `mvp` and `coturn`, volume `mvp-data`); the owner's UI-only guide is
`docs/engine/DEPLOY_DOKPLOY.md`. There the disc image is uploaded on the logged-in
Server disc page (`/disc`: chunked, resumable, GV4E69 rev 0 checked server-side by
the DOL's SHA-1, atomic replace), the TURN secret is generated on the first start,
and `RESET_PASSWORD` reopens setup once without a shell. The install script below
is the older SSH path.

**Package.** `node engine/server/tools/package.mjs` (after building `mvp-web` and
`npm ci && npm run build` in `engine/server`) writes
`build/engine/deploy/mvp-netplay/` and `mvp-netplay.tar.gz`: `site/` (the engine:
no game data, the script refuses other file types), `server/` (compiled, with
`ws`), `deploy/` (install script and templates) and `MANIFEST.txt` (SHA-256 of
every file).

**Requirements.** A VPS with Debian 12 or Ubuntu 24.04, a public IPv4 address, root
SSH access, and a domain name (or subdomain) whose A record points to it. Open
ports: 80/tcp and 443/tcp (Caddy, HTTPS), 3478/udp and 3478/tcp (TURN), 49160-49200/udp
(TURN relay ports).

**Install** (an agent with SSH access does this; the owner runs nothing):
1. Copy `mvp-netplay.tar.gz` to the VPS and unpack it.
2. `./deploy/install.sh <domain>` as root. There is no password to pass (the script
   refuses `MVP_PASSWORD`): a fresh install starts in setup mode with **no
   password**. It installs Caddy and coturn (distribution packages) and Node.js
   24.18.0 (the official build, checksum verified), creates the `mvp` user and
   `/opt/mvp/{site,server}`, writes `/etc/mvp/{turn-secret,server.json,server.env}`
   (mode 0640, root:mvp), `/etc/caddy/Caddyfile`, `/etc/turnserver.conf` and the
   `mvp-server` systemd unit (whose `StateDirectory` is `/var/lib/mvp`), opens the
   ports with ufw when present, and starts everything. It ends by saying what the
   first visit will find. Running it again updates the engine and keeps the secrets
   and the password. (An install from before first-run setup keeps its
   `/etc/mvp/password`.)
3. **First visit: create the site password.** The agent tells the owner the address
   and asks them to open it right away (see the residual risk above: the window is
   48 hours, but the host name is findable within hours). `https://<domain>/` goes to
   `/setup`; the owner types the password twice and is logged in. The agent never
   sees the password. To confirm without it: `curl -s -o /dev/null -w '%{http_code}'
   https://<domain>/setup` says 404 (a password exists) and the service log has
   `setup: the site password was created`; the owner confirms they saw the form (a
   login form on the first visit means someone else got there first: reset, below).
4. Only now, for the private deployment, copy the owner's disc image to
   `/srv/mvp/disc/GV4E69.ciso` and run `./deploy/install.sh <domain>` again (it
   adds the image to `server.json`), so the image is never behind a password
   a stranger could have chosen.
5. Check: `https://<domain>/` shows the login form when logged out; after logging
   in, "Create a game" gives an invite link; `curl https://<domain>/healthz` says `ok`.

**Resetting the password** (operator action; the owner runs nothing): `systemctl
stop mvp-server`, delete `/var/lib/mvp/state.json`, `systemctl start mvp-server`;
setup is open again for a fresh window (step 3).

**Updating the engine** later: build, package, copy, run `install.sh` again (both
players must reload the page: the build id changes).

## 7. Measurements (2026-10-14, one machine)

Details and the commands: `docs/engine/E5_LOG.md` §2, §4 and §5.

- **Native, two processes over UDP** (`local_netplay_lockstep`, 1,200 frames, D = 3):
  both peers and the solo reference agree on every frame's input, all 20 window
  hashes and the final MEM1 SHA-1; also under 25% loss, 40 ms latency and 30 ms jitter.
- **Two browsers** through the server (two isolated profiles, login, lobby, invite
  link, direct connection), 1,200 frames: identical guest lines, equal to the native
  build's, and identical window hashes, zero desyncs and zero gate misses, for Chrome
  x2 at D = 2, 3 and 4 and for Chrome + Firefox at D = 3.
- **Input latency** (sampling the pad to the frame that uses it, menus, steady state):
  D frames of the game's pace plus about 1.5 ms, the same for both players:

  | D | local link | 40-50 ms one way, 2% loss (simulated) |
  |---:|---|---|
  | 2 | 38.5-39.2 ms, full speed | 57.8-58.1 ms, **36.8 frames/s**: the delay is shorter than the latency, both games wait |
  | 3 | 56.5-57.7 ms, full speed | 56.8-62.3 ms, full speed |
  | 4 | 74.6-75.4 ms, full speed | 75.6-76.3 ms, full speed, almost no waiting |

  "Full speed" is 54-58 frame polls per second, the game's own rate there (55.4 per
  guest second). Rule of thumb: D x 18 ms must cover the one-way latency plus about
  10 ms. D = 3 suits a round trip up to about 90 ms; longer distances need 4 or more.
- **Bandwidth per direction**: 2.6 KB/s of netplay payload on a local link, about
  4 KB/s at a 45 ms one-way latency (more unacknowledged inputs per datagram);
  5.5-9.5 KB/s through WebRTC's DTLS and SCTP; plus 28 bytes of IP/UDP per datagram
  (50-60 datagrams/s). A TURN relay carries the same.
- **Into the match (E5c, 2026-10-15; E5_LOG §5):** a two-player exhibition game (player
  2 takes the away team at Select Team), D = 3. Natively: two peers equal the solo run
  to frame 5,600 (the first pitches) over a link with 25% loss and 40-70 ms latency,
  and to frame 60,000 on a local link (two full innings and part of the third);
  `local_netplay_lockstep` and `local_netplay_lossy` run into the match's loading. Two
  Chrome pages: identical guest lines, equal to native, at frame 5,600 (also with
  `--netsim 40,10,0.02`), and with the example roster pack on both pages at frame 4,420
  (the Harbor City Lanterns' defensive line-up on both screens); zero desyncs and zero
  gate misses in every run.

## 8. What netplay relies on (DETERMINISM.md §6)

Lockstep is correct only if the same inputs at the same frame polls give the same
guest state on every machine and browser. It relies on:

- **R1** (virtual time base: no host clock in guest state), **R2** (VI retrace
  schedule), **R4** (interrupts at safe points, counted in instructions), **R5**
  (deterministic scheduler), **R6** (EA tick from the virtual clock): the guest's
  timeline is a function of the instruction count only. **R3**: the RNG seeds come
  from the virtual time base, so they are equal on both sides without a handshake
  seed.
- **R7** (DVD completes after a virtual delay) and the disc identity: both sides read
  the same bytes at the same virtual times. The DVD1 constants are in the handshake.
- **R8** (audio never feeds back: E3b's mixer is a read-only observer) and **R11**
  (GX events at defined points; the renderer only reads memory): rendering and audio
  output differ freely between machines.
- **R9**: no memory card is present (every EXI device but the IPL chip is absent), the
  same on both sides. **R10**, **R12**, **R16**: the VM region, the environment
  constants (in the settings string) and the power-on memory image are fixed.
- **R14** (E0c-IN1): input enters only at the frame poll, so only the per-frame input
  stream has to be agreed on. **R15**: exact arithmetic, identical in native code and
  in V8, SpiderMonkey and JavaScriptCore (E2: identical guest lines across browsers).
- E2's host yields are invisible to the guest; netplay's early yields are the same
  mechanism, and so are its stops at a frame poll (the poll is made again, from its
  entry, with the repeated safe point skipped).

## 9. Limits and next steps

- **Rollback** (hiding the delay) needs fast save states (K5); lockstep pays D frames
  of input latency (D x 16.7 ms when a frame is one field).
- **No rejoin** after a disconnect, and no jump into a match from a save state (K5).
- **Pacing:** a player whose browser cannot keep up still slows both games to its
  speed (lockstep); since Q5 both run evenly at that pace (section 1, "Pacing
  between unequal machines"), a little below what the slower machine reaches flat
  out (the 2 % margin, the steps, and fields run one at a time). The page's Stats
  overlay shows both machines and which one sets the pace (WEB.md).
- **Browsers:** Chrome x2 and Chrome + Firefox (headless, so without rendering) were
  run; Edge is Chromium; Safari is untested (no Mac).
- **Not yet over the internet:** the server, TURN and the two-network exit need the
  VPS (E5_LOG, "Next step").
- **Into a match:** fixed in E5c (2026-10-15). E6 found two pages stopping at the
  match's loading with "gate missed"; the run now stops at any poll whose input is
  missing (§1, E5_LOG §5). A whole nine-inning game has not been run in netplay yet.
- **Memory card:** absent on both sides; saves in netplay come with K5 or later.
