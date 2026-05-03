#include "core/UniformRegistry.h"

#include "core/GlLoader.h"

#include <SDL.h>

namespace acs
{

void UniformRegistry::setProgram(GLuint program)
{
    m_program = program;
    m_locations.clear();
    m_warnedMissing.clear();
}

void UniformRegistry::clear()
{
    m_program = 0U;
    m_locations.clear();
    m_warnedMissing.clear();
}

GLint UniformRegistry::location(const char* name)
{
    if (m_program == 0U)
    {
        return -1;
    }
    const std::string key(name);
    const auto it = m_locations.find(key);
    if (it != m_locations.end())
    {
        return it->second;
    }
    const GLint loc = glGetUniformLocation(m_program, name);
    m_locations.emplace(key, loc);
    return loc;
}

void UniformRegistry::warnMissing(const char* name)
{
    if (m_warnedMissing.insert(name).second)
    {
        SDL_Log("UniformRegistry: униформ «%s» не найден в текущей программе (пропуск).", name);
    }
}

void UniformRegistry::setFloat(const char* name, float v)
{
    const GLint loc = location(name);
    if (loc < 0)
    {
        warnMissing(name);
        return;
    }
    glUniform1f(loc, v);
}

void UniformRegistry::setVec2(const char* name, float x, float y)
{
    const GLint loc = location(name);
    if (loc < 0)
    {
        warnMissing(name);
        return;
    }
    glUniform2f(loc, x, y);
}

void UniformRegistry::setVec3(const char* name, float x, float y, float z)
{
    const GLint loc = location(name);
    if (loc < 0)
    {
        warnMissing(name);
        return;
    }
    glUniform3f(loc, x, y, z);
}

void UniformRegistry::setVec4(const char* name, float x, float y, float z, float w)
{
    const GLint loc = location(name);
    if (loc < 0)
    {
        warnMissing(name);
        return;
    }
    glUniform4f(loc, x, y, z, w);
}

void UniformRegistry::setInt(const char* name, int v)
{
    const GLint loc = location(name);
    if (loc < 0)
    {
        warnMissing(name);
        return;
    }
    glUniform1i(loc, v);
}

void UniformRegistry::setBool(const char* name, bool v)
{
    setInt(name, v ? 1 : 0);
}

void UniformRegistry::setSampler2D(const char* name, int textureUnit)
{
    setInt(name, textureUnit);
}

void UniformRegistry::applySystemUniforms(
    float timeSec, float resolutionX, float resolutionY, float viewportW, float viewportH)
{
    setFloat("u_time", timeSec);
    setVec2("u_resolution", resolutionX, resolutionY);
    setVec4("u_viewport", 0.0F, 0.0F, viewportW, viewportH);
    setVec2("uViewport", viewportW, viewportH);
}

void UniformRegistry::applyConstructorConfig(const ShaderConstructorConfig& cfg)
{
    setFloat("u_bgWidth", cfg.m_bgWidth);
    setFloat("u_bgHeight", cfg.m_bgHeight);
    setFloat("u_objWidth", cfg.m_objWidth);
    setFloat("u_objHeight", cfg.m_objHeight);
    setFloat("u_objPosX", cfg.m_objPosX);
    setFloat("u_objPosY", cfg.m_objPosY);
    setInt("u_objPosPixelMode", cfg.m_objPosPixelMode ? 1 : 0);
    setFloat("u_animSpeed", cfg.m_animSpeed);
    setFloat("u_timeScale", cfg.m_timeScale);
    setVec4("u_bgColor", cfg.m_bgColor[0], cfg.m_bgColor[1], cfg.m_bgColor[2], cfg.m_bgColor[3]);
    setVec4("u_objColor", cfg.m_objColor[0], cfg.m_objColor[1], cfg.m_objColor[2], cfg.m_objColor[3]);
    setVec4("u_glowColor", cfg.m_glowColor[0], cfg.m_glowColor[1], cfg.m_glowColor[2], cfg.m_glowColor[3]);
    setFloat("u_intensity", cfg.m_intensity);
    setFloat("u_density", cfg.m_density);
    setFloat("u_noiseScale", cfg.m_noiseScale);
    setFloat("u_edgeSoftness", cfg.m_edgeSoftness);
    setInt("u_enableFog", cfg.m_enableFog ? 1 : 0);
    setInt("u_enableTrail", cfg.m_enableTrail ? 1 : 0);
    setInt("u_enableGlow", cfg.m_enableGlow ? 1 : 0);
    setInt("u_enableDistortion", cfg.m_enableDistortion ? 1 : 0);
    setInt("u_animate", cfg.m_animate ? 1 : 0);
    setInt("u_effectPreset", cfg.m_effectPreset);
    setInt("u_blendMode", cfg.m_blendMode);
    setInt("u_noiseType", cfg.m_noiseType);

    for (const auto& kv : cfg.m_exposedValues)
    {
        const ExposedParamDef* def = nullptr;
        for (const ExposedParamDef& d : cfg.m_activeExposedDefs)
        {
            if (d.m_uniformName == kv.first)
            {
                def = &d;
                break;
            }
        }
        const GLint loc = location(kv.first.c_str());
        if (loc < 0)
        {
            warnMissing(kv.first.c_str());
            continue;
        }
        const std::array<float, 4> v = kv.second;
        if (def == nullptr)
        {
            glUniform1f(loc, v[0]);
            continue;
        }
        switch (def->m_type)
        {
        case ExposedParamType::Float:
            glUniform1f(loc, v[0]);
            break;
        case ExposedParamType::Vec2:
            glUniform2f(loc, v[0], v[1]);
            break;
        case ExposedParamType::Vec3:
            glUniform3f(loc, v[0], v[1], v[2]);
            break;
        case ExposedParamType::Vec4:
            glUniform4f(loc, v[0], v[1], v[2], v[3]);
            break;
        default:
            break;
        }
    }

    for (const auto& kv : cfg.m_exposedInts)
    {
        const GLint loc = location(kv.first.c_str());
        if (loc < 0)
        {
            warnMissing(kv.first.c_str());
            continue;
        }
        glUniform1i(loc, kv.second);
    }

    float extraWarp = cfg.m_extraWarp;
    const auto ewIt = cfg.m_exposedValues.find("u_extraWarp");
    if (ewIt != cfg.m_exposedValues.end())
    {
        extraWarp = ewIt->second[0];
    }
    setFloat("u_extraWarp", extraWarp);
}

} // namespace acs
