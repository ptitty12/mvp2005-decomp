// TEV -> WGSL (E1): one WGSL module per pixel pipeline configuration, the way
// Aurora (MIT) builds its shaders from GX state, but written for this engine's
// PipelineConfig and computed in integers like the hardware: TEV inputs are
// 8-bit, registers 11-bit signed, the lerp uses c + (c >> 7), compare modes
// compare packed 8/16/24-bit values, and the alpha test runs on 8-bit alpha.
//
// Interface shared with the WebGPU backend:
// - vertex buffer (E3c): only what the fragment stage reads, packed in this
//   order: @location(0) clip position (vec4f), (1) color0, (2) color1 (vec4f),
//   (3..10) texture coordinates 0-7 (s, t, q as vec3f); vertex_attrs() says which
//   are present, vertex_stride() the stride. Absent ones are zero in the shader;
// - @group(0) @binding(0..7): texture maps 0-7, @binding(8..15): their samplers;
// - @group(0) @binding(16): DrawUniforms (draw.h) as `U`, with a dynamic offset (V6: in
//   the textures' group, one bind group per draw; it was group 0 of its own).
// Entry points: `vs_main`, `fs_main`.
#pragma once

#include <string>

#include "gfx/draw.h"

namespace mvp::gfx {

std::string generate_wgsl(const PipelineConfig& c);

// The vertex attributes a configuration's shader reads: bit 0 color0, bit 1
// color1, bit 2 + k texture coordinate k (the position is always there).
u32 vertex_attrs(const PipelineConfig& c);
// Bytes per vertex with those attributes (16 + 16 per color + 12 per coordinate).
u32 vertex_stride(u32 attrs);

// Size of DrawUniforms as WGSL lays out `U` (the buffer slot is rounded up to 256).
constexpr std::size_t kUniformSize = 448;
static_assert(sizeof(DrawUniforms) == kUniformSize);

}  // namespace mvp::gfx
