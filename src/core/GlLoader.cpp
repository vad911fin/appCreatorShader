#include "core/GlLoader.h"

#if !defined(__ANDROID__)

#include "core/GlEnums.h"

#include <SDL.h>

#include <cstring>
#include <string>

#ifndef APIENTRY
#define APIENTRY
#endif

namespace acs
{

namespace
{

using PFNGLGENVERTEXARRAYSPROC = void(APIENTRY*)(GLsizei, GLuint*);
using PFNGLDELETEVERTEXARRAYSPROC = void(APIENTRY*)(GLsizei, const GLuint*);
using PFNGLBINDVERTEXARRAYPROC = void(APIENTRY*)(GLuint);
using PFNGLGENBUFFERSPROC = void(APIENTRY*)(GLsizei, GLuint*);
using PFNGLDELETEBUFFERSPROC = void(APIENTRY*)(GLsizei, const GLuint*);
using PFNGLBINDBUFFERPROC = void(APIENTRY*)(GLenum, GLuint);
using PFNGLBUFFERDATAPROC = void(APIENTRY*)(GLenum, GLsizeiptr, const void*, GLenum);
using PFNGLENABLEVERTEXATTRIBARRAYPROC = void(APIENTRY*)(GLuint);
using PFNGLDISABLEVERTEXATTRIBARRAYPROC = void(APIENTRY*)(GLuint);
using PFNGLVERTEXATTRIBPOINTERPROC = void(APIENTRY*)(GLuint, GLint, GLenum, GLboolean, GLsizei,
    const void*);
using PFNGLVIEWPORTPROC = void(APIENTRY*)(GLint, GLint, GLsizei, GLsizei);
using PFNGLCLEARCOLORPROC = void(APIENTRY*)(GLfloat, GLfloat, GLfloat, GLfloat);
using PFNGLCLEARPROC = void(APIENTRY*)(GLbitfield);
using PFNGLDRAWARRAYSPROC = void(APIENTRY*)(GLenum, GLint, GLsizei);
using PFNGLUSEPROGRAMPROC = void(APIENTRY*)(GLuint);
using PFNGLCREATESHADERPROC = GLuint(APIENTRY*)(GLenum);
using PFNGLSHADERSOURCEPROC = void(APIENTRY*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
using PFNGLCOMPILESHADERPROC = void(APIENTRY*)(GLuint);
using PFNGLGETSHADERIVPROC = void(APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETSHADERINFOLOGPROC = void(APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLDELETESHADERPROC = void(APIENTRY*)(GLuint);
using PFNGLCREATEPROGRAMPROC = GLuint(APIENTRY*)(void);
using PFNGLATTACHSHADERPROC = void(APIENTRY*)(GLuint, GLuint);
using PFNGLLINKPROGRAMPROC = void(APIENTRY*)(GLuint);
using PFNGLGETPROGRAMIVPROC = void(APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETPROGRAMINFOLOGPROC = void(APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLDELETEPROGRAMPROC = void(APIENTRY*)(GLuint);
using PFNGLGETUNIFORMLOCATIONPROC = GLint(APIENTRY*)(GLuint, const GLchar*);
using PFNGLUNIFORM4FPROC = void(APIENTRY*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
using PFNGLUNIFORM2FPROC = void(APIENTRY*)(GLint, GLfloat, GLfloat);
using PFNGLUNIFORM1IPROC = void(APIENTRY*)(GLint, GLint);
using PFNGLUNIFORMMATRIX4FVPROC = void(APIENTRY*)(GLint, GLsizei, GLboolean, const GLfloat*);
using PFNGLGETERRORPROC = GLenum(APIENTRY*)(void);

PFNGLGENVERTEXARRAYSPROC s_glGenVertexArrays = nullptr;
PFNGLDELETEVERTEXARRAYSPROC s_glDeleteVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC s_glBindVertexArray = nullptr;
PFNGLGENBUFFERSPROC s_glGenBuffers = nullptr;
PFNGLDELETEBUFFERSPROC s_glDeleteBuffers = nullptr;
PFNGLBINDBUFFERPROC s_glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC s_glBufferData = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC s_glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC s_glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC s_glVertexAttribPointer = nullptr;
PFNGLVIEWPORTPROC s_glViewport = nullptr;
PFNGLCLEARCOLORPROC s_glClearColor = nullptr;
PFNGLCLEARPROC s_glClear = nullptr;
PFNGLDRAWARRAYSPROC s_glDrawArrays = nullptr;
PFNGLUSEPROGRAMPROC s_glUseProgram = nullptr;
PFNGLCREATESHADERPROC s_glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC s_glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC s_glCompileShader = nullptr;
PFNGLGETSHADERIVPROC s_glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC s_glGetShaderInfoLog = nullptr;
PFNGLDELETESHADERPROC s_glDeleteShader = nullptr;
PFNGLCREATEPROGRAMPROC s_glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC s_glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC s_glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC s_glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC s_glGetProgramInfoLog = nullptr;
PFNGLDELETEPROGRAMPROC s_glDeleteProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC s_glGetUniformLocation = nullptr;
PFNGLUNIFORM4FPROC s_glUniform4f = nullptr;
PFNGLUNIFORM2FPROC s_glUniform2f = nullptr;
PFNGLUNIFORM1IPROC s_glUniform1i = nullptr;
PFNGLUNIFORMMATRIX4FVPROC s_glUniformMatrix4fv = nullptr;
PFNGLGETERRORPROC s_glGetError = nullptr;

template <typename Fn>
static Fn load(const char* name, std::string& err, bool required)
{
    void* p = SDL_GL_GetProcAddress(name);
    if (p == nullptr)
    {
        if (required)
        {
            err += "Missing GL proc: ";
            err += name;
            err += '\n';
        }
        return nullptr;
    }
    return reinterpret_cast<Fn>(p);
}

} // namespace

GLenum glGetErrorProc(void)
{
    if (s_glGetError == nullptr)
    {
        return GL_NO_ERROR;
    }
    return s_glGetError();
}

void glGenVertexArraysProc(GLsizei n, GLuint* arrays)
{
    s_glGenVertexArrays(n, arrays);
}

void glDeleteVertexArraysProc(GLsizei n, const GLuint* arrays)
{
    s_glDeleteVertexArrays(n, arrays);
}

void glBindVertexArrayProc(GLuint array)
{
    s_glBindVertexArray(array);
}

void glGenBuffersProc(GLsizei n, GLuint* buffers)
{
    s_glGenBuffers(n, buffers);
}

void glDeleteBuffersProc(GLsizei n, const GLuint* buffers)
{
    s_glDeleteBuffers(n, buffers);
}

void glBindBufferProc(GLenum target, GLuint buffer)
{
    s_glBindBuffer(target, buffer);
}

void glBufferDataProc(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    s_glBufferData(target, size, data, usage);
}

void glEnableVertexAttribArrayProc(GLuint index)
{
    s_glEnableVertexAttribArray(index);
}

void glDisableVertexAttribArrayProc(GLuint index)
{
    s_glDisableVertexAttribArray(index);
}

void glVertexAttribPointerProc(GLuint index, GLint size, GLenum type, GLboolean normalized,
    GLsizei stride, const void* pointer)
{
    s_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void glViewportProc(GLint x, GLint y, GLsizei width, GLsizei height)
{
    s_glViewport(x, y, width, height);
}

void glClearColorProc(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    s_glClearColor(r, g, b, a);
}

void glClearProc(GLbitfield mask)
{
    s_glClear(mask);
}

void glDrawArraysProc(GLenum mode, GLint first, GLsizei count)
{
    s_glDrawArrays(mode, first, count);
}

void glUseProgramProc(GLuint program)
{
    s_glUseProgram(program);
}

GLuint glCreateShaderProc(GLenum type)
{
    return s_glCreateShader(type);
}

void glShaderSourceProc(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    s_glShaderSource(shader, count, string, length);
}

void glCompileShaderProc(GLuint shader)
{
    s_glCompileShader(shader);
}

void glGetShaderivProc(GLuint shader, GLenum pname, GLint* params)
{
    s_glGetShaderiv(shader, pname, params);
}

void glGetShaderInfoLogProc(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    s_glGetShaderInfoLog(shader, bufSize, length, infoLog);
}

void glDeleteShaderProc(GLuint shader)
{
    s_glDeleteShader(shader);
}

GLuint glCreateProgramProc(void)
{
    return s_glCreateProgram();
}

void glAttachShaderProc(GLuint program, GLuint shader)
{
    s_glAttachShader(program, shader);
}

void glLinkProgramProc(GLuint program)
{
    s_glLinkProgram(program);
}

void glGetProgramivProc(GLuint program, GLenum pname, GLint* params)
{
    s_glGetProgramiv(program, pname, params);
}

void glGetProgramInfoLogProc(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    s_glGetProgramInfoLog(program, bufSize, length, infoLog);
}

void glDeleteProgramProc(GLuint program)
{
    s_glDeleteProgram(program);
}

GLint glGetUniformLocationProc(GLuint program, const GLchar* name)
{
    return s_glGetUniformLocation(program, name);
}

void glUniform4fProc(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    s_glUniform4f(location, v0, v1, v2, v3);
}

void glUniform2fProc(GLint location, GLfloat v0, GLfloat v1)
{
    s_glUniform2f(location, v0, v1);
}

void glUniform1iProc(GLint location, GLint v0)
{
    s_glUniform1i(location, v0);
}

void glUniformMatrix4fvProc(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    s_glUniformMatrix4fv(location, count, transpose, value);
}

bool loadDesktopGlProcs(std::string& outError)
{
    outError.clear();

    s_glGenVertexArrays = load<PFNGLGENVERTEXARRAYSPROC>("glGenVertexArrays", outError, true);
    s_glDeleteVertexArrays = load<PFNGLDELETEVERTEXARRAYSPROC>("glDeleteVertexArrays", outError, true);
    s_glBindVertexArray = load<PFNGLBINDVERTEXARRAYPROC>("glBindVertexArray", outError, true);
    s_glGenBuffers = load<PFNGLGENBUFFERSPROC>("glGenBuffers", outError, true);
    s_glDeleteBuffers = load<PFNGLDELETEBUFFERSPROC>("glDeleteBuffers", outError, true);
    s_glBindBuffer = load<PFNGLBINDBUFFERPROC>("glBindBuffer", outError, true);
    s_glBufferData = load<PFNGLBUFFERDATAPROC>("glBufferData", outError, true);
    s_glEnableVertexAttribArray = load<PFNGLENABLEVERTEXATTRIBARRAYPROC>("glEnableVertexAttribArray", outError, true);
    s_glDisableVertexAttribArray = load<PFNGLDISABLEVERTEXATTRIBARRAYPROC>("glDisableVertexAttribArray", outError, true);
    s_glVertexAttribPointer = load<PFNGLVERTEXATTRIBPOINTERPROC>("glVertexAttribPointer", outError, true);
    s_glViewport = load<PFNGLVIEWPORTPROC>("glViewport", outError, true);
    s_glClearColor = load<PFNGLCLEARCOLORPROC>("glClearColor", outError, true);
    s_glClear = load<PFNGLCLEARPROC>("glClear", outError, true);
    s_glDrawArrays = load<PFNGLDRAWARRAYSPROC>("glDrawArrays", outError, true);
    s_glUseProgram = load<PFNGLUSEPROGRAMPROC>("glUseProgram", outError, true);
    s_glCreateShader = load<PFNGLCREATESHADERPROC>("glCreateShader", outError, true);
    s_glShaderSource = load<PFNGLSHADERSOURCEPROC>("glShaderSource", outError, true);
    s_glCompileShader = load<PFNGLCOMPILESHADERPROC>("glCompileShader", outError, true);
    s_glGetShaderiv = load<PFNGLGETSHADERIVPROC>("glGetShaderiv", outError, true);
    s_glGetShaderInfoLog = load<PFNGLGETSHADERINFOLOGPROC>("glGetShaderInfoLog", outError, true);
    s_glDeleteShader = load<PFNGLDELETESHADERPROC>("glDeleteShader", outError, true);
    s_glCreateProgram = load<PFNGLCREATEPROGRAMPROC>("glCreateProgram", outError, true);
    s_glAttachShader = load<PFNGLATTACHSHADERPROC>("glAttachShader", outError, true);
    s_glLinkProgram = load<PFNGLLINKPROGRAMPROC>("glLinkProgram", outError, true);
    s_glGetProgramiv = load<PFNGLGETPROGRAMIVPROC>("glGetProgramiv", outError, true);
    s_glGetProgramInfoLog = load<PFNGLGETPROGRAMINFOLOGPROC>("glGetProgramInfoLog", outError, true);
    s_glDeleteProgram = load<PFNGLDELETEPROGRAMPROC>("glDeleteProgram", outError, true);
    s_glGetUniformLocation = load<PFNGLGETUNIFORMLOCATIONPROC>("glGetUniformLocation", outError, true);
    s_glUniform4f = load<PFNGLUNIFORM4FPROC>("glUniform4f", outError, true);
    s_glUniform2f = load<PFNGLUNIFORM2FPROC>("glUniform2f", outError, true);
    s_glUniform1i = load<PFNGLUNIFORM1IPROC>("glUniform1i", outError, true);
    s_glUniformMatrix4fv = load<PFNGLUNIFORMMATRIX4FVPROC>("glUniformMatrix4fv", outError, true);
    s_glGetError = load<PFNGLGETERRORPROC>("glGetError", outError, false);

    return outError.empty();
}

} // namespace acs

#endif
