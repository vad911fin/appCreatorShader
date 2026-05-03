#include "core/PresetLibrary.h"

#include "core/AppState.h"
#include "core/ShaderDefaults.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>

namespace acs
{

namespace
{

std::vector<PresetDefinition> makeBuiltins()
{
    std::vector<PresetDefinition> out;
    out.push_back(PresetDefinition{"none", "Нет", 0, std::vector<ExposedParamDef>{}, ""});
    out.push_back(PresetDefinition{"smoke", "Дым", 1, std::vector<ExposedParamDef>{}, ""});
    out.push_back(PresetDefinition{"fog", "Туман", 2, std::vector<ExposedParamDef>{}, ""});
    out.push_back(PresetDefinition{"trail", "Шлейф", 3, std::vector<ExposedParamDef>{}, ""});
    out.push_back(PresetDefinition{"glow", "Свечение", 4, std::vector<ExposedParamDef>{}, ""});
    out.push_back(PresetDefinition{"distort", "Искажение", 5, std::vector<ExposedParamDef>{}, ""});
    out.push_back(PresetDefinition{"cell", "Клеточный шум", 6, std::vector<ExposedParamDef>{}, ""});

    ExposedParamDef extra{};
    extra.m_uniformName = "u_extraWarp";
    extra.m_label = "Доп. искажение UV";
    extra.m_type = ExposedParamType::Float;
    extra.m_defaultValue = {0.15F, 0.0F, 0.0F, 0.0F};
    extra.m_min = {0.0F, 0.0F, 0.0F, 0.0F};
    extra.m_max = {1.0F, 0.0F, 0.0F, 0.0F};
    out[5].m_exposed.push_back(extra);
    return out;
}

bool readIntAfterKey(const std::string& text, const char* key, int& outVal)
{
    const std::size_t p = text.find(key);
    if (p == std::string::npos)
    {
        return false;
    }
    std::size_t i = p + strlen(key);
    while (i < text.size() && (text[i] == ' ' || text[i] == '\t' || text[i] == ':'))
    {
        ++i;
    }
    int sign = 1;
    if (i < text.size() && text[i] == '-')
    {
        sign = -1;
        ++i;
    }
    int v = 0;
    bool any = false;
    while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])))
    {
        any = true;
        v = v * 10 + (text[i] - '0');
        ++i;
    }
    if (!any)
    {
        return false;
    }
    outVal = v * sign;
    return true;
}

} // namespace

const std::vector<PresetDefinition>& PresetLibrary::presets()
{
    static const std::vector<PresetDefinition> k = makeBuiltins();
    return k;
}

void PresetLibrary::applyPresetByIndex(AppState& state, int index)
{
    const auto& list = presets();
    if (index < 0 || index >= static_cast<int>(list.size()))
    {
        return;
    }
    const PresetDefinition& def = list[static_cast<std::size_t>(index)];
    state.m_selectedPresetIndex = index;
    state.m_constructor.m_effectPreset = def.m_effectPresetIndex;
    state.m_constructor.m_activeExposedDefs = def.m_exposed;
    state.m_constructor.syncExposedDefaultsFromDefs();
    if (!def.m_fragmentOverride.empty())
    {
        state.m_fragmentSource = def.m_fragmentOverride;
    }
    else
    {
#if defined(__ANDROID__)
        state.m_fragmentSource = defaultMobileConstructorFragment();
#else
        if (state.m_shaderPlatform == ShaderPlatformChoice::MobileGlslEs300)
        {
            state.m_fragmentSource = defaultMobileConstructorFragment();
        }
        else
        {
            state.m_fragmentSource = defaultDesktopConstructorFragment();
        }
#endif
    }
}

bool PresetLibrary::tryLoadUserPresetJson(AppState& state, const std::string& path, std::string& outLog)
{
    outLog.clear();
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f)
    {
        outLog = "Не удалось открыть файл: " + path;
        return false;
    }
    std::stringstream buf;
    buf << f.rdbuf();
    const std::string json = buf.str();
    int effect = 0;
    if (!readIntAfterKey(json, "\"effectPreset\"", effect))
    {
        outLog = "JSON: не найдено поле effectPreset (ожидается \"effectPreset\": <int>).";
        return false;
    }
    const auto& list = presets();
    int chosen = 0;
    for (int i = 0; i < static_cast<int>(list.size()); ++i)
    {
        if (list[static_cast<std::size_t>(i)].m_effectPresetIndex == effect)
        {
            chosen = i;
            break;
        }
    }
    applyPresetByIndex(state, chosen);
    outLog = "Загружен пресет, effectPreset=" + std::to_string(effect);
    return true;
}

bool PresetLibrary::saveCurrentPresetToJsonFile(const AppState& state, const std::string& path, std::string& outLog)
{
    outLog.clear();
    std::ofstream f(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!f)
    {
        outLog = "Не удалось записать: " + path;
        return false;
    }
    const ShaderConstructorConfig& c = state.m_constructor;
    f << "{\n";
    f << "  \"name\": \"user_preset\",\n";
    f << "  \"effectPreset\": " << c.m_effectPreset << ",\n";
    f << "  \"blendMode\": " << c.m_blendMode << ",\n";
    f << "  \"noiseType\": " << c.m_noiseType << ",\n";
    f << "  \"u_bgWidth\": " << c.m_bgWidth << ",\n";
    f << "  \"u_bgHeight\": " << c.m_bgHeight << ",\n";
    f << "  \"u_objWidth\": " << c.m_objWidth << ",\n";
    f << "  \"u_objHeight\": " << c.m_objHeight << "\n";
    f << "}\n";
    outLog = "Сохранено в " + path;
    return true;
}

} // namespace acs
