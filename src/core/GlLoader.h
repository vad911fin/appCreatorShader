#ifndef APP_CREATOR_SHADER_GL_LOADER_H
#define APP_CREATOR_SHADER_GL_LOADER_H

#include "core/GlIncludes.h"

#include <string>

#if !defined(__ANDROID__)

namespace acs
{

bool loadDesktopGlProcs(std::string& outError);

GLenum glGetErrorProc(void);
void glGenVertexArraysProc(GLsizei n, GLuint* arrays);
void glDeleteVertexArraysProc(GLsizei n, const GLuint* arrays);
void glBindVertexArrayProc(GLuint array);
void glGenBuffersProc(GLsizei n, GLuint* buffers);
void glDeleteBuffersProc(GLsizei n, const GLuint* buffers);
void glBindBufferProc(GLenum target, GLuint buffer);
void glBufferDataProc(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
void glEnableVertexAttribArrayProc(GLuint index);
void glDisableVertexAttribArrayProc(GLuint index);
void glVertexAttribPointerProc(GLuint index, GLint size, GLenum type, GLboolean normalized,
    GLsizei stride, const void* pointer);
void glViewportProc(GLint x, GLint y, GLsizei width, GLsizei height);
void glClearColorProc(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void glClearProc(GLbitfield mask);
void glDrawArraysProc(GLenum mode, GLint first, GLsizei count);
void glUseProgramProc(GLuint program);
GLuint glCreateShaderProc(GLenum type);
void glShaderSourceProc(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
void glCompileShaderProc(GLuint shader);
void glGetShaderivProc(GLuint shader, GLenum pname, GLint* params);
void glGetShaderInfoLogProc(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
void glDeleteShaderProc(GLuint shader);
GLuint glCreateProgramProc(void);
void glAttachShaderProc(GLuint program, GLuint shader);
void glLinkProgramProc(GLuint program);
void glGetProgramivProc(GLuint program, GLenum pname, GLint* params);
void glGetProgramInfoLogProc(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
void glDeleteProgramProc(GLuint program);
GLint glGetUniformLocationProc(GLuint program, const GLchar* name);
void glUniform1fProc(GLint location, GLfloat v0);
void glUniform3fProc(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void glUniform4fProc(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void glUniform2fProc(GLint location, GLfloat v0, GLfloat v1);
void glUniform1iProc(GLint location, GLint v0);
void glUniformMatrix4fvProc(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

#define glGetError glGetErrorProc
#define glGenVertexArrays glGenVertexArraysProc
#define glDeleteVertexArrays glDeleteVertexArraysProc
#define glBindVertexArray glBindVertexArrayProc
#define glGenBuffers glGenBuffersProc
#define glDeleteBuffers glDeleteBuffersProc
#define glBindBuffer glBindBufferProc
#define glBufferData glBufferDataProc
#define glEnableVertexAttribArray glEnableVertexAttribArrayProc
#define glDisableVertexAttribArray glDisableVertexAttribArrayProc
#define glVertexAttribPointer glVertexAttribPointerProc
#define glViewport glViewportProc
#define glClearColor glClearColorProc
#define glClear glClearProc
#define glDrawArrays glDrawArraysProc
#define glUseProgram glUseProgramProc
#define glCreateShader glCreateShaderProc
#define glShaderSource glShaderSourceProc
#define glCompileShader glCompileShaderProc
#define glGetShaderiv glGetShaderivProc
#define glGetShaderInfoLog glGetShaderInfoLogProc
#define glDeleteShader glDeleteShaderProc
#define glCreateProgram glCreateProgramProc
#define glAttachShader glAttachShaderProc
#define glLinkProgram glLinkProgramProc
#define glGetProgramiv glGetProgramivProc
#define glGetProgramInfoLog glGetProgramInfoLogProc
#define glDeleteProgram glDeleteProgramProc
#define glGetUniformLocation glGetUniformLocationProc
#define glUniform1f glUniform1fProc
#define glUniform3f glUniform3fProc
#define glUniform4f glUniform4fProc
#define glUniform2f glUniform2fProc
#define glUniform1i glUniform1iProc
#define glUniformMatrix4fv glUniformMatrix4fvProc

} // namespace acs

#else

// Android: GLES symbols linked at build time; macros not used.
#include <GLES3/gl3.h>

#endif

#endif
