#include "effects/param_descriptor.h"

namespace acs
{

void EffectRuntimeParams::clear()
{
    m_floats.clear();
    m_ints.clear();
}

void EffectRuntimeParams::initDefaults(const std::vector<ParamDescriptor>& descriptors)
{
    clear();
    for (const ParamDescriptor& d : descriptors)
    {
        if (d.m_gpuType == ParamGpuType::Int || d.m_gpuType == ParamGpuType::Bool)
        {
            m_ints[d.m_uniformName] = static_cast<int>(d.m_defaultValue[0]);
        }
        else
        {
            m_floats[d.m_uniformName] = d.m_defaultValue;
        }
    }
}

float EffectRuntimeParams::getFloat(const std::string& key, float fallback) const
{
    const auto it = m_floats.find(key);
    if (it == m_floats.end())
    {
        return fallback;
    }
    return it->second[0];
}

void EffectRuntimeParams::setFloat(const std::string& key, float v)
{
    std::array<float, 4> a{};
    a[0] = v;
    m_floats[key] = a;
}

int EffectRuntimeParams::getInt(const std::string& key, int fallback) const
{
    const auto it = m_ints.find(key);
    if (it == m_ints.end())
    {
        return fallback;
    }
    return it->second;
}

void EffectRuntimeParams::setInt(const std::string& key, int v)
{
    m_ints[key] = v;
}

std::array<float, 4> EffectRuntimeParams::getVec4(const std::string& key, const std::array<float, 4>& fb) const
{
    const auto it = m_floats.find(key);
    if (it == m_floats.end())
    {
        return fb;
    }
    return it->second;
}

void EffectRuntimeParams::setVec4(const std::string& key, const std::array<float, 4>& v)
{
    m_floats[key] = v;
}

} // namespace acs
