#include "core/ShaderDefaults.h"

namespace acs
{

std::string defaultDesktopVertexSource()
{
    return R"glsl(#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
uniform vec4 uRect;
uniform vec2 uViewport;
uniform int uObjectId;
uniform float u_time;
uniform int u_enableDistortion;
out vec2 v_uv;
out float v_time;
flat out int v_objId;

void main()
{
    vec2 px = a_pos * uRect.zw + uRect.xy;
    vec2 ndc = px / uViewport * 2.0 - 1.0;
    ndc.y = -ndc.y;
    if (u_enableDistortion != 0)
    {
        ndc.x += 0.03 * sin(u_time * 2.2 + a_uv.y * 6.28318);
        ndc.y += 0.02 * cos(u_time * 1.7 + a_uv.x * 6.28318);
    }
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uv = a_uv;
    v_time = u_time;
    v_objId = uObjectId;
}
)glsl";
}

static const char kFragCommonBody330[] = R"glsl(
float hash21(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float valueNoise(vec2 x)
{
    vec2 i = floor(x);
    vec2 f = fract(x);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float voronoiCell(vec2 p)
{
    vec2 g = floor(p);
    return hash21(g);
}

float gradientNoise(vec2 uv, float t)
{
    return fract(dot(uv, vec2(12.9898, 78.233)) + t);
}

float sampleNoise(vec2 uv, int mode, float t, float scale)
{
    vec2 p = uv * scale + t * 0.07;
    if (mode == 0)
    {
        return valueNoise(p);
    }
    if (mode == 1)
    {
        return voronoiCell(p * 4.0);
    }
    if (mode == 2)
    {
        return valueNoise(p) + 0.5 * valueNoise(p * 2.3);
    }
    return gradientNoise(uv, t);
}

vec3 applyBlend(vec3 base, vec3 layer, int mode)
{
    if (mode == 1)
    {
        return base + layer * 0.35;
    }
    if (mode == 3)
    {
        return 1.0 - (1.0 - base) * (1.0 - clamp(layer, 0.0, 1.0));
    }
    return mix(base, layer, 0.5);
}

void main()
{
    vec2 uv = v_uv;
    if (u_enableDistortion != 0)
    {
        float w = 0.04 * u_intensity;
        uv.x += w * sin(v_time * 3.0 + uv.y * 20.0);
        uv.y += w * cos(v_time * 2.5 + uv.x * 20.0);
    }
    if (u_effectPreset == 5)
    {
        uv += u_extraWarp * vec2(sin(uv.y * 12.0 + v_time), cos(uv.x * 12.0 - v_time)) * 0.02;
    }

    float n = sampleNoise(uv, u_noiseType, v_time * u_timeScale, u_noiseScale);
    vec3 col = u_bgColor.rgb;
    if (v_objId == 1)
    {
        col = u_objColor.rgb;
    }

    float edge = smoothstep(0.0, max(u_edgeSoftness, 1e-4),
        min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y)));
    vec3 border = col * 0.25;
    if (v_objId == 1)
    {
        col = mix(border, u_objColor.rgb, edge);
    }

    if (u_enableGlow != 0 && v_objId == 1)
    {
        col += u_glowColor.rgb * u_intensity * (1.0 - edge);
    }

    if (u_enableFog != 0 && v_objId == 0)
    {
        vec3 fogc = mix(col, vec3(n * 0.4), u_density);
        col = mix(col, fogc, u_intensity);
    }

    if (u_effectPreset == 1)
    {
        col += vec3(0.06) * n * u_intensity * float(v_objId == 0 ? 1 : 0);
    }
    else if (u_effectPreset == 2)
    {
        col = mix(col, col * 0.55 + vec3(0.15) * n, u_density * float(v_objId == 0 ? 1 : 0));
    }
    else if (u_effectPreset == 3 && v_objId == 1)
    {
        col += vec3(0.12) * (1.0 - edge) * sin(v_time * 6.0);
    }
    else if (u_effectPreset == 4 && v_objId == 1)
    {
        col += u_glowColor.rgb * 0.35 * pow(1.0 - edge, 3.0);
    }
    else if (u_effectPreset == 5)
    {
        col = applyBlend(col, vec3(n * 0.2), u_blendMode);
    }
    else if (u_effectPreset == 6)
    {
        float c = 1.0 - distance(fract(uv * 8.0 + v_time * 0.1), vec2(0.5));
        col += vec3(0.08) * c * u_intensity;
    }

    if (u_enableTrail != 0 && v_objId == 1)
    {
        col += vec3(0.05) * sin(v_time * 10.0 + uv.x * 30.0);
    }

    vec4 outc = vec4(col, (v_objId == 0) ? u_bgColor.a : u_objColor.a);
    if (u_blendMode == 1 && v_objId == 1)
    {
        outc.rgb *= 1.15;
    }
    OUT_COLOR = outc;
}
)glsl";

std::string defaultDesktopConstructorFragment()
{
    std::string head = R"glsl(#version 330 core
in vec2 v_uv;
in float v_time;
flat in int v_objId;
uniform vec2 u_resolution;
uniform vec4 u_viewport;
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
    std::string body(kFragCommonBody330);
    return head + body;
}

std::string defaultMobileConstructorFragment()
{
    std::string head = R"glsl(#version 300 es
precision highp float;
in vec2 v_uv;
in float v_time;
flat in int v_objIdOut;
uniform vec2 u_resolution;
uniform vec4 u_viewport;
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
    std::string body(kFragCommonBody330);
    return head + body;
}

std::string defaultDesktopFragmentSource()
{
    return defaultDesktopConstructorFragment();
}

std::string defaultMobileFragmentSource()
{
    return defaultMobileConstructorFragment();
}

std::string defaultMobileVertexSource()
{
    return R"glsl(#version 300 es
precision highp float;
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
uniform vec4 uRect;
uniform vec2 uViewport;
uniform int uObjectId;
uniform float u_time;
uniform int u_enableDistortion;
out vec2 v_uv;
out float v_time;
flat out int v_objIdOut;

void main()
{
    vec2 px = a_pos * uRect.zw + uRect.xy;
    vec2 ndc = px / uViewport * 2.0 - 1.0;
    ndc.y = -ndc.y;
    if (u_enableDistortion != 0)
    {
        ndc.x += 0.03 * sin(u_time * 2.2 + a_uv.y * 6.28318);
        ndc.y += 0.02 * cos(u_time * 1.7 + a_uv.x * 6.28318);
    }
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uv = a_uv;
    v_time = u_time;
    v_objIdOut = uObjectId;
}
)glsl";
}

} // namespace acs
