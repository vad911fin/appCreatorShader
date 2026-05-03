#ifndef APP_CREATOR_SHADER_RENDERER_H
#define APP_CREATOR_SHADER_RENDERER_H

#include "core/GlIncludes.h"

#include <cstdint>

namespace acs
{

class ShaderProgram;

class Renderer
{
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(Renderer&& other) noexcept;
    Renderer& operator=(Renderer&& other) noexcept;

    void resizeViewport(int drawableW, int drawableH);

    void render(const ShaderProgram& program, float skyW, float skyH, float squareW, float squareH,
        float moveSpeed, std::uint32_t ticksMs);

private:
    void ensureGpuResources();

    GLuint m_vao{0U};
    GLuint m_vbo{0U};
    int m_viewportW{1};
    int m_viewportH{1};

    void destroyGpu();
    void drawObject(const ShaderProgram& program, float x, float y, float w, float h, int objectId);
};

} // namespace acs

#endif
