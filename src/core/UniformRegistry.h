#ifndef APP_CREATOR_SHADER_UNIFORM_REGISTRY_H
#define APP_CREATOR_SHADER_UNIFORM_REGISTRY_H

#include "core/GlIncludes.h"
#include "core/ShaderConstructorConfig.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace acs
{

class UniformRegistry
{
public:
    void setProgram(GLuint program);
    void clear();

    void applySystemUniforms(float timeSec, float resolutionX, float resolutionY, float viewportW, float viewportH);
    void applyConstructorConfig(const ShaderConstructorConfig& cfg);

    void setFloat(const char* name, float v);
    void setVec2(const char* name, float x, float y);
    void setVec3(const char* name, float x, float y, float z);
    void setVec4(const char* name, float x, float y, float z, float w);
    void setInt(const char* name, int v);
    void setBool(const char* name, bool v);
    void setSampler2D(const char* name, int textureUnit);

private:
    GLuint m_program{0U};
    std::unordered_map<std::string, GLint> m_locations;
    std::unordered_set<std::string> m_warnedMissing;

    GLint location(const char* name);
    void warnMissing(const char* name);
};

} // namespace acs

#endif
