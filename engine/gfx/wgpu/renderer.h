// The WebGPU backend (E1): draws the GX state machine's DrawCalls into an
// emulated EFB (640 x 528 RGBA8 + 32-bit float depth), performs EFB copies
// (to XFB frames kept per address, or to textures that later draws sample in
// place of guest memory), and keeps the decoded-texture, sampler and pipeline
// caches. Only reads guest memory.
//
// Draws are recorded into render passes on the EFB, in one command encoder (a
// batch) with the clears (drawn inside the open pass) and the EFB copies (between
// passes), until something needs the work submitted (a read-back, a texture
// upload over contents the batch samples, the in-between replay, a VI field):
// flush() uploads the batch's vertices, indices and constants (one write each,
// at offsets that grow through the batch) and submits it (V6: a submit per pass,
// clear and copy before; Chrome's GPU process spent ~2 ms a field on that).
//
// Internal resolution (Q1): the EFB, its copies to the XFB and to textures can
// be kept at 2x or 3x the console's pixels in each direction. Everything the
// GX state machine hands over stays in console pixels (scissors, copy
// rectangles, texture sizes); only the GPU-side textures grow, and draws that
// sample an EFB copy sample it by normalized coordinates, so they see the same
// picture in more detail. Guest memory is never written either way.
#pragma once

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "gfx/draw.h"
#include "gfx/gp_memory.h"

namespace mvp::gfx {

class Gpu;

class GpuTimer;

class Renderer : public Backend {
 public:
  Renderer(Gpu& gpu, const std::uint8_t* mem, std::uint32_t mem_size, std::uint32_t scale = 1);
  ~Renderer() override;

  void draw(const DrawCall& d) override;
  void efb_copy(const EfbCopy& c) override;
  void invalidate_textures() override { ++epoch_; }

  // Q1: read guest memory as the GP sees it (gfx/gp_memory.h). Null: directly.
  void set_gp_memory(GpMemory* g) { gpm_ = g; }

  // Q5: time the render passes on the GPU (gpu_timer.h); null: not timed.
  void set_timer(GpuTimer* t) { timer_ = t; }

  // Q1: the internal resolution, 1-3 times the console's in each direction. A
  // change applies at the next XFB copy that clears the EFB (the game clears it
  // with every frame's copy), so no frame is drawn at two scales.
  void set_scale(std::uint32_t s) { want_scale_ = s < 1 ? 1 : s > kMaxScale ? kMaxScale : s; }
  std::uint32_t scale() const { return scale_; }
  static constexpr std::uint32_t kMaxScale = 3;

  // Ends the open render pass (if any) and submits the batch.
  void flush();
  // V6: the batch's encoder, for commands that must follow its work and go in its
  // submit (the presents: the frame's last commands and the picture shown, one
  // submit). Ends the open pass; the next flush() submits them.
  const wgpu::CommandEncoder& encoder() {
    end_pass();
    begin_batch();
    return encoder_;
  }

  // An XFB copy: the frame the video interface shows when it scans `addr`.
  // `width` and `height` are the texture's (console pixels times the scale).
  struct Frame {
    wgpu::Texture tex;
    wgpu::TextureView view;
    std::uint32_t width = 0, height = 0, addr = 0;
    std::uint64_t serial = 0;
  };
  // The frame last copied to `addr`, else null.
  const Frame* xfb(std::uint32_t addr) const;
  // The most recent XFB copy, else null.
  const Frame* latest_xfb() const { return latest_; }
  // Reads a frame back as RGBA8 rows (blocks; flushes first).
  std::vector<std::uint8_t> read_frame(const Frame& f);

  // ---- What the video interface scanned out (P1) ----
  // The console sends 480i: each VI field shows every other line of the XFB as it
  // is at that field, and the game sometimes draws two different pictures on
  // alternate fields (a highlight or panel on one field, not on the next: a
  // 30 Hz alternation). An interlaced TV shows such pixels as a steady mix of
  // both; shown as whole frames, one after the other, they flash (and strobe
  // when frames are skipped). scan_field() records, at every VI field, the XFB
  // the VI scans (by address, else the latest copy); scanout() is the picture to
  // show now: the frame of the last field, except that pixels alternating over
  // the last three fields (A, B, A) show the mean of the last two. Frames that do
  // not alternate are shown exactly as copied. Captures read the XFB itself.
  // V6: the mix is drawn in the batch: flush() (or present into encoder()) before
  // showing the picture.
  void scan_field(std::uint32_t addr);
  const Frame* scanout();
  bool scan_mix = true;  // false: scanout() is always the last field's frame (comparisons)
  bool is_mixed(const Frame* f) const { return f == &mixed_; }
  struct ScanStats {
    std::uint64_t fields = 0, new_frames = 0, mixed = 0;  // fields seen, new XFB frames, presents composed
  };
  const ScanStats& scan_stats() const { return scan_stats_; }
  // Reads the EFB color back (640 x 528 RGBA8, times the scale).
  std::vector<std::uint8_t> read_efb();

  // ---- In-between frames (V3, interp.cpp) ----
  // On a display that refreshes at 120 Hz the game's 60 frames a second can be shown
  // with a picture between each two: the frame drawn again with every object halfway
  // between where it was in the frame before and where it is now. While on, the
  // renderer records each frame from one XFB copy to the next (its passes' vertices,
  // indices, constants and draws, its clears and EFB-to-texture copies, and per
  // DrawCall the vertex range and the geometry's identity, DrawCall::ident). At the
  // XFB copy of frame N, if frame N-1 was recorded and ended exactly one VI field
  // before, it matches N's draws with N-1's (the same identity, the same occurrence
  // of that identity in the frame and the same vertex count; among several of one
  // identity the nearest when the order changed), replays N's record with each
  // matched vertex at the mean of its two clip-space positions (exactly the frame
  // drawn with the mean of the two frames' matrices: the transform is linear in
  // them), into a private EFB, private copy textures and a private frame, and keeps
  // that picture. Nothing the live frame draws changes. Not made on a scene cut
  // (too few vertices matched, or the matched ones moved too far on average).
  // Decoded textures are sampled as they are at the end of frame N (a texture the
  // game changed in the middle of the frame shows its last contents). The GX
  // processor must fill DrawCall::ident (GxProcessor::set_identify) while it is on.
  void set_interp(bool on);
  bool interp() const { return interp_on_; }
  // The game jumped (a save state loaded, a review restored): the frames recorded so far
  // are not the ones before the next, so the next frame gets no in-between.
  void interp_reset() {
    if (recording_) rec_[rec_cur_].valid = false;
    rec_[rec_cur_ ^ 1].valid = false;
  }
  // The in-between picture to show before the new frame `to` (XFB serial) when the
  // frame shown before it is `from`; scan-mixed like scanout() when the frames
  // around it alternate (V6: in the batch, as scanout()'s). Null if there is none.
  const Frame* between(std::uint64_t from, std::uint64_t to);
  struct InterpTune {
    float min_match = 0.6f;     // a cut below this share of the frame's vertices matched
    float cut_shift = 0.2f;     // ... or above this mean shift of matched 3D vertices (NDC units: the screen is 2 wide)
    float range_shift = 0.5f;   // a matched draw whose vertices moved more than this on average is not lerped
    // Order pairs of one identity farther apart than this: pair by nearest instead, if that
    // moves them less than half as far in all (the game drew them in another order).
    float reorder_shift = 0.1f;
  } interp_tune;
  struct InterpStats {
    std::uint64_t frames = 0;      // frames recorded and ended
    std::uint64_t made = 0;        // in-between pictures made
    std::uint64_t gap = 0;         // not made: the frame before was not one field earlier (or not recorded)
    std::uint64_t cut_match = 0;   // not made: too few vertices matched
    std::uint64_t cut_shift = 0;   // not made: matched vertices moved too far
    std::uint64_t still = 0;       // not needed: nothing moved (the in-between would be the frame)
    std::uint64_t other = 0;       // not made: another scale, too large, nothing drawn
    std::uint64_t shown = 0;       // pictures handed out by between()
    std::uint64_t retextured = 0;  // textures re-decoded after the frame had sampled them (shown with their last contents)
    std::uint64_t early_copies = 0;  // EFB copies sampled in a frame before it redid them (shown with their last contents)
    std::uint64_t reordered = 0;   // draws paired by nearest position instead of order
    double last_match = 0, last_shift = 0;  // the last frame's matched share and mean 3D shift
    double ms = 0;                 // CPU time in matching and replaying, in all
    double ms_match = 0;           // ... of which matching, the cut test and the halfway positions
    std::uint64_t ranges = 0, vertices = 0, replayed = 0;  // DrawCalls and vertices matched for, draws replayed
    // Frames matched (cut or not), by their mean 3D move (NDC: under 0.01, 0.02, 0.05, 0.1, 0.2,
    // 0.4, more) and by their matched share (under 0.6, 0.8, 0.9, 0.95, 0.99, more): for tuning.
    std::uint64_t shift_hist[7] = {}, match_hist[6] = {};
  };
  const InterpStats& interp_stats() const { return istats_; }

  struct Stats {
    std::uint64_t draws = 0, skipped = 0, passes = 0, pipelines = 0, textures_decoded = 0, texture_hits = 0;
    std::uint64_t copy_textures_used = 0, xfb_copies = 0, tex_copies = 0, clears = 0;
    std::uint64_t copies_dropped = 0;    // EFB-copy textures dropped: their memory changed since the copy
    std::uint64_t copy_size_misses = 0;  // a texture at an EFB copy's address with another size
    std::uint64_t copy_tall = 0;         // P2: a taller texture over a copy (the copy's rows, then memory)
    std::uint64_t copy_inside = 0;       // Q7: a texture whose memory starts inside an EFB copy's, not at it
    std::uint64_t unknown_formats = 0, indirect_draws = 0, fog_draws = 0;
    std::uint64_t merged = 0, encoded = 0;  // draws appended to the previous one; WebGPU draws encoded
    std::uint64_t same_state = 0;           // draws that reused the previous draw's resolved state
    std::uint64_t tmem_textures = 0;        // preloaded (TMEM) textures decoded
    std::uint64_t vertex_bytes = 0, index_bytes = 0;  // uploaded
    std::uint64_t submits = 0;  // V6: batches submitted
  };
  const Stats& stats() const { return stats_; }

 private:
  // A pipeline and its vertex layout (which attributes, the stride).
  struct Pipeline {
    wgpu::RenderPipeline pipe;
    std::uint32_t attrs = 0, stride = 16;
  };
  struct TexEntry;
  struct CopyEntry;
  struct CfgHash {
    std::size_t operator()(const PipelineConfig& c) const;
  };

  void begin_batch();  // V6: the command encoder that the next passes, clears and copies go into
  void begin_pass();
  void end_pass();     // V6: ends the open pass (its last draw emitted); the batch stays open
  // V6: one constant slot (kUniformSlot bytes, zero-padded) at the end of the batch's constants; its offset.
  std::uint32_t push_slot(const void* data, std::size_t size);
  void create_efb();  // the EFB's textures at scale_
  void emit_pending();
  const Pipeline& pipeline(const PipelineConfig& c);
  wgpu::TextureView texture(const TextureDesc& t, std::uint32_t* w, std::uint32_t* h);
  wgpu::Sampler sampler(const TextureDesc& t);
  wgpu::TextureView tmem_texture(const TextureDesc& t);
  wgpu::TextureView tall_copy(CopyEntry& e, const TextureDesc& t);
  void clear_efb(const EfbCopy& c);
  void copy_to_texture(const EfbCopy& c);
  void ensure_mix_pipe();  // the scan-out mix's pipeline (scanout(), between())
  std::uint64_t hash_memory(std::uint32_t addr, std::uint32_t size);
  // [addr, addr + size) of guest memory as the GP sees it, null if outside.
  const std::uint8_t* guest(std::uint32_t addr, std::uint32_t size);

  Gpu& gpu_;
  const std::uint8_t* mem_;
  std::uint32_t mem_size_;
  GpMemory* gpm_ = nullptr;
  GpuTimer* timer_ = nullptr;  // Q5
  std::uint32_t scale_ = 1, want_scale_ = 1;

  wgpu::Texture efb_color_, efb_depth_;
  wgpu::TextureView efb_color_view_, efb_depth_view_;
  wgpu::BindGroupLayout bgl_uniforms_, bgl_textures_;
  wgpu::PipelineLayout layout_;
  wgpu::Buffer vbuf_, ubuf_, ibuf_;
  wgpu::BindGroup uniform_group_;
  wgpu::Texture dummy_tex_;
  wgpu::TextureView dummy_view_;
  wgpu::Sampler dummy_sampler_;

  // The open batch (V6) and its open pass. The batch's vertices, indices and
  // constants, uploaded at flush().
  wgpu::CommandEncoder encoder_;
  bool batch_open_ = false;
  std::uint64_t batch_serial_ = 0;
  wgpu::RenderPassEncoder pass_;
  bool pass_open_ = false;
  std::uint64_t pass_serial_ = 0;
  std::vector<std::uint8_t> vdata_, udata_;
  std::vector<std::uint32_t> idata_;
  // V6: which uploads must wait for a submit. Each draw that resolves its textures
  // gets a serial (sample_serial_) that marks every texture it samples; flush()
  // sets submitted_ to the last draw whose commands it submitted (not a draw still
  // resolving its textures: it lands in the next batch). A texture marked after
  // submitted_ is sampled by commands not submitted yet: its contents may only be
  // replaced (a queue write, which runs before them) after a submit.
  std::uint64_t sample_serial_ = 0, submitted_ = 0;
  bool resolving_ = false;
  bool unsubmitted(std::uint64_t mark) const { return mark > submitted_; }
  // The last draw, recorded but not encoded yet: a next draw with the same
  // pipeline, textures, constants and scissor appends its vertices and indices
  // to it (the game sends thousands of small primitives with one state). Its
  // vertices start at `base` (in strides) in the pass's vertex buffer.
  struct Pending {
    bool active = false;
    wgpu::RenderPipeline pipeline;
    wgpu::BindGroup textures;
    std::uint32_t uoff = 0, first_index = 0, index_count = 0, base = 0, vertices = 0, stride = 0;
    std::uint32_t scissor[4] = {};
    std::uint64_t state = 0;
    DrawUniforms u;
    const void* pipe = nullptr;  // V3: the Pipeline (for the frame record)
    std::uint32_t bind = 0;      // V3: its texture bind group in the frame record
  } pending_;
  // The previous draw's resolved state (E3c): a draw with the same nonzero
  // DrawCall::state and topology, in the same open pass and texture epoch,
  // samples the same textures with the same pipeline and constants.
  struct Last {
    std::uint64_t state = 0, pass = 0, epoch = 0;
    std::uint8_t topology = 0xFF;
    const Pipeline* pipe = nullptr;
    wgpu::BindGroup textures;
    bool indirect = false, fog = false;
    std::uint32_t bind = 0;       // V3: `textures` in the frame record (rec_bind)
    std::uint64_t bind_rec = 0;   // ... of this record (Record::id)
  } last_;
  // What the open pass has bound, to skip repeated state.
  const void* bound_pipeline_ = nullptr;
  const void* bound_textures_ = nullptr;
  std::uint32_t bound_uoff_ = ~0u;
  std::uint32_t bound_scissor_[4] = {};
  bool bound_vbuf_ = false;
  bool bound_ibuf_ = false;

  std::unordered_map<PipelineConfig, Pipeline, CfgHash> pipelines_;
  std::unordered_map<std::string, wgpu::ShaderModule> modules_;
  std::unordered_map<std::uint64_t, std::unique_ptr<TexEntry>> textures_;  // V5: hashed (was ordered; never iterated)
  std::map<std::uint32_t, std::unique_ptr<CopyEntry>> copies_;  // EFB copies to textures, by address
  std::unordered_map<std::uint64_t, std::unique_ptr<TexEntry>> tmem_textures_;  // preloaded textures, by content
  std::vector<std::uint8_t> tmem_buf_;
  std::unordered_map<std::uint32_t, wgpu::Sampler> samplers_;
  // Texture bind groups, keyed by the addresses of the 8 views and 8 samplers
  // they bind. Q7: each entry holds references to those views and samplers.
  // A bind group does not keep the handles alive everywhere: emdawnwebgpu (the
  // browser) frees a view's handle when the last reference to it goes, and the
  // next view created can get the same address. A key of a released view could
  // then find the old group for a new view, and the draw would sample the old
  // texture (EFB copies replaced at a new scale stayed frozen: the game's
  // top-left 128 x 128 save and restore showed a stale block). forget_view()
  // drops the entries of a view that is being replaced, so its texture can go.
  struct BindEntry {
    wgpu::BindGroup group;
    std::array<wgpu::TextureView, 8> views;
    std::array<wgpu::Sampler, 8> samplers;
  };
  std::map<std::array<const void*, 16>, BindEntry> bind_groups_;
  void forget_view(const wgpu::TextureView& v);
  std::uint64_t epoch_ = 1;

  // Clears and EFB-to-texture conversion. V6: their constants are slots of the
  // batch's constants, bound by dynamic offset (the replay binds its own buffers).
  wgpu::RenderPipeline clear_pipes_[8];
  wgpu::BindGroupLayout bgl_clear_;
  wgpu::BindGroup clear_group_;      // the constants buffer
  wgpu::RenderPipeline convert_pipe_;
  wgpu::BindGroupLayout bgl_convert_;
  wgpu::BindGroup convert_group_;    // the constants buffer and the EFB's views (made again with the EFB)
  wgpu::Sampler point_sampler_;

  std::map<std::uint32_t, Frame> frames_;
  const Frame* latest_ = nullptr;
  // The scan-out history: three frame textures; the slot and XFB serial of the
  // frame at the last field ([0]) and the two before.
  Frame scan_[3];
  int scan_slot_[3] = {-1, -1, -1};
  std::uint64_t scan_serial_[3] = {};
  Frame mixed_;
  std::uint64_t mixed_key_ = ~0ull;
  wgpu::RenderPipeline mix_pipe_;
  ScanStats scan_stats_;
  std::uint64_t serial_ = 0;
  std::uint64_t copy_serial_ = 0;  // EFB copies to textures (CopyEntry::serial)
  Stats stats_;

  // ---- V3: in-between frames (interp.cpp) ----
  // One encoded draw of a recorded batch, as emit_pending() encoded it.
  struct RecDraw {
    const Pipeline* pipe;
    std::uint32_t bind, uoff, first_index, index_count, base;
    std::uint32_t scissor[4];
  };
  // A recorded batch (V6; a pass before): what flush() uploaded and the draws it encoded.
  struct RecPass {
    std::vector<std::uint8_t> v, u;
    std::vector<std::uint32_t> i;
    std::vector<RecDraw> draws;
    bool filled = false;
  };
  // A DrawCall's vertices in a recorded batch: `count` vertices of `stride` bytes at
  // byte `voff` of batch `pass`; their clip-space positions before the 2D snap at
  // Record::pos[4 * pos ...].
  struct RecRange {
    std::uint64_t ident;
    std::uint32_t pass, voff, count, stride, pos;
    std::uint32_t n3;  // vertices in front of the camera that are not 2D (w is not exactly 1)
  };
  struct RecClear {
    int m;
    float data[8];
    std::uint32_t rect[4];  // EFB pixels at the scale: x, y, width, height
  };
  struct RecCopy {
    wgpu::TextureView live;  // the copy's texture at the time (draws that sample it bind this view)
    std::uint32_t pw, ph;
    std::int32_t params[8];  // the conversion's
  };
  struct RecBind {
    wgpu::BindGroup group;
    std::array<wgpu::TextureView, 8> views;
    std::array<wgpu::Sampler, 8> samplers;
  };
  enum class RecKind : std::uint8_t { Pass, Clear, Copy };
  // Pass: draws [begin, end) of batch `index` (V6: a batch's draws, between its clears and
  // copies); Clear, Copy: clears[index], copies[index].
  struct RecOp {
    RecKind kind;
    std::uint32_t index;
    std::uint32_t begin = 0, end = 0;
  };
  struct Record {
    bool valid = false;
    std::uint64_t id = 0;             // never reused (last_.bind_rec)
    std::uint64_t field = 0;          // the VI field count at the XFB copy that ended it
    std::uint64_t serial = 0;         // that copy's frame serial
    std::uint32_t scale = 1;
    std::uint32_t xfb[4] = {};        // the XFB copy's EFB rectangle (console pixels)
    std::vector<RecOp> ops;
    std::vector<RecPass> passes;      // batches: [0, npasses) in use; the rest keep their buffers
    std::size_t npasses = 0;
    std::vector<RecClear> clears;
    std::vector<RecCopy> copies;
    std::vector<RecBind> binds;
    std::unordered_map<const void*, std::uint32_t> bind_index;
    std::vector<RecRange> ranges;
    std::vector<float> pos;
    std::uint64_t batch_serial = 0;   // the live batch being recorded into passes[npasses - 1]
  };
  void rec_begin(const EfbCopy& c);   // at an XFB copy, before its clear
  // At an XFB copy of the EFB rectangle (x0, y0, w, h): ends the record, makes the in-between.
  void rec_end(std::uint32_t x0, std::uint32_t y0, std::uint32_t w, std::uint32_t h, std::uint64_t serial);
  void rec_drop();
  std::uint32_t rec_bind(const BindEntry& be);
  void rec_range(const DrawCall& d, std::size_t voff, std::uint32_t stride);
  void rec_pass_end();                // flush() of a recorded batch: its data moves into the record
  void rec_draw();                    // emit_pending() of a recorded batch
  void rec_clear(int m, const float* data, const std::uint32_t* rect);
  void rec_copy(const wgpu::TextureView& view, std::uint32_t pw, std::uint32_t ph, const std::int32_t* params);
  bool interp_make(Record& cur, Record& prev);  // matching and the replay; false: not made
  void interp_targets();              // the private EFB at the current scale
  const wgpu::BindGroup& interp_bind(const RecBind& b);
  wgpu::Buffer interp_buffer(wgpu::Buffer& buf, std::uint64_t& cap, std::uint64_t need, wgpu::BufferUsage usage,
                             const char* label);
  // Frame record or not: whether this pass, these draws are being recorded.
  bool rec_on() const { return recording_; }

  bool interp_on_ = false;            // asked for
  bool recording_ = false;            // a record is open (from an XFB copy on)
  Record rec_[2];
  int rec_cur_ = 0;                   // rec_[rec_cur_] is being recorded, the other is the frame before
  std::uint64_t rec_ids_ = 0;
  std::uint64_t fields_ = 0;          // VI fields so far (scan_field)
  InterpStats istats_;
  // Private targets: the EFB (color, depth) at iefb_scale_, the copies redone by the
  // replay (by the live copy view they stand for), the in-between pictures.
  wgpu::Texture iefb_color_, iefb_depth_;
  wgpu::TextureView iefb_color_view_, iefb_depth_view_;
  std::uint32_t iefb_scale_ = 0;
  struct PrivCopy {
    wgpu::TextureView live;  // keeps the key's view alive
    wgpu::Texture tex;
    wgpu::TextureView view;
    std::uint32_t pw = 0, ph = 0;
    std::uint64_t used = 0;  // the replay that last used it
  };
  std::map<const void*, PrivCopy> priv_copies_;
  std::unordered_map<const void*, wgpu::TextureView> subst_;  // this replay: live copy view -> private view
  std::map<std::array<const void*, 16>, BindEntry> ibind_;    // substitute bind groups
  wgpu::Buffer ivbuf_, iibuf_, iubuf_, iclear_buf_, iconv_buf_;
  std::uint64_t ivcap_ = 0, iicap_ = 0, iucap_ = 0, iclear_cap_ = 0, iconv_cap_ = 0;
  wgpu::BindGroup iuniform_group_;
  std::uint64_t replays_ = 0;
  struct Between {
    Frame frame;
    std::uint64_t from = 0, to = 0;
  };
  Between between_[3];
  int between_next_ = 0;
  Frame between_mixed_;
  std::uint64_t between_mixed_key_ = ~0ull;
  // Matching scratch (reused).
  struct Pair {
    std::uint32_t cur, prev;
  };
  std::vector<Pair> near_;
  // An identity of the frame before: its draws there (a chain in order), this frame's.
  struct Slot {
    std::uint64_t ident = 0;
    std::uint32_t phead = ~0u, ptail = ~0u, cursor = ~0u, np = 0;
    std::uint32_t chead = ~0u, ctail = ~0u, nc = 0;
    std::uint8_t far = 0;
  };
  std::vector<Slot> slots_;
  std::vector<std::uint32_t> pnext_, cnext_, match_, far_;
  std::vector<std::uint8_t> taken_;
};

}  // namespace mvp::gfx
