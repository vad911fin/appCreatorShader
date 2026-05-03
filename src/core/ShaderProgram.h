#ifndef APP_CREATOR_SHADER_SHADER_PROGRAM_H
#define APP_CREATOR_SHADER_SHADER_PROGRAM_H

#include "core/GlIncludes.h"

#include <string>

namespace acs
{

class ShaderProgram
{
public:
    ShaderProgram();
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    [[nodiscard]] bool build(const std::string& vertexSource, const std::string& fragmentSource,
        std::string& outLog);

    void bind() const;
    void release();

    void setRectPixels(float x, float y, float w, float h) const;
    void setViewportPixels(float w, float h) const;
    void setObjectId(int objectId) const;

    [[nodiscard]] bool valid() const { return m_program != 0U; }

private:
    GLuint m_program{0};

    static bool compileStage(GLenum type, const std::string& src, GLuint& outShader, std::string& outLog);
    static void appendInfoLog(std::string& dst, const char* label, const std::string& msg);
};

} // namespace acs

#endif
