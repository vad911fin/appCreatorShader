#include "ui/ui_constructor.h"

#include "effects/effect_registry.h"
#include "ui/UiManager.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace acs
{

namespace
{

const char* categoryPrefix(EffectCategory c)
{
    return c == EffectCategory::Production ? "[P] " : "[E] ";
}

void drawOneParamWidget(EffectRuntimeParams& values, const ParamDescriptor& d)
{
    const std::string& key = d.m_uniformName;
    ImGui::PushID(key.c_str());

    switch (d.m_widget)
    {
    case UiWidgetType::SliderFloat:
        if (d.m_gpuType == ParamGpuType::Float)
        {
            float v = values.getFloat(key, d.m_defaultValue[0]);
            if (ImGui::SliderFloat(d.m_label.c_str(), &v, d.m_min[0], d.m_max[0]))
            {
                values.setFloat(key, v);
            }
        }
        else if (d.m_gpuType == ParamGpuType::Vec4)
        {
            std::array<float, 4> a = values.getVec4(key, d.m_defaultValue);
            if (ImGui::SliderFloat4(d.m_label.c_str(), a.data(), d.m_min[0], d.m_max[0]))
            {
                values.setVec4(key, a);
            }
        }
        else if (d.m_gpuType == ParamGpuType::Vec3)
        {
            std::array<float, 4> a = values.getVec4(key, d.m_defaultValue);
            float x = a[0];
            float y = a[1];
            float z = a[2];
            bool ch = false;
            ch |= ImGui::SliderFloat((d.m_label + " X").c_str(), &x, d.m_min[0], d.m_max[0]);
            ch |= ImGui::SliderFloat((d.m_label + " Y").c_str(), &y, d.m_min[0], d.m_max[0]);
            ch |= ImGui::SliderFloat((d.m_label + " Z").c_str(), &z, d.m_min[0], d.m_max[0]);
            if (ch)
            {
                values.setVec4(key, {x, y, z, a[3]});
            }
        }
        else if (d.m_gpuType == ParamGpuType::Vec2)
        {
            std::array<float, 4> a = values.getVec4(key, d.m_defaultValue);
            float xy[2] = {a[0], a[1]};
            if (ImGui::SliderFloat2(d.m_label.c_str(), xy, d.m_min[0], d.m_max[0]))
            {
                values.setVec4(key, {xy[0], xy[1], a[2], a[3]});
            }
        }
        break;
    case UiWidgetType::SliderInt:
        if (d.m_gpuType == ParamGpuType::Int)
        {
            int v = values.getInt(key, static_cast<int>(d.m_defaultValue[0]));
            const int mn = static_cast<int>(d.m_min[0]);
            const int mx = static_cast<int>(d.m_max[0]);
            if (ImGui::SliderInt(d.m_label.c_str(), &v, mn, mx))
            {
                values.setInt(key, v);
            }
        }
        break;
    case UiWidgetType::Color3: {
        std::array<float, 4> a = values.getVec4(key, d.m_defaultValue);
        float rgb[3] = {a[0], a[1], a[2]};
        if (ImGui::ColorEdit3(d.m_label.c_str(), rgb))
        {
            values.setVec4(key, {rgb[0], rgb[1], rgb[2], a[3]});
        }
        break;
    }
    case UiWidgetType::Color4: {
        std::array<float, 4> a = values.getVec4(key, d.m_defaultValue);
        if (ImGui::ColorEdit4(d.m_label.c_str(), a.data()))
        {
            values.setVec4(key, a);
        }
        break;
    }
    case UiWidgetType::Checkbox:
        if (d.m_gpuType == ParamGpuType::Bool)
        {
            int iv = values.getInt(key, static_cast<int>(d.m_defaultValue[0]));
            bool b = iv != 0;
            if (ImGui::Checkbox(d.m_label.c_str(), &b))
            {
                values.setInt(key, b ? 1 : 0);
            }
        }
        break;
    case UiWidgetType::Combo:
        break;
    }

    ImGui::PopID();
}

} // namespace

void drawEffectLibraryPanel(AppState& state, const UiCallbacks& cb)
{
    EffectRegistry::instance().registerAll();

    ImGui::Separator();
    ImGui::TextUnformatted("Библиотека эффектов (GLSL 3.3 / ES 3.0)");

    if (ImGui::Checkbox("Использовать эффект библиотеки (заменяет FS пресета)", &state.m_effectLibraryActive))
    {
        if (cb.onEffectLibraryChanged)
        {
            cb.onEffectLibraryChanged();
        }
    }

    int tier = static_cast<int>(state.m_perfTier);
    if (ImGui::Combo("PerfTier", &tier, "Low\0Medium\0High\0"))
    {
        state.m_perfTier = static_cast<PerfTier>(std::clamp(tier, 0, 2));
    }

    if (!state.m_effectLibraryActive)
    {
        ImGui::TextDisabled("Включите чекбокс и выберите эффект.");
        return;
    }

    const std::size_t n = EffectRegistry::instance().count();
    if (n == 0U)
    {
        ImGui::TextUnformatted("Реестр пуст.");
        return;
    }

    if (state.m_selectedLibraryEffectIndex < 0 || static_cast<std::size_t>(state.m_selectedLibraryEffectIndex) >= n)
    {
        state.m_selectedLibraryEffectIndex = 0;
    }

    int filterCat = static_cast<int>(state.m_effectLibraryCategoryFilter);
    if (ImGui::Combo("Категория", &filterCat, "Все\0Production\0Experimental\0"))
    {
        state.m_effectLibraryCategoryFilter = static_cast<EffectCategoryFilter>(std::clamp(filterCat, 0, 2));
    }

    std::string comboLabels;
    std::vector<int> indexRemap;
    for (std::size_t i = 0; i < n; ++i)
    {
        const IEffect* e = EffectRegistry::instance().effectAt(i);
        if (e == nullptr)
        {
            continue;
        }
        if (state.m_effectLibraryCategoryFilter == EffectCategoryFilter::Production && e->category() != EffectCategory::Production)
        {
            continue;
        }
        if (state.m_effectLibraryCategoryFilter == EffectCategoryFilter::Experimental && e->category() != EffectCategory::Experimental)
        {
            continue;
        }
        comboLabels += categoryPrefix(e->category());
        comboLabels += e->displayName();
        comboLabels.push_back('\0');
        indexRemap.push_back(static_cast<int>(i));
    }
    comboLabels.push_back('\0');

    if (indexRemap.empty())
    {
        ImGui::TextUnformatted("Нет эффектов в выбранной категории.");
        return;
    }

    const bool selInList = std::find(indexRemap.begin(), indexRemap.end(), state.m_selectedLibraryEffectIndex) != indexRemap.end();
    if (!selInList)
    {
        state.m_selectedLibraryEffectIndex = indexRemap[0];
        const IEffect* ne = EffectRegistry::instance().effectAt(static_cast<std::size_t>(indexRemap[0]));
        if (ne != nullptr)
        {
            state.m_effectRuntimeParams.initDefaults(ne->exposedParams());
        }
        if (cb.onEffectLibraryChanged)
        {
            cb.onEffectLibraryChanged();
        }
    }

    int comboIdx = 0;
    for (std::size_t j = 0; j < indexRemap.size(); ++j)
    {
        if (indexRemap[j] == state.m_selectedLibraryEffectIndex)
        {
            comboIdx = static_cast<int>(j);
            break;
        }
    }

    if (ImGui::Combo("Эффект", &comboIdx, comboLabels.c_str()))
    {
        const int newGlobal = indexRemap[static_cast<std::size_t>(std::clamp(comboIdx, 0, static_cast<int>(indexRemap.size()) - 1))];
        if (newGlobal != state.m_selectedLibraryEffectIndex)
        {
            state.m_selectedLibraryEffectIndex = newGlobal;
            const IEffect* ne = EffectRegistry::instance().effectAt(static_cast<std::size_t>(newGlobal));
            if (ne != nullptr)
            {
                state.m_effectRuntimeParams.initDefaults(ne->exposedParams());
            }
            if (cb.onEffectLibraryChanged)
            {
                cb.onEffectLibraryChanged();
            }
        }
    }

    const IEffect* eff = EffectRegistry::instance().effectAt(static_cast<std::size_t>(state.m_selectedLibraryEffectIndex));
    if (eff == nullptr)
    {
        return;
    }

    ImGui::Text("id: %s", eff->id());
    if (eff->needsDesktopGl43())
    {
        ImGui::TextColored(ImVec4(1.0F, 0.6F, 0.2F, 1.0F), "Требует Desktop GL 4.3+ (заглушка метаданных).");
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Параметры эффекта");
    for (const ParamDescriptor& d : eff->exposedParams())
    {
        drawOneParamWidget(state.m_effectRuntimeParams, d);
    }

    if (ImGui::Button("Сброс параметров эффекта"))
    {
        state.m_effectRuntimeParams.initDefaults(eff->exposedParams());
    }
}

} // namespace acs
