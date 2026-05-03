#ifndef APP_CREATOR_SHADER_GL_DEBUG_H
#define APP_CREATOR_SHADER_GL_DEBUG_H

#include "core/GlEnums.h"
#include "core/GlLoader.h"

#include <SDL.h>

namespace acs
{

inline void logGlError(const char* where)
{
    const GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        SDL_Log("%s: GL error 0x%x", where, static_cast<unsigned int>(e));
    }
}

} // namespace acs

#endif
