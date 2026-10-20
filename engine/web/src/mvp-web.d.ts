// The engine module that Emscripten builds from wasm/mvp_web.cpp (mvp-web.js,
// and P2's threaded build mvp-web-mt.js, which has the same exports plus
// wasmMemory). Only what the Worker uses; pointers and sizes are numbers.

export interface MvpEngine {
  HEAPU8: Uint8Array;
  wasmMemory?: WebAssembly.Memory;  // the threaded build (P2): views made after another thread grew it
  UTF8ToString(ptr: number, maxBytes?: number): string;
  specialHTMLTargets: Record<string, unknown>;
  _malloc(size: number): number;
  _free(ptr: number): void;

  _mvp_last_error(): number;
  _mvp_open_disc(size: number): number;
  _mvp_disc_info(): number;
  _mvp_translate(text: number, len: number): number;
  _mvp_translate_info(): number;
  _mvp_boot(script: number, len: number, stopFields: number, stopFrames: number): number;
  _mvp_gfx_init(selector: number, width: number, height: number): void;
  _mvp_canvas_resize(width: number, height: number): void;
  _mvp_run_fields(n: number): number;
  _mvp_present(): number;
  _mvp_batch_end(): void;  // Q5: ends a batch that made no new field, without a present
  _mvp_trace(on: number): void;   // P1: the presentation trace (development)
  _mvp_trace_take(): number;
  // P2: the render thread (the threaded build), and the renderer digest (development).
  _mvp_threads_available(): number;
  _mvp_set_threaded(on: number): void;
  _mvp_threaded(): number;
  _mvp_digest(on: number, detailFrom: number, detailTo: number): void;
  _mvp_render_tune(fenceSpinUs: number, renderSpinMs: number, publishKib: number, syncFences: number, copies: number): void;
  _mvp_host_slow(factor: number): void;  // Q2 (development): a slower machine, simulated
  _mvp_digest_take(): number;
  _mvp_gp_view(on: number): void;
  _mvp_render_scale(scale: number): void;  // Q1: the internal resolution, 1-3
  _mvp_gpu_load(passes: number): void;  // Q5 (development): extra GPU work per present
  _mvp_gpu_timing(on: number): void;  // Q5 (development): 0 = no GPU timestamps (before mvp_gfx_init)
  _mvp_mod_enable(spec: number): number;
  _mvp_mod_seed(seed: number): void;
  _mvp_mod_intro(ms: number): void;  // the page's announcement before each review
  _mvp_mod_settings(): number;
  _mvp_mod_status(): number;
  _mvp_set_pad(port: number, connected: number, buttons: number, sx: number, sy: number, cx: number, cy: number,
               tl: number, tr: number): void;
  _mvp_status(): number;
  _mvp_guest_line(): number;
  // Save-state files (runtime/state_file.h): carried out at the next frame poll; then mvp_state_result.
  _mvp_state_save(header: number): number;
  _mvp_state_load(data: number, n: number): number;
  _mvp_state_result(): number;
  _mvp_state_out_ptr(): number;
  _mvp_state_out_len(): number;
  _mvp_state_out_release(): void;
  // The memory card in slot A (runtime/memcard.h): before mvp_boot, its bytes (or n = 0: a new card); offline only.
  _mvp_card_set(data: number, n: number): number;
  _mvp_card_writes(): number;  // -1 without a card
  _mvp_card_ptr(): number;
  _mvp_card_len(): number;

  // E2c: the server's image, the module cache, audio.
  _mvp_open_disc_url(size: number, blockKib: number, maxBlocks: number, maxRun: number, ahead: number): number;
  _mvp_disc_trace_enable(): void;
  _mvp_disc_trace(): number;
  _mvp_disc_want(): number;
  _mvp_disc_want_first(): number;
  _mvp_disc_block(): number;
  _mvp_disc_insert(first: number, count: number, data: number, n: number, ok: number): void;
  _mvp_keep_module(keep: number): void;
  _mvp_module_blob(): number;
  _mvp_module_blob_ptr(): number;
  _mvp_module_bytes_ptr(): number;
  _mvp_module_bytes_len(): number;
  _mvp_module_release(): void;
  _mvp_translate_cached(blob: number, len: number): number;
  _mvp_audio_enable(on: number): void;
  _mvp_audio_take(): number;
  _mvp_audio_ptr(): number;
  _mvp_audio_rate(): number;
  _mvp_audio_ring(capacity: number): number;  // Q2: the threaded build's ring in shared memory (0: none)

  // P1: every guest DVD read with its host timing (development).
  _mvp_io_log_enable(): void;
  _mvp_io_log(): number;
  _mvp_input_log_enable(): void;
  _mvp_input_log(): number;

  // Netplay (E5, wasm/net_web.cpp).
  _mvp_net_begin(player: number, delay: number, hashEvery: number, build: number, buildLen: number, nonce: number): number;
  _mvp_net_recv(data: number, len: number): void;
  _mvp_net_pump(): number;
  _mvp_net_quit(): void;
  _mvp_net_status(): number;
  _mvp_net_windows(): number;
  _mvp_net_diagnostic(): number;
  _mvp_net_lead(): number;  // Q5: the input lead, ms (NaN before any)

  // Roster packs (E6, wasm/packs_web.cpp; the same exports as mvp-packs.js).
  _mvp_packs_reset(): void;
  _mvp_pack_begin(label: number, len: number, kind: number): void;
  _mvp_pack_file(path: number, len: number, data: number, n: number): void;
  _mvp_pack_end(): void;
  _mvp_packs_json(): number;
  _mvp_packs_mount(): number;

  // Set by the Worker before the calls that use them.
  mvpDisc?: File;
  mvpCanvas?: OffscreenCanvas | null;  // P2: the canvas the render thread takes
  mvpVsync?: SharedArrayBuffer | null;  // P2: the page's refresh counter, for the render thread
  mvpFrames?: MessagePort | null;       // P2: where each frame goes as an ImageBitmap (the page shows it)
  mvpDiscUrl?: string;
  mvpOnGfx?: (ok: number, message: string) => void;
  mvpOnProgress?: (text: string, step: number, steps: number) => void;
  mvpNetSend?: (datagram: Uint8Array) => void;
}

export interface MvpEngineOptions {
  print?: (text: string) => void;
  printErr?: (text: string) => void;
}

export default function createMvpEngine(options?: MvpEngineOptions): Promise<MvpEngine>;
