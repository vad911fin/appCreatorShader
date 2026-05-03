#ifndef APP_CREATOR_SHADER_PRESET_LIBRARY_H
#define APP_CREATOR_SHADER_PRESET_LIBRARY_H

#include "core/ShaderConstructorConfig.h"

#include <string>
#include <vector>

namespace acs
{

struct AppState;

struct PresetDefinition
{
    std::string m_id;
    std::string m_title;
    int m_effectPresetIndex{0};
    std::vector<ExposedParamDef> m_exposed;
    std::string m_fragmentOverride;
};

class PresetLibrary
{
public:
    [[nodiscard]] static const std::vector<PresetDefinition>& presets();

    static void applyPresetByIndex(AppState& state, int index);
    static bool tryLoadUserPresetJson(AppState& state, const std::string& path, std::string& outLog);
    static bool saveCurrentPresetToJsonFile(const AppState& state, const std::string& path, std::string& outLog);
};

} // namespace acs

#endif
