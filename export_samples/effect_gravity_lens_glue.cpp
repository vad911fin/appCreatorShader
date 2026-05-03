// Sample glue: bind uniforms each frame after glUseProgram(prog).
// Tier: Medium | GLES: 3.0 | Notes: phenomenological bend, not full geodesics.

#include "effect_gravity_lens_config.h"

#include <GLES3/gl3.h>
// On desktop replace with <GL/gl.h> + your loader; keep names identical.

static void applyEffectGravityLensUniforms(GLuint prog, const EffectGravityLensConfig& c)
{
    const GLint locMass = glGetUniformLocation(prog, "u_lens_mass");
    const GLint locEps = glGetUniformLocation(prog, "u_lens_eps");
    const GLint locTime = glGetUniformLocation(prog, "u_lens_time");
    if (locMass >= 0)
    {
        glUniform1f(locMass, c.mass);
    }
    if (locEps >= 0)
    {
        glUniform1f(locEps, c.horizonRadiusSoftEps);
    }
    if (locTime >= 0)
    {
        glUniform1f(locTime, c.timeDilationFactor);
    }
}

static const char kEffectGravityLensFs[] = R"glsl(
#ifdef __ANDROID__
#version 300 es
precision highp float;
in vec2 v_uv;
flat in int v_objIdOut;
out vec4 fragColor;
#define OUT_COLOR fragColor
#define OBJ_ID v_objIdOut
#else
#version 330 core
in vec2 v_uv;
flat in int v_objId;
out vec4 FragColor;
#define OUT_COLOR FragColor
#define OBJ_ID v_objId
#endif
// Tier: Medium | GLES: 3.0 | Notes: Schwarzschild-ish screen deflection (toy)
uniform float u_lens_mass;
uniform float u_lens_eps;
uniform float u_lens_time;
uniform vec4 u_bgColor;
uniform vec4 u_objColor;
uniform vec4 u_glowColor;
void main(){
    vec2 uv = v_uv;
    vec2 c = vec2(0.5);
    vec2 d = uv - c;
    float r = length(d) + u_lens_eps;
    float bend = u_lens_mass / (r*r + 0.001);
    vec2 suv = uv + normalize(d + 1e-5) * bend * u_lens_time * 0.02;
    float bgpat = sin(suv.x*40.0)*sin(suv.y*40.0)*0.05;
    vec3 col = u_bgColor.rgb + vec3(bgpat) + u_glowColor.rgb * bend * 0.15;
    if(OBJ_ID==1) col = mix(col, u_objColor.rgb, 0.25);
    OUT_COLOR = vec4(col,1.0);
}
)glsl";
