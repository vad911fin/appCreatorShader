#include "core/Renderer.h"

#include "core/GlEnums.h"
#include "core/GlLoader.h"
#include "core/ShaderProgram.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace acs
{

namespace
{

constexpr GLfloat kUnitQuad[] = {
    0.0F, 0.0F,
    1.0F, 0.0F,
    1.0F, 1.0F,
    0.0F, 0.0F,
    1.0F, 1.0F,
    0.0F, 1.0F,
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
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(kUnitQuad)), kUnitQuad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0U);
    glVertexAttribPointer(0U, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
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

void Renderer::drawObject(const ShaderProgram& program, float x, float y, float w, float h, int objectId)
{
    program.bind();
    program.setRectPixels(x, y, w, h);
    program.setViewportPixels(static_cast<float>(m_viewportW), static_cast<float>(m_viewportH));
    program.setObjectId(objectId);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0U);
}

void Renderer::render(const ShaderProgram& program, float skyW, float skyH, float squareW, float squareH,
    float moveSpeed, std::uint32_t ticksMs)
{
    ensureGpuResources();
    if (!program.valid())
    {
        return;
    }

    const float vw = static_cast<float>(m_viewportW);
    const float vh = static_cast<float>(m_viewportH);

    const float sx = (vw - skyW) * 0.5F;
    const float sy = (vh - skyH) * 0.5F;

    drawObject(program, sx, sy, skyW, skyH, 0);

    const float t = static_cast<float>(ticksMs) * 0.001F * moveSpeed;
    const float halfSqW = squareW * 0.5F;
    const float halfSqH = squareH * 0.5F;
    const float rx = std::max(0.0F, skyW * 0.5F - halfSqW);
    const float ry = std::max(0.0F, skyH * 0.5F - halfSqH);
    const float cx = sx + skyW * 0.5F + std::cos(t) * rx;
    const float cy = sy + skyH * 0.5F + std::sin(t * 1.37F) * ry;
    const float qx = cx - halfSqW;
    const float qy = cy - halfSqH;

    drawObject(program, qx, qy, squareW, squareH, 1);
}

} // namespace acs
