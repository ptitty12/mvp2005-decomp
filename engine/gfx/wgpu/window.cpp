#include "gfx/wgpu/window.h"

#ifndef __EMSCRIPTEN__
#include <SDL3/SDL.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <cstring>

#include "gfx/wgpu/gpu.h"
#include "gfx/wgpu/gpu_timer.h"

namespace mvp::gfx {

namespace {

// A full-window triangle that shows the source in the rectangle `rect` (x0,
// y0, x1, y1 in normalized window coordinates) and black outside it. `size`:
// the source's texels (xy) and the rectangle's window pixels (zw). Larger
// source: an area filter, a box over each window pixel's footprint from a grid
// of bilinear taps. Smaller: sharp bilinear (an integer nearest prescale, then
// bilinear from that).
const char* kBlitWgsl = R"(
struct Params { rect: vec4f, size: vec4f };
@group(0) @binding(0) var src: texture_2d<f32>;
@group(0) @binding(1) var samp: sampler;
@group(0) @binding(2) var<uniform> params: Params;

struct VsOut { @builtin(position) pos: vec4f, @location(0) uv: vec2f };

@vertex fn vs(@builtin(vertex_index) i: u32) -> VsOut {
  var p = array<vec2f, 3>(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
  var o: VsOut;
  o.pos = vec4f(p[i], 0.0, 1.0);
  o.uv = vec2f(p[i].x * 0.5 + 0.5, 0.5 - p[i].y * 0.5);
  return o;
}

@fragment fn fs(in: VsOut) -> @location(0) vec4f {
  let r = params.rect;
  let uv = (in.uv - r.xy) / (r.zw - r.xy);
  let ts = params.size.xy;
  let ds = max(params.size.zw, vec2f(1.0));
  let ratio = ts / ds;  // source texels per window pixel
  var c = vec3f(0.0);
  if (ratio.x > 1.001 || ratio.y > 1.001) {
    // Area: n x n bilinear taps spread over the pixel's footprint.
    let n = clamp(ceil(max(ratio.x, ratio.y)) + 1.0, 2.0, 5.0);
    let step = ratio / n / ts;
    let start = uv - ratio * 0.5 / ts + step * 0.5;
    for (var j = 0.0; j < n; j += 1.0) {
      for (var i = 0.0; i < n; i += 1.0) {
        c += textureSampleLevel(src, samp, start + vec2f(i, j) * step, 0.0).rgb;
      }
    }
    c /= n * n;
  } else {
    // Sharp bilinear.
    let prescale = max(floor(ds / ts), vec2f(1.0));
    let t = uv * ts;
    let f = fract(t) - 0.5;
    let range = 0.5 - 0.5 / prescale;
    let g = (f - clamp(f, -range, range)) * prescale + 0.5;
    c = textureSampleLevel(src, samp, (floor(t) + g) / ts, 0.0).rgb;
  }
  if (any(uv < vec2f(0.0)) || any(uv > vec2f(1.0))) { return vec4f(0.0, 0.0, 0.0, 1.0); }
  return vec4f(c, 1.0);
}
)";

}  // namespace

#ifndef __EMSCRIPTEN__

std::unique_ptr<Window> Window::open(const char* title, int width, int height, std::string* err) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    if (err) *err = std::string("SDL_Init: ") + SDL_GetError();
    return nullptr;
  }
  auto w = std::unique_ptr<Window>(new Window());
  w->window_ = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
  if (!w->window_) {
    if (err) *err = std::string("SDL_CreateWindow: ") + SDL_GetError();
    return nullptr;
  }
  w->width_ = width;
  w->height_ = height;
  return w;
}

Window::~Window() {
  if (configured_) surface_.Unconfigure();
  surface_ = nullptr;
  if (window_) SDL_DestroyWindow(window_);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

#else  // __EMSCRIPTEN__

std::unique_ptr<Window> Window::open_canvas(const char* selector, int width, int height, std::string* err) {
  if (!selector || !*selector || width <= 0 || height <= 0) {
    if (err) *err = "no canvas";
    return nullptr;
  }
  auto w = std::unique_ptr<Window>(new Window());
  w->selector_ = selector;
  w->width_ = width;
  w->height_ = height;
  return w;
}

Window::~Window() {
  if (configured_) surface_.Unconfigure();
  surface_ = nullptr;
}

void Window::resize(int width, int height) {
  if (width <= 0 || height <= 0 || (width == width_ && height == height_)) return;
  width_ = width;
  height_ = height;
  if (gpu_) configure();
}

#endif  // __EMSCRIPTEN__

bool Window::attach(Gpu& gpu, std::string* err) {
  gpu_ = &gpu;
  wgpu::SurfaceDescriptor sd{};
#if defined(__EMSCRIPTEN__)
  wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector src{};
  src.selector = wgpu::StringView(selector_.data(), selector_.size());
  sd.nextInChain = &src;
#elif defined(_WIN32)
  SDL_PropertiesID props = SDL_GetWindowProperties(window_);
  wgpu::SurfaceSourceWindowsHWND src{};
  src.hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
  if (!src.hwnd) {
    if (err) *err = "no HWND for the SDL window";
    return false;
  }
  src.hinstance = GetModuleHandleW(nullptr);
  sd.nextInChain = &src;
#else
  if (err) *err = "window surfaces are only implemented for Windows and the browser so far";
  return false;
#endif
  surface_ = gpu.instance().CreateSurface(&sd);
  if (!surface_) {
    if (err) *err = "CreateSurface failed";
    return false;
  }
  wgpu::SurfaceCapabilities caps{};
  surface_.GetCapabilities(gpu.adapter(), &caps);
  if (caps.formatCount == 0) {
    if (err) *err = "the surface supports no format on this adapter";
    return false;
  }
  for (std::size_t i = 0; i < caps.presentModeCount; ++i)
    if (caps.presentModes[i] == wgpu::PresentMode::Mailbox) have_mailbox_ = true;
  format_ = caps.formats[0];
  for (std::size_t i = 0; i < caps.formatCount; ++i)
    if (caps.formats[i] == wgpu::TextureFormat::BGRA8Unorm) format_ = caps.formats[i];

  wgpu::ShaderModule sm = gpu.shader(kBlitWgsl, "blit");
  wgpu::ColorTargetState target{};
  target.format = format_;
  wgpu::FragmentState fs{};
  fs.module = sm;
  fs.entryPoint = "fs";
  fs.targetCount = 1;
  fs.targets = &target;
  wgpu::RenderPipelineDescriptor pd{};
  pd.label = "blit";
  pd.vertex.module = sm;
  pd.vertex.entryPoint = "vs";
  pd.fragment = &fs;
  pd.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
  blit_ = gpu.device().CreateRenderPipeline(&pd);
  blit_layout_ = blit_.GetBindGroupLayout(0);

  wgpu::SamplerDescriptor samp{};
  samp.magFilter = wgpu::FilterMode::Linear;
  samp.minFilter = wgpu::FilterMode::Linear;
  sampler_ = gpu.device().CreateSampler(&samp);
  wgpu::BufferDescriptor bd{};
  bd.size = 32;
  bd.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
  params_ = gpu.device().CreateBuffer(&bd);
  configure();
  return true;
}

void Window::configure() {
#ifndef __EMSCRIPTEN__
  int w = 0, h = 0;
  SDL_GetWindowSizeInPixels(window_, &w, &h);
  if (w <= 0 || h <= 0) return;
  width_ = w;
  height_ = h;
#else
  const int w = width_, h = height_;
#endif
  wgpu::SurfaceConfiguration cfg{};
  cfg.device = gpu_->device();
  cfg.format = format_;
  cfg.usage = wgpu::TextureUsage::RenderAttachment;
  cfg.width = static_cast<std::uint32_t>(w);
  cfg.height = static_cast<std::uint32_t>(h);
  mode_ = want_mailbox_ && have_mailbox_ ? wgpu::PresentMode::Mailbox : wgpu::PresentMode::Fifo;
  cfg.presentMode = mode_;
  cfg.alphaMode = wgpu::CompositeAlphaMode::Opaque;
  surface_.Configure(&cfg);
  configured_ = true;
}

const char* Window::present_mode_name() const { return mode_ == wgpu::PresentMode::Mailbox ? "mailbox" : "fifo"; }

bool Window::pump() {
#ifndef __EMSCRIPTEN__
  SDL_Event e;
  bool open = true;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) open = false;
    if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED && gpu_) configure();
  }
  return open;
#else
  return true;
#endif
}

void Window::present(const wgpu::TextureView& src, std::uint32_t src_width, std::uint32_t src_height,
                     GpuTimer* timer, const wgpu::CommandEncoder* into) {
  if (!configured_ || src_width == 0 || src_height == 0) return;
  wgpu::SurfaceTexture st{};
  surface_.GetCurrentTexture(&st);
  if (st.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
      st.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
    configure();
    return;
  }
  // Fit the picture into the window, centered, at the display's aspect ratio
  // (4:3; or the frame's own with display_aspect_ 0).
  const float sw = static_cast<float>(src_width), sh = static_cast<float>(src_height);
  const float ww = static_cast<float>(width_), wh = static_cast<float>(height_);
  const float aspect = display_aspect_ > 0 ? display_aspect_ : sw / sh;
  const float pw = std::min(ww, wh * aspect), ph = std::min(wh, ww / aspect);
  const float x0 = (ww - pw) * 0.5f / ww, y0 = (wh - ph) * 0.5f / wh;
  const float params[8] = {x0, y0, 1.0f - x0, 1.0f - y0, sw, sh, pw, ph};
  // V6: written when they change, and the bind group of a source shown before reused
  // (each was a write and a bind group per present).
  if (std::memcmp(params, params_now_, sizeof params) != 0) {
    gpu_->queue().WriteBuffer(params_, 0, params, sizeof params);
    std::memcpy(params_now_, params, sizeof params);
  }
  const wgpu::BindGroup* found = nullptr;
  for (const BlitGroup& g : blit_groups_)
    if (g.view.Get() == src.Get() && g.group) found = &g.group;
  if (!found) {
    wgpu::BindGroupEntry entries[3]{};
    entries[0].binding = 0;
    entries[0].textureView = src;
    entries[1].binding = 1;
    entries[1].sampler = sampler_;
    entries[2].binding = 2;
    entries[2].buffer = params_;
    entries[2].size = 32;
    wgpu::BindGroupDescriptor bgd{};
    bgd.layout = blit_layout_;
    bgd.entryCount = 3;
    bgd.entries = entries;
    BlitGroup& g = blit_groups_[blit_next_++ % 8];
    g.view = src;
    g.group = gpu_->device().CreateBindGroup(&bgd);
    found = &g.group;
  }
  const wgpu::BindGroup& bg = *found;

  wgpu::RenderPassColorAttachment ca{};
  ca.view = st.texture.CreateView();
  ca.loadOp = wgpu::LoadOp::Clear;
  ca.storeOp = wgpu::StoreOp::Store;
  ca.clearValue = {0, 0, 0, 1};
  wgpu::RenderPassDescriptor rp{};
  rp.colorAttachmentCount = 1;
  rp.colorAttachments = &ca;
  if (timer) rp.timestampWrites = timer->pass();  // Q5
  wgpu::CommandEncoder own;
  if (!into) own = gpu_->device().CreateCommandEncoder();
  const wgpu::CommandEncoder& enc = into ? *into : own;
  wgpu::RenderPassEncoder pass = enc.BeginRenderPass(&rp);
  pass.SetPipeline(blit_);
  pass.SetBindGroup(0, bg);
  pass.Draw(3);
  pass.End();
  if (into) return;  // V6: the caller submits it (and presents nothing natively: the browser's path)
  wgpu::CommandBuffer cb = own.Finish();
  gpu_->queue().Submit(1, &cb);
#ifndef __EMSCRIPTEN__
  surface_.Present();  // the browser presents the canvas when the task ends
#endif
}

void Window::burn(const wgpu::TextureView& src, std::uint32_t src_width, std::uint32_t src_height, int passes,
                  GpuTimer* timer) {
  if (!gpu_ || passes <= 0 || src_width == 0 || src_height == 0) return;
  if (!burn_params_) {
    wgpu::BufferDescriptor bd{};
    bd.size = 32;
    bd.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    burn_params_ = gpu_->device().CreateBuffer(&bd);
  }
  if (!burn_target_ || burn_w_ != src_width || burn_h_ != src_height) {
    wgpu::TextureDescriptor td{};
    td.size = {src_width, src_height, 1};
    td.format = format_;
    td.usage = wgpu::TextureUsage::RenderAttachment;
    burn_target_ = gpu_->device().CreateTexture(&td);
    burn_w_ = src_width;
    burn_h_ = src_height;
  }
  // The whole target, as if it were a quarter of the source's size: the area
  // filter's 5 x 5 taps at every one of the source's pixels.
  const float sw = static_cast<float>(src_width), sh = static_cast<float>(src_height);
  const float params[8] = {0.0f, 0.0f, 1.0f, 1.0f, sw, sh, sw / 4.0f, sh / 4.0f};
  gpu_->queue().WriteBuffer(burn_params_, 0, params, sizeof params);
  wgpu::BindGroupEntry entries[3]{};
  entries[0].binding = 0;
  entries[0].textureView = src;
  entries[1].binding = 1;
  entries[1].sampler = sampler_;
  entries[2].binding = 2;
  entries[2].buffer = burn_params_;
  entries[2].size = 32;
  wgpu::BindGroupDescriptor bgd{};
  bgd.layout = blit_layout_;
  bgd.entryCount = 3;
  bgd.entries = entries;
  wgpu::BindGroup bg = gpu_->device().CreateBindGroup(&bgd);
  wgpu::TextureView view = burn_target_.CreateView();
  wgpu::CommandEncoder enc = gpu_->device().CreateCommandEncoder();
  wgpu::RenderPassColorAttachment ca{};
  ca.view = view;
  ca.loadOp = wgpu::LoadOp::Clear;
  ca.storeOp = wgpu::StoreOp::Store;
  ca.clearValue = {0, 0, 0, 1};
  wgpu::RenderPassDescriptor rp{};
  rp.colorAttachmentCount = 1;
  rp.colorAttachments = &ca;
  if (timer) rp.timestampWrites = timer->pass();  // one pass, so one timestamp pair covers it all
  wgpu::RenderPassEncoder pass = enc.BeginRenderPass(&rp);
  pass.SetPipeline(blit_);
  pass.SetBindGroup(0, bg);
  for (int k = 0; k < passes; ++k) pass.Draw(3);
  pass.End();
  wgpu::CommandBuffer cb = enc.Finish();
  gpu_->queue().Submit(1, &cb);
}

}  // namespace mvp::gfx
