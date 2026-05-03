#include "core/ShaderConstructorConfig.h"

namespace acs
{

void ShaderConstructorConfig::resetToDefaults()
{
    *this = ShaderConstructorConfig{};
}

void ShaderConstructorConfig::syncExposedDefaultsFromDefs()
{
    for (const ExposedParamDef& def : m_activeExposedDefs)
    {
        if (def.m_type == ExposedParamType::Int)
        {
            m_exposedInts[def.m_uniformName] = static_cast<int>(def.m_defaultValue[0]);
        }
        else if (def.m_type == ExposedParamType::Bool)
        {
            m_exposedInts[def.m_uniformName] = def.m_defaultValue[0] > 0.5F ? 1 : 0;
        }
        else
        {
            m_exposedValues[def.m_uniformName] = def.m_defaultValue;
        }
    }
}

} // namespace acs
