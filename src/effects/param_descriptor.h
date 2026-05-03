#ifndef APP_CREATOR_SHADER_EFFECTS_PARAM_DESCRIPTOR_H
#define APP_CREATOR_SHADER_EFFECTS_PARAM_DESCRIPTOR_H

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace acs
{

enum class EffectCategory
{
    Production,
    Experimental
};

enum class PerfTier
{
    Low = 0,
    Medium = 1,
    High = 2
};

enum class ParamGpuType
{
    Float,
    Int,
    Vec2,
    Vec3,
    Vec4,
    Bool
};

enum class UiWidgetType
{
    SliderFloat,
    SliderInt,
    Color3,
    Color4,
    Checkbox,
    Combo
};

struct ParamDescriptor
{
    std::string m_uniformName;
    std::string m_label;
    ParamGpuType m_gpuType{ParamGpuType::Float};
    UiWidgetType m_widget{UiWidgetType::SliderFloat};
    std::array<float, 4> m_defaultValue{};
    std::array<float, 4> m_min{};
    std::array<float, 4> m_max{};
    std::string m_comboItems;
};

class EffectRuntimeParams
{
public:
    void clear();
    void initDefaults(const std::vector<ParamDescriptor>& descriptors);

    [[nodiscard]] float getFloat(const std::string& key, float fallback) const;
    void setFloat(const std::string& key, float v);

    [[nodiscard]] int getInt(const std::string& key, int fallback) const;
    void setInt(const std::string& key, int v);

    [[nodiscard]] std::array<float, 4> getVec4(const std::string& key, const std::array<float, 4>& fb) const;
    void setVec4(const std::string& key, const std::array<float, 4>& v);

    [[nodiscard]] const std::unordered_map<std::string, std::array<float, 4>>& floats() const { return m_floats; }
    [[nodiscard]] const std::unordered_map<std::string, int>& ints() const { return m_ints; }

private:
    std::unordered_map<std::string, std::array<float, 4>> m_floats;
    std::unordered_map<std::string, int> m_ints;
};

} // namespace acs

#endif
