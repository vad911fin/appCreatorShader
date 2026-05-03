#ifndef APP_CREATOR_SHADER_SHADER_CONSTRUCTOR_CONFIG_H
#define APP_CREATOR_SHADER_SHADER_CONSTRUCTOR_CONFIG_H

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace acs
{

enum class ExposedParamType
{
    Float,
    Vec2,
    Vec3,
    Vec4,
    Int,
    Bool
};

struct ExposedParamDef
{
    std::string m_uniformName;
    std::string m_label;
    ExposedParamType m_type{ExposedParamType::Float};
    std::array<float, 4> m_defaultValue{};
    std::array<float, 4> m_min{};
    std::array<float, 4> m_max{};
};

struct ShaderConstructorConfig
{
    float m_bgWidth{400.0F};
    float m_bgHeight{400.0F};
    float m_objWidth{64.0F};
    float m_objHeight{64.0F};

    float m_objPosX{0.0F};
    float m_objPosY{0.0F};
    bool m_objPosPixelMode{false};

    float m_animSpeed{1.0F};
    float m_timeScale{1.0F};

    std::array<float, 4> m_bgColor{0.0F, 0.0F, 0.0F, 1.0F};
    std::array<float, 4> m_objColor{0.95F, 0.55F, 0.15F, 1.0F};
    std::array<float, 4> m_glowColor{1.0F, 1.0F, 0.5F, 1.0F};

    float m_intensity{0.5F};
    float m_density{0.3F};
    float m_noiseScale{3.0F};
    float m_edgeSoftness{0.06F};
    float m_extraWarp{0.15F};

    bool m_enableFog{false};
    bool m_enableTrail{false};
    bool m_enableGlow{false};
    bool m_enableDistortion{false};
    bool m_animate{true};

    int m_effectPreset{0};
    int m_blendMode{0};
    int m_noiseType{0};

    std::vector<ExposedParamDef> m_activeExposedDefs;
    std::unordered_map<std::string, std::array<float, 4>> m_exposedValues;
    std::unordered_map<std::string, int> m_exposedInts;

    void resetToDefaults();
    void syncExposedDefaultsFromDefs();
};

} // namespace acs

#endif
