// The threaded engine's render thread (P2; the mvp-web-mt build, Emscripten
// pthreads). The emulation (the engine Worker, Emscripten's main runtime
// thread) appends what the renderer must see to a command ring in shared
// memory (gfx/render_ring.h): every GX command (copied), VI fields, presents,
// canvas resizes. The render thread (a pthread, its own Worker) owns the WebGPU
// device and the OffscreenCanvas and runs the GX processor and the renderer on
// them (render_side.h), so the emulation does not wait for them.
//
// The emulation waits for the render thread only where the guest is about to
// write memory the render thread has still to read. As each GX command is
// appended, gfx::GpReads works out what the renderer will read for it (the
// array elements its indices select, indexed XF loads, palettes, TMEM preloads,
// what an EFB copy's texture covers, a texture the first time a draw samples
// it) and marks those bytes in ppc::g_gp_pages (per 2^ppc::kGpShift = 64 bytes:
// the ring position after the command). Every guest write path (the
// interpreter's and the translated module's stores, the write-gather pipe's
// bursts, DI and ARAM DMA) checks the table and, where the render thread has
// not yet passed that position, waits for it (RenderRing::wait_read). Q6: what
// a command reads of vertex arrays, indexed XF loads, palettes and preloads goes
// into the ring as a copy instead, ahead of the command (gfx/gp_memory.h,
// GpMemory::shadow), and a guest write there ends the copy (a record ahead of
// the write) rather than waiting; textures and EFB-copy ranges are marked. A
// primitive too large for the ring is read in place, behind a fence. The
// guest's GPU sync points (runtime/gx_fifo.h, GfxObserver::gp_sync: draw done,
// PE tokens, CP status reads, a CP breakpoint) can be fences as well
// (render_tune's `sync_fences`; off: the write checks cover what they cover).
// The renderer only reads guest memory, so guest state is the same as with the
// inline renderer, and so is everything it draws (gfx/digest_tap.h: the
// per-field digests of both agree).
//
// The render thread waits for records in place (Atomics.wait) while the
// emulation runs a batch, so a fence is answered at once; between batches it
// waits in its event loop and the emulation wakes it (a proxied call). It shows
// at most one frame per display refresh (the page counts its animation frames in
// shared memory: a Worker started by a Worker has none). Where the page gave a
// frame port (main.ts, cross-origin isolated), the render thread draws into a
// canvas of its own and posts each frame to the page as an ImageBitmap, which
// the page's idle main thread shows: an OffscreenCanvas of the page drawn by
// this thread reached the screen irregularly (its frames are committed only
// between tasks, and this thread works inside long tasks).
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "runtime/gx_fifo.h"

namespace mvp::web {

// Starts the render thread: WebGPU device first, then the canvas the page
// handed to this Worker (Module.mvpCanvas) or the page's frame port
// (Module.mvpFrames, with Module.mvpVsync), transferred to the render thread,
// which shows its canvas under `selector`. `on_gfx(ok, message)` runs later on this
// thread when the renderer is ready or has failed. False if the thread could
// not be created.
bool render_thread_start(const char* selector, int width, int height, void (*on_gfx)(int ok, const char* msg));
bool render_thread_running();
// Development: the fence's polling before it sleeps (us), the render thread's
// polling for records (ms), the publishing step (KiB); negative or 0: unchanged.
// `sync_fences` 1: also wait at the guest's GPU sync points (draw done, tokens,
// ...), 0: only where a guest write meets memory the render thread still reads
// (the default); -1: unchanged. `copies` (Q6) 1: what commands read of vertex
// arrays, XF loads, palettes and preloads goes to the render thread as a copy
// (the default), 0: guest writes there wait for it instead; -1: unchanged.
void render_tune(double fence_spin_us, double render_spin_ms, int publish_kib, int sync_fences, int copies);
// Development (Q2): a slower machine, simulated: after each stretch of work the
// render thread spins (factor - 1) times as long (1: off).
void render_host_slow(double factor);
// The emulation thread's time spent waiting for the render thread so far (write
// waits, lag waits, fences), ms.
double render_waited_ms();

// From the emulation, in order (each appends to the ring).
void render_memory(const std::uint8_t* mem, std::uint32_t size);
void render_gx(const std::uint8_t* cmd, std::size_t len);
// Q4: the commands of one GX FIFO push (rt::GfxObserver::gx_commands), as one record.
void render_gx_batch(const rt::GxCommand* cmds, std::size_t n);
void render_field(std::uint64_t field, std::uint32_t xfb);
void render_sync();          // the fence: returns when the render thread finished every record so far
void render_batch_begin();   // a run batch starts (the render thread waits in place for records)
bool render_present(std::uint64_t frame_polls);  // ends the batch; true once a frame has reached the canvas
void render_present_field(std::uint64_t frame_polls);  // V1: a field's picture inside a batch (the batch goes on)
void render_batch_end();     // Q5: ends the batch without a present (the run made no new field: nothing new to show)
void render_flush();
void render_resize(int width, int height);
void render_scale(int scale);  // Q1: the internal resolution (render_side.h)
void render_gpu_load(int passes);  // Q5 (development): extra GPU work per present (render_side.h)
// V3: in-between frames (render_side.h). While on, every present is shown at once and
// a new frame with an in-between posts two pictures; the page queues them (presenter.ts).
void render_interp(bool on);
void render_trace(bool on);
void render_digest(bool on, std::uint64_t detail_from, std::uint64_t detail_to);
// Q1: what the GP sees of memory (gfx/gp_memory.h). The game wrote the data
// cache's lines over [ea, ea + len) back (the lines that changed travel in the
// ring with the range), or memory there was written without the cache.
void render_gp_flush(std::uint32_t ea, std::uint32_t len);
void render_gp_write_through(std::uint32_t addr, std::uint32_t len);

// Save-state files (runtime/state_file.h) with the render thread: the GX
// processor's state once the render thread has finished every record so far
// (RenderSide::export_view; empty without a renderer), and back before the next
// record (import_view, which also starts the GP's snapshots over, as a field
// does). A load replaces guest memory outside the guest's write paths: call
// render_memory_replaced() before it, which does what a guest write over all of
// memory does (waits for the commands that read it, ends the render thread's
// copies).
std::vector<std::uint8_t> render_view_export();
void render_view_import(const std::uint8_t* data, std::size_t len);
void render_memory_replaced();
// V2: gameplay mods' snapshots (mods/gameplay/timeline.h): the GX processor's state saved
// with a slot and restored from it, in stream order (a restore also needs
// render_memory_replaced() before guest memory is replaced).
void render_view_save(int slot);
void render_view_load(int slot);

// For mvp_status and the tools (snapshots the render thread took at its last present).
std::string render_trace_take();
std::string render_digest_take();
std::string render_status_json();
std::uint64_t render_presents();

}  // namespace mvp::web
