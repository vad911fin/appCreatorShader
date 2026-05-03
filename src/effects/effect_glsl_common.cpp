#include "effects/effect_glsl_common.h"

namespace acs
{

const char* effectLibraryFragmentHeadDesktop()
{
    return R"glsl(#version 330 core
// Tier: varies | GLES: N/A | Notes: shared compatibility block for effect library (texture-less)
in vec2 v_uv;
in float v_time;
flat in int v_objId;
uniform float u_time;
uniform vec2 u_resolution;
uniform vec4 u_viewport;
uniform int u_perfTier;
uniform float u_bgWidth;
uniform float u_bgHeight;
uniform float u_objWidth;
uniform float u_objHeight;
uniform float u_objPosX;
uniform float u_objPosY;
uniform int u_objPosPixelMode;
uniform float u_animSpeed;
uniform float u_timeScale;
uniform vec4 u_bgColor;
uniform vec4 u_objColor;
uniform vec4 u_glowColor;
uniform float u_intensity;
uniform float u_density;
uniform float u_noiseScale;
uniform float u_edgeSoftness;
uniform int u_enableFog;
uniform int u_enableTrail;
uniform int u_enableGlow;
uniform int u_enableDistortion;
uniform int u_animate;
uniform int u_effectPreset;
uniform int u_blendMode;
uniform int u_noiseType;
uniform float u_extraWarp;
out vec4 FragColor;
#define OUT_COLOR FragColor
)glsl";
}

const char* effectLibraryFragmentHeadEs()
{
    return R"glsl(#version 300 es
precision highp float;
// Tier: varies | GLES: 3.0 | Notes: ES effect library preamble
in vec2 v_uv;
in float v_time;
flat in int v_objIdOut;
uniform float u_time;
uniform vec2 u_resolution;
uniform vec4 u_viewport;
uniform int u_perfTier;
uniform float u_bgWidth;
uniform float u_bgHeight;
uniform float u_objWidth;
uniform float u_objHeight;
uniform float u_objPosX;
uniform float u_objPosY;
uniform int u_objPosPixelMode;
uniform float u_animSpeed;
uniform float u_timeScale;
uniform vec4 u_bgColor;
uniform vec4 u_objColor;
uniform vec4 u_glowColor;
uniform float u_intensity;
uniform float u_density;
uniform float u_noiseScale;
uniform float u_edgeSoftness;
uniform int u_enableFog;
uniform int u_enableTrail;
uniform int u_enableGlow;
uniform int u_enableDistortion;
uniform int u_animate;
uniform int u_effectPreset;
uniform int u_blendMode;
uniform int u_noiseType;
uniform float u_extraWarp;
out vec4 fragColor;
#define v_objId v_objIdOut
#define OUT_COLOR fragColor
)glsl";
}

} // namespace acs
