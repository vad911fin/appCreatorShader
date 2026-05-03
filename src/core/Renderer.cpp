#include "core/Renderer.h"

#include "core/AppState.h"
#include "core/GlEnums.h"
#include "core/GlLoader.h"
#include "core/ShaderProgram.h"
#include "core/UniformRegistry.h"
#include "effects/effect_registry.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace acs
{

namespace
{

constexpr GLfloat kInterleavedQuad[] = {
    0.0F, 0.0F, 0.0F, 0.0F,
    1.0F, 0.0F, 1.0F, 0.0F,
    1.0F, 1.0F, 1.0F, 1.0F,
    0.0F, 0.0F, 0.0F, 0.0F,
    1.0F, 1.0F, 1.0F, 1.0F,
    0.0F, 1.0F, 0.0F, 1.0F,
};

} // namespace

Renderer::Renderer() = default;

Renderer::~Renderer()
{
    destroyGpu();
}

Renderer::Renderer(Renderer&& other) noexcept
    : m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_viewportW(other.m_viewportW)
    , m_viewportH(other.m_viewportH)
{
    other.m_vao = 0U;
    other.m_vbo = 0U;
}

Renderer& Renderer::operator=(Renderer&& other) noexcept
{
    if (this != &other)
    {
        destroyGpu();
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_viewportW = other.m_viewportW;
        m_viewportH = other.m_viewportH;
        other.m_vao = 0U;
        other.m_vbo = 0U;
    }
    return *this;
}

void Renderer::destroyGpu()
{
    if (m_vbo != 0U)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0U;
    }
    if (m_vao != 0U)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0U;
    }
}

void Renderer::ensureGpuResources()
{
    if (m_vao != 0U)
    {
        return;
    }
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(kInterleavedQuad)), kInterleavedQuad, GL_STATIC_DRAW);
    constexpr GLsizei stride = sizeof(GLfloat) * 4;
    glEnableVertexAttribArray(0U);
    glVertexAttribPointer(0U, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(1U);
    glVertexAttribPointer(1U, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(GLfloat) * 2));
    glBindVertexArray(0U);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);
}

void Renderer::resizeViewport(int drawableW, int drawableH)
{
    ensureGpuResources();
    m_viewportW = drawableW > 0 ? drawableW : 1;
    m_viewportH = drawableH > 0 ? drawableH : 1;
    glViewport(0, 0, m_viewportW, m_viewportH);
}

void Renderer::drawObject(
    const ShaderProgram& program, UniformRegistry& uniforms, float x, float y, float w, float h, int objectId)
{
    program.bind();
    uniforms.setVec4("uRect", x, y, w, h);
    uniforms.setInt("uObjectId", objectId);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0U);
}

void Renderer::render(
    const ShaderProgram& program, UniformRegistry& uniforms, AppState& state, std::uint32_t ticksMs)
{
    ensureGpuResources();
    if (!program.valid())
    {
        return;
    }

    const ShaderConstructorConfig& cfg = state.m_constructor;
    const float vw = static_cast<float>(m_viewportW);
    const float vh = static_cast<float>(m_viewportH);
    const float skyW = cfg.m_bgWidth;
    const float skyH = cfg.m_bgHeight;
    const float squareW = cfg.m_objWidth;
    const float squareH = cfg.m_objHeight;

    const float sx = (vw - skyW) * 0.5F;
    const float sy = (vh - skyH) * 0.5F;

    const float t = static_cast<float>(ticksMs) * 0.001F * cfg.m_timeScale * cfg.m_animSpeed;
    const float halfSqW = squareW * 0.5F;
    const float halfSqH = squareH * 0.5F;
    const float rx = std::max(0.0F, skyW * 0.5F - halfSqW);
    const float ry = std::max(0.0F, skyH * 0.5F - halfSqH);

    float cx = sx + skyW * 0.5F;
    float cy = sy + skyH * 0.5F;
    if (cfg.m_animate)
    {
        cx += std::cos(t) * rx;
        cy += std::sin(t * 1.37F) * ry;
    }
    if (cfg.m_objPosPixelMode)
    {
        cx += cfg.m_objPosX;
        cy += cfg.m_objPosY;
    }
    else
    {
        cx += cfg.m_objPosX * skyW * 0.5F;
        cy += cfg.m_objPosY * skyH * 0.5F;
    }
    const float qx = cx - halfSqW;
    const float qy = cy - halfSqH;

    program.bind();
    uniforms.applySystemUniforms(t, vw, vh, vw, vh);
    uniforms.applyConstructorConfig(cfg);
    uniforms.setInt("u_perfTier", static_cast<int>(state.m_perfTier));
    if (state.m_effectLibraryActive)
    {
        EffectRegistry::instance().registerAll();
        const IEffect* eff = EffectRegistry::instance().effectAt(static_cast<std::size_t>(state.m_selectedLibraryEffectIndex));
        if (eff != nullptr)
        {
            eff->applyUniforms(uniforms, state.m_effectRuntimeParams);
        }
    }

    drawObject(program, uniforms, sx, sy, skyW, skyH, 0);
    drawObject(program, uniforms, qx, qy, squareW, squareH, 1);
}

} // namespace acs
