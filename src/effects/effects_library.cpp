#include "effects/effects_library.h"

#include "core/UniformRegistry.h"
#include "effects/effect_glsl_common.h"
#include "effects/ieffect.h"

#include <memory>

namespace acs
{

namespace
{

class EffectRaymarch final : public IEffect
{
public:
    const char* id() const override { return "effect_raymarch"; }
    const char* displayName() const override { return "Volumetric Raymarch (FBM)"; }
    EffectCategory category() const override { return EffectCategory::Production; }
    PerfTier recommendedMinTier() const override { return PerfTier::Medium; }

    std::string fragmentGlsl(bool gles, PerfTier tier) const override
    {
        (void)tier;
        const char* body = R"glsl(
uniform float u_rm_density;
uniform float u_rm_absorption;
uniform int u_rm_steps;
uniform vec3 u_rm_lightDir;
// Tier: High | GLES: 3.0 | Notes: exponential transmittance; integral replaced by fixed-step sum (real-time)
float h31(vec3 p){ return fract(sin(dot(p.xy,vec2(127.1,311.7))+p.z*19.19)*43758.5453); }
float fbm3(vec3 p){
    float a=0.0, w=0.5;
    for(int i=0;i<5;i++){ a+=h31(p)*w; p=p*2.03+0.1; w*=0.5; }
    return a;
}
void main(){
    vec2 uv = v_uv;
    vec3 ro = vec3(uv*6.0-3.0, -0.5);
    vec3 rd = normalize(vec3(0.15*sin(v_time*0.4), 0.08, 1.0));
    float trans = 1.0;
    vec3 scatter = vec3(0.0);
    int steps = u_rm_steps;
    if (u_perfTier == 0) steps = min(steps, 10);
    else if (u_perfTier == 1) steps = min(steps, 22);
    for(int i=0;i<64;i++){
        if(i>=steps) break;
        float t = float(i)*0.045;
        vec3 p = ro + rd*t;
        float rho = fbm3(p*0.9 + u_time*0.07) * u_rm_density;
        float dt = 0.045;
        float absorb = 1.0 - exp(-u_rm_absorption * rho * dt);
        vec3 Li = u_glowColor.rgb * max(0.0, dot(rd, normalize(u_rm_lightDir))) * 0.25;
        scatter += trans * absorb * Li * u_rm_density;
        trans *= exp(-u_rm_absorption * rho * dt * 0.35);
    }
    vec3 bg = u_bgColor.rgb * trans + scatter;
    vec3 col = (v_objId==0) ? bg : mix(bg, u_objColor.rgb, 0.35);
    OUT_COLOR = vec4(col, 1.0);
}
)glsl";
        return std::string(gles ? effectLibraryFragmentHeadEs() : effectLibraryFragmentHeadDesktop()) + body;
    }

    std::vector<ParamDescriptor> exposedParams() const override
    {
        std::vector<ParamDescriptor> p;
        p.push_back({"u_rm_density", "density", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.35F}, {0.05F}, {2.0F}, {}});
        p.push_back({"u_rm_absorption", "absorption", ParamGpuType::Float, UiWidgetType::SliderFloat, {1.2F}, {0.1F}, {6.0F}, {}});
        p.push_back({"u_rm_steps", "stepCount", ParamGpuType::Int, UiWidgetType::SliderInt, {32.0F}, {8.0F}, {64.0F}, {}});
        p.push_back({"u_rm_lightDir", "lightDir (xyz)", ParamGpuType::Vec4, UiWidgetType::SliderFloat, {0.3F, 0.7F, 0.45F, 0.0F}, {-1.0F}, {1.0F}, {}});
        return p;
    }

    void applyUniforms(UniformRegistry& reg, const EffectRuntimeParams& v) const override
    {
        reg.setFloat("u_rm_density", v.getFloat("u_rm_density", 0.35F));
        reg.setFloat("u_rm_absorption", v.getFloat("u_rm_absorption", 1.2F));
        reg.setInt("u_rm_steps", v.getInt("u_rm_steps", 32));
        const auto L = v.getVec4("u_rm_lightDir", {0.3F, 0.7F, 0.45F, 0.0F});
        reg.setVec3("u_rm_lightDir", L[0], L[1], L[2]);
    }
};

class EffectSdf final : public IEffect
{
public:
    const char* id() const override { return "effect_sdf"; }
    const char* displayName() const override { return "SDF 2D (smooth ops)"; }
    EffectCategory category() const override { return EffectCategory::Production; }
    PerfTier recommendedMinTier() const override { return PerfTier::Low; }

    std::string fragmentGlsl(bool gles, PerfTier) const override
    {
        const char* body = R"glsl(
uniform float u_sdf_smooth;
uniform float u_sdf_def;
uniform vec4 u_sdf_mat;
// Tier: Medium | GLES: 3.0 | Notes: analytic SDF, no raymarch; min/smooth-min replace integral CSG
float sdCircle(vec2 p, float r){ return length(p)-r; }
float sdBox(vec2 p, vec2 b){
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}
float smin(float a,float b,float k){ float h=max(k-abs(a-b),0.0)/k; return mix(b,a,h)-h*k*(1.0/4.0); }
void main(){
    vec2 uv = v_uv*2.0-1.0;
    uv.x *= u_resolution.x / max(u_resolution.y,1.0);
    float def = sin(u_time*u_sdf_def + uv.y*5.0)*0.08;
    float c = sdCircle(uv+vec2(0.15,0.0), 0.35+def);
    float b = sdBox(uv-vec2(0.25,0.1), vec2(0.22,0.14));
    float d = smin(c,b,u_sdf_smooth);
    vec3 mat = u_sdf_mat.rgb;
    float edge = 1.0-smoothstep(0.0,0.03,abs(d));
    vec3 col = mix(u_bgColor.rgb, mat, edge);
    if(v_objId==1) col = mix(col, u_objColor.rgb, 0.25);
    OUT_COLOR = vec4(col,1.0);
}
)glsl";
        return std::string(gles ? effectLibraryFragmentHeadEs() : effectLibraryFragmentHeadDesktop()) + body;
    }

    std::vector<ParamDescriptor> exposedParams() const override
    {
        std::vector<ParamDescriptor> p;
        p.push_back({"u_sdf_smooth", "smoothness", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.22F}, {0.05F}, {0.6F}, {}});
        p.push_back({"u_sdf_def", "deformation", ParamGpuType::Float, UiWidgetType::SliderFloat, {1.0F}, {0.0F}, {4.0F}, {}});
        p.push_back({"u_sdf_mat", "materialColor", ParamGpuType::Vec4, UiWidgetType::Color4, {0.2F, 0.65F, 0.9F, 1.0F}, {0.0F}, {1.0F}, {}});
        return p;
    }

    void applyUniforms(UniformRegistry& reg, const EffectRuntimeParams& v) const override
    {
        reg.setFloat("u_sdf_smooth", v.getFloat("u_sdf_smooth", 0.22F));
        reg.setFloat("u_sdf_def", v.getFloat("u_sdf_def", 1.0F));
        const auto c = v.getVec4("u_sdf_mat", {0.2F, 0.65F, 0.9F, 1.0F});
        reg.setVec4("u_sdf_mat", c[0], c[1], c[2], c[3]);
    }
};

class EffectFluid final : public IEffect
{
public:
    const char* id() const override { return "effect_fluid"; }
    const char* displayName() const override { return "Fluid / Wave (curl-ish)"; }
    EffectCategory category() const override { return EffectCategory::Production; }
    PerfTier recommendedMinTier() const override { return PerfTier::Low; }

    std::string fragmentGlsl(bool gles, PerfTier) const override
    {
        const char* body = R"glsl(
uniform float u_fluid_visc;
uniform float u_fluid_wind;
uniform float u_fluid_splash;
// Tier: Medium | GLES: 3.0 | Notes: FFT replaced by multi-band sine waves; curl from orthogonal noise gradients approximated
float n2(vec2 p){ return fract(sin(dot(p,vec2(41,289)))*758.5453); }
float curlish(vec2 p){
    float ex = n2(p+vec2(1,0))-n2(p-vec2(1,0));
    float ey = n2(p+vec2(0,1))-n2(p-vec2(0,1));
    return ex-ey;
}
void main(){
    vec2 uv = v_uv;
    vec2 p = uv * 12.0 + u_time * u_fluid_wind;
    float h = 0.0;
    h += sin(p.x*2.1+u_time*1.3)*0.35;
    h += sin(p.y*1.7-u_time*0.9)*0.25;
    h += sin((p.x+p.y)*1.3+u_time*0.7)*0.2;
    h += curlish(p*0.5) * u_fluid_visc * 0.4;
    float splash = sin(30.0*length(uv-vec2(0.5))+u_time*8.0)*u_fluid_splash;
    vec3 col = u_bgColor.rgb + vec3(h*0.3, h*0.15, h*0.45) + vec3(splash*0.05);
    if(v_objId==1) col = mix(col, u_objColor.rgb, 0.3);
    OUT_COLOR = vec4(col,1.0);
}
)glsl";
        return std::string(gles ? effectLibraryFragmentHeadEs() : effectLibraryFragmentHeadDesktop()) + body;
    }

    std::vector<ParamDescriptor> exposedParams() const override
    {
        std::vector<ParamDescriptor> p;
        p.push_back({"u_fluid_visc", "viscosity", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.6F}, {0.0F}, {2.0F}, {}});
        p.push_back({"u_fluid_wind", "windSpeed", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.35F}, {0.0F}, {2.0F}, {}});
        p.push_back({"u_fluid_splash", "splashIntensity", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.25F}, {0.0F}, {1.5F}, {}});
        return p;
    }

    void applyUniforms(UniformRegistry& reg, const EffectRuntimeParams& v) const override
    {
        reg.setFloat("u_fluid_visc", v.getFloat("u_fluid_visc", 0.6F));
        reg.setFloat("u_fluid_wind", v.getFloat("u_fluid_wind", 0.35F));
        reg.setFloat("u_fluid_splash", v.getFloat("u_fluid_splash", 0.25F));
    }
};

class EffectQuantum final : public IEffect
{
public:
    const char* id() const override { return "effect_quantum"; }
    const char* displayName() const override { return "Quantum interference (approx)"; }
    EffectCategory category() const override { return EffectCategory::Experimental; }
    PerfTier recommendedMinTier() const override { return PerfTier::Low; }

    std::string fragmentGlsl(bool gles, PerfTier) const override
    {
        const char* body = R"glsl(
uniform float u_q_phase;
uniform float u_q_coh;
uniform float u_q_prob;
// Tier: Low | GLES: 3.0 | Notes: path integral -> two-plane wave superposition; coherence controls envelope
void main(){
    vec2 uv = v_uv;
    float p1 = dot(uv, vec2(18.0, 12.0)) + u_time * u_q_phase;
    float p2 = dot(uv, vec2(-14.0, 20.0)) + u_time * (u_q_phase*0.73);
    float psi = sin(p1) + sin(p2);
    float env = exp(-length(uv-vec2(0.5))*u_q_coh);
    float prob = psi*psi;
    float th = step(u_q_prob, prob*env);
    vec3 col = mix(u_bgColor.rgb, u_glowColor.rgb, th*0.85 + prob*0.08*env);
    if(v_objId==1) col = mix(col, u_objColor.rgb, 0.2);
    OUT_COLOR = vec4(col,1.0);
}
)glsl";
        return std::string(gles ? effectLibraryFragmentHeadEs() : effectLibraryFragmentHeadDesktop()) + body;
    }

    std::vector<ParamDescriptor> exposedParams() const override
    {
        std::vector<ParamDescriptor> p;
        p.push_back({"u_q_phase", "phaseOffset", ParamGpuType::Float, UiWidgetType::SliderFloat, {2.0F}, {0.0F}, {8.0F}, {}});
        p.push_back({"u_q_coh", "coherenceLength", ParamGpuType::Float, UiWidgetType::SliderFloat, {6.0F}, {1.0F}, {24.0F}, {}});
        p.push_back({"u_q_prob", "probabilityThreshold", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.35F}, {0.0F}, {1.0F}, {}});
        return p;
    }

    void applyUniforms(UniformRegistry& reg, const EffectRuntimeParams& v) const override
    {
        reg.setFloat("u_q_phase", v.getFloat("u_q_phase", 2.0F));
        reg.setFloat("u_q_coh", v.getFloat("u_q_coh", 6.0F));
        reg.setFloat("u_q_prob", v.getFloat("u_q_prob", 0.35F));
    }
};

class EffectLens final : public IEffect
{
public:
    const char* id() const override { return "effect_gravity_lens"; }
    const char* displayName() const override { return "Gravitational lensing (Schwarzschild-ish)"; }
    EffectCategory category() const override { return EffectCategory::Experimental; }
    PerfTier recommendedMinTier() const override { return PerfTier::Low; }

    std::string fragmentGlsl(bool gles, PerfTier) const override
    {
        const char* body = R"glsl(
uniform float u_lens_mass;
uniform float u_lens_eps;
uniform float u_lens_time;
// Tier: Medium | GLES: 3.0 | Notes: deflection alpha ~ GM/(c^2 b) replaced by tunable u_lens_mass/(r+u_lens_eps); no geodesic integration
void main(){
    vec2 uv = v_uv;
    vec2 c = vec2(0.5);
    vec2 d = uv - c;
    float r = length(d) + u_lens_eps;
    float bend = u_lens_mass / (r*r + 0.001);
    vec2 suv = uv + normalize(d) * bend * u_lens_time * 0.02;
    float bgpat = sin(suv.x*40.0)*sin(suv.y*40.0)*0.05;
    vec3 col = u_bgColor.rgb + vec3(bgpat) + u_glowColor.rgb * bend * 0.15;
    if(v_objId==1) col = mix(col, u_objColor.rgb, 0.25);
    OUT_COLOR = vec4(col,1.0);
}
)glsl";
        return std::string(gles ? effectLibraryFragmentHeadEs() : effectLibraryFragmentHeadDesktop()) + body;
    }

    std::vector<ParamDescriptor> exposedParams() const override
    {
        std::vector<ParamDescriptor> p;
        p.push_back({"u_lens_mass", "mass", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.08F}, {0.0F}, {0.35F}, {}});
        p.push_back({"u_lens_eps", "horizonRadius (soft eps)", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.12F}, {0.02F}, {0.5F}, {}});
        p.push_back({"u_lens_time", "timeDilationFactor", ParamGpuType::Float, UiWidgetType::SliderFloat, {1.0F}, {0.2F}, {3.0F}, {}});
        return p;
    }

    void applyUniforms(UniformRegistry& reg, const EffectRuntimeParams& v) const override
    {
        reg.setFloat("u_lens_mass", v.getFloat("u_lens_mass", 0.08F));
        reg.setFloat("u_lens_eps", v.getFloat("u_lens_eps", 0.12F));
        reg.setFloat("u_lens_time", v.getFloat("u_lens_time", 1.0F));
    }
};

class EffectStub final : public IEffect
{
    const char* m_id;
    const char* m_name;
    EffectCategory m_cat;
    const char* m_body;
    std::vector<ParamDescriptor> m_params;

public:
    EffectStub(const char* id, const char* name, EffectCategory cat, const char* body, std::vector<ParamDescriptor> params)
        : m_id(id)
        , m_name(name)
        , m_cat(cat)
        , m_body(body)
        , m_params(std::move(params))
    {
    }

    const char* id() const override { return m_id; }
    const char* displayName() const override { return m_name; }
    EffectCategory category() const override { return m_cat; }
    PerfTier recommendedMinTier() const override { return PerfTier::Low; }

    std::string fragmentGlsl(bool gles, PerfTier) const override
    {
        return std::string(gles ? effectLibraryFragmentHeadEs() : effectLibraryFragmentHeadDesktop()) + m_body;
    }

    std::vector<ParamDescriptor> exposedParams() const override { return m_params; }

    void applyUniforms(UniformRegistry& reg, const EffectRuntimeParams& v) const override
    {
        for (const ParamDescriptor& d : m_params)
        {
            switch (d.m_gpuType)
            {
            case ParamGpuType::Float:
                reg.setFloat(d.m_uniformName.c_str(), v.getFloat(d.m_uniformName, d.m_defaultValue[0]));
                break;
            case ParamGpuType::Int:
            case ParamGpuType::Bool:
                reg.setInt(d.m_uniformName.c_str(), v.getInt(d.m_uniformName, static_cast<int>(d.m_defaultValue[0])));
                break;
            case ParamGpuType::Vec2: {
                const auto a = v.getVec4(d.m_uniformName, {d.m_defaultValue[0], d.m_defaultValue[1], 0.0F, 0.0F});
                reg.setVec2(d.m_uniformName.c_str(), a[0], a[1]);
                break;
            }
            case ParamGpuType::Vec3: {
                const auto a = v.getVec4(d.m_uniformName, {d.m_defaultValue[0], d.m_defaultValue[1], d.m_defaultValue[2], 0.0F});
                reg.setVec3(d.m_uniformName.c_str(), a[0], a[1], a[2]);
                break;
            }
            case ParamGpuType::Vec4: {
                const auto a = v.getVec4(d.m_uniformName, d.m_defaultValue);
                reg.setVec4(d.m_uniformName.c_str(), a[0], a[1], a[2], a[3]);
                break;
            }
            }
        }
    }
};

static const char kStubBloom[] = R"glsl(
uniform float u_stub_bloom_thr;
uniform float u_stub_bloom_int;
uniform float u_stub_aberr;
// Tier: High (stub) | GLES: 3.0 | Notes: placeholder — full bloom needs multi-pass FBO; chroma = UV split (no textures)
void main(){
    vec2 dir = normalize(v_uv - vec2(0.5) + 1e-5);
    vec2 off = dir * u_stub_aberr * 0.015;
    float bloom = pow(max(v_uv.x * v_uv.y, 0.0), u_stub_bloom_thr) * u_stub_bloom_int;
    vec3 base = u_bgColor.rgb + u_objColor.rgb * bloom;
    vec3 ab = vec3(
        u_bgColor.r * (1.0 + off.x * 6.0) + u_objColor.r * bloom,
        u_bgColor.g + u_objColor.g * bloom,
        u_bgColor.b * (1.0 - off.x * 6.0) + u_objColor.b * bloom
    );
    OUT_COLOR = vec4(mix(base, ab, 0.35), 1.0);
}
)glsl";

static const char kStubWarp[] = R"glsl(
uniform float u_stub_warp;
// Tier: Medium (stub) | GLES: 3.0 | Notes: single-layer warp; nested FBM warp deferred
void main(){
    vec2 uv = v_uv + u_stub_warp*sin(10.0*v_uv.y+u_time);
    float n = sin(20.0*uv.x)*0.5+0.5;
    OUT_COLOR = vec4(mix(u_bgColor.rgb, u_glowColor.rgb, n),1.0);
}
)glsl";

static const char kStubNeural[] = R"glsl(
uniform float u_stub_latent;
// Tier: Low (stub) | GLES: 3.0 | Notes: random Fourier features toy, not a trained latent
void main(){
    float z = sin(dot(v_uv,vec2(12.9898,78.233))*u_stub_latent) + tanh(dot(v_uv,vec2(-3.1,7.4))*0.7);
    OUT_COLOR = vec4(vec3(0.5+0.5*z),1.0);
}
)glsl";

static const char kStubEntropy[] = R"glsl(
uniform float u_stub_ent_rate;
uniform float u_stub_decay;
uniform float u_stub_fb_gain;
// Tier: Medium (stub) | GLES: 3.0 | Notes: feedback approximated by single-frame fractal sum + gain
void main(){
    float e = 0.0;
    vec2 p = v_uv;
    for(int i=0;i<6;i++){
        p = fract(p*2.7+0.1)-0.5;
        e += length(p)*u_stub_ent_rate;
    }
    float f = exp(-e*u_stub_decay) * (1.0 + u_stub_fb_gain * sin(v_uv.x*50.0+v_uv.y*50.0));
    OUT_COLOR = vec4(vec3(clamp(f,0.0,1.0)),1.0);
}
)glsl";

static const char kStubPlasma[] = R"glsl(
// Tier: Low | GLES: 3.0 | Notes: demo plasma
void main(){
    float c = sin(10.0*v_uv.x+u_time)+sin(10.0*v_uv.y-u_time*0.7);
    OUT_COLOR = vec4(vec3(0.5+0.5*c),1.0);
}
)glsl";

} // namespace

std::unique_ptr<IEffect> makeEffectRaymarch()
{
    return std::make_unique<EffectRaymarch>();
}

std::unique_ptr<IEffect> makeEffectSdf()
{
    return std::make_unique<EffectSdf>();
}

std::unique_ptr<IEffect> makeEffectFluid()
{
    return std::make_unique<EffectFluid>();
}

std::unique_ptr<IEffect> makeEffectQuantum()
{
    return std::make_unique<EffectQuantum>();
}

std::unique_ptr<IEffect> makeEffectGravitationalLens()
{
    return std::make_unique<EffectLens>();
}

std::unique_ptr<IEffect> makeEffectBloomStub()
{
    std::vector<ParamDescriptor> p;
    p.push_back({"u_stub_bloom_thr", "bloomThreshold", ParamGpuType::Float, UiWidgetType::SliderFloat, {4.0F}, {1.0F}, {12.0F}, {}});
    p.push_back({"u_stub_bloom_int", "intensity", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.4F}, {0.0F}, {2.0F}, {}});
    p.push_back({"u_stub_aberr", "aberrationOffset", ParamGpuType::Float, UiWidgetType::SliderFloat, {1.0F}, {0.0F}, {4.0F}, {}});
    return std::make_unique<EffectStub>("effect_bloom_stub", "Chromatic+Bloom (stub)", EffectCategory::Production, kStubBloom, std::move(p));
}

std::unique_ptr<IEffect> makeEffectDomainWarpStub()
{
    std::vector<ParamDescriptor> p;
    p.push_back({"u_stub_warp", "warpStrength", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.06F}, {0.0F}, {0.3F}, {}});
    return std::make_unique<EffectStub>("effect_domain_warp_stub", "Domain Warp (stub)", EffectCategory::Production, kStubWarp, std::move(p));
}

std::unique_ptr<IEffect> makeEffectNeuralStub()
{
    std::vector<ParamDescriptor> p;
    p.push_back({"u_stub_latent", "latentDim scale", ParamGpuType::Float, UiWidgetType::SliderFloat, {6.0F}, {1.0F}, {24.0F}, {}});
    return std::make_unique<EffectStub>("effect_neural_stub", "Latent slice (stub)", EffectCategory::Experimental, kStubNeural, std::move(p));
}

std::unique_ptr<IEffect> makeEffectEntropyStub()
{
    std::vector<ParamDescriptor> p;
    p.push_back({"u_stub_ent_rate", "entropyRate", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.35F}, {0.05F}, {1.5F}, {}});
    p.push_back({"u_stub_decay", "decayCurve", ParamGpuType::Float, UiWidgetType::SliderFloat, {1.2F}, {0.2F}, {4.0F}, {}});
    p.push_back({"u_stub_fb_gain", "feedbackGain", ParamGpuType::Float, UiWidgetType::SliderFloat, {0.5F}, {0.0F}, {3.0F}, {}});
    return std::make_unique<EffectStub>("effect_entropy_stub", "Digital entropy (stub)", EffectCategory::Experimental, kStubEntropy, std::move(p));
}

std::unique_ptr<IEffect> makeEffectPlasmaStub()
{
    return std::make_unique<EffectStub>("effect_plasma_stub", "Plasma demo (stub)", EffectCategory::Production, kStubPlasma, std::vector<ParamDescriptor>{});
}

} // namespace acs
