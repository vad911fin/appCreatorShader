#ifndef APP_CREATOR_SHADER_GL_INCLUDES_H
#define APP_CREATOR_SHADER_GL_INCLUDES_H

#if defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include <cstdint>
using GLbyte = std::int8_t;
using GLubyte = std::uint8_t;
using GLshort = std::int16_t;
using GLushort = std::uint16_t;
using GLint = std::int32_t;
using GLuint = std::uint32_t;
using GLsizei = std::int32_t;
using GLfloat = float;
using GLdouble = double;
using GLboolean = unsigned char;
using GLenum = std::uint32_t;
using GLintptr = std::intptr_t;
using GLsizeiptr = std::ptrdiff_t;
using GLchar = char;
using GLbitfield = std::uint32_t;
#endif

#endif
