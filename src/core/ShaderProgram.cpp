#include "core/ShaderProgram.h"

#include "core/GlEnums.h"
#include "core/GlLoader.h"

#include <string>
#include <utility>
#include <vector>

namespace acs
{

namespace
{

constexpr GLsizei kInfoLogMax = 8192;

} // namespace

ShaderProgram::ShaderProgram() = default;

ShaderProgram::~ShaderProgram()
{
    release();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : m_program(other.m_program)
{
    other.m_program = 0U;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    if (this != &other)
    {
        release();
        m_program = other.m_program;
        other.m_program = 0U;
    }
    return *this;
}

void ShaderProgram::release()
{
    if (m_program != 0U)
    {
        glDeleteProgram(m_program);
        m_program = 0U;
    }
}

void ShaderProgram::appendInfoLog(std::string& dst, const char* label, const std::string& msg)
{
    dst += label;
    dst += msg;
    dst += '\n';
}

bool ShaderProgram::compileStage(GLenum type, const std::string& src, GLuint& outShader, std::string& outLog)
{
    outShader = glCreateShader(type);
    if (outShader == 0U)
    {
        appendInfoLog(outLog, "glCreateShader failed (",
            type == GL_VERTEX_SHADER ? std::string("vertex)\n") : std::string("fragment)\n"));
        return false;
    }

    const char* ptr = src.c_str();
    const GLint len = static_cast<GLint>(src.size());
    glShaderSource(outShader, 1, &ptr, &len);
    glCompileShader(outShader);

    GLint ok = 0;
    glGetShaderiv(outShader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE)
    {
        std::vector<GLchar> buf(static_cast<std::size_t>(kInfoLogMax));
        GLsizei written = 0;
        glGetShaderInfoLog(outShader, kInfoLogMax, &written, buf.data());
        appendInfoLog(outLog, "Shader compile error:\n", std::string(buf.data(), static_cast<std::size_t>(written)));
        glDeleteShader(outShader);
        outShader = 0U;
        return false;
    }

    return true;
}

bool ShaderProgram::build(const std::string& vertexSource, const std::string& fragmentSource, std::string& outLog)
{
    outLog.clear();
    release();

    GLuint vs = 0U;
    GLuint fs = 0U;
    if (!compileStage(GL_VERTEX_SHADER, vertexSource, vs, outLog))
    {
        return false;
    }
    if (!compileStage(GL_FRAGMENT_SHADER, fragmentSource, fs, outLog))
    {
        glDeleteShader(vs);
        return false;
    }

    const GLuint prog = glCreateProgram();
    if (prog == 0U)
    {
        appendInfoLog(outLog, "glCreateProgram failed", "");
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE)
    {
        std::vector<GLchar> buf(static_cast<std::size_t>(kInfoLogMax));
        GLsizei written = 0;
        glGetProgramInfoLog(prog, kInfoLogMax, &written, buf.data());
        appendInfoLog(outLog, "Program link error:\n", std::string(buf.data(), static_cast<std::size_t>(written)));
        glDeleteProgram(prog);
        return false;
    }

    m_program = prog;
    return true;
}

void ShaderProgram::bind() const
{
    glUseProgram(m_program);
}

void ShaderProgram::setRectPixels(float x, float y, float w, float h) const
{
    const GLint loc = glGetUniformLocation(m_program, "uRect");
    if (loc >= 0)
    {
        glUniform4f(loc, x, y, w, h);
    }
}

void ShaderProgram::setViewportPixels(float w, float h) const
{
    const GLint loc = glGetUniformLocation(m_program, "uViewport");
    if (loc >= 0)
    {
        glUniform2f(loc, w, h);
    }
}

void ShaderProgram::setObjectId(int objectId) const
{
    const GLint loc = glGetUniformLocation(m_program, "uObjectId");
    if (loc >= 0)
    {
        glUniform1i(loc, objectId);
    }
}

} // namespace acs
