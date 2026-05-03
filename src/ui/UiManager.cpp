#include "ui/UiManager.h"

#include "converter/ShaderConverter.h"
#include "core/PresetLibrary.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <SDL.h>

#include <algorithm>
#include <cstring>
#include <filesystem>

namespace acs
{

namespace
{

constexpr std::size_t kShaderScratch = 256U * 1024U;
constexpr std::size_t kConverterScratch = 512U * 1024U;

void loadFontWithCyrillic(ImGuiIO& io)
{
    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 1;
    const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();

    const char* const paths[] = {
#if defined(_WIN32)
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial Unicode.ttf",
#elif defined(__ANDROID__)
        "/system/fonts/Roboto-Regular.ttf",
        "/system/fonts/NotoSans-Regular.ttf",
        "/system/fonts/DroidSans.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
#endif
    };

    for (const char* const path : paths)
    {
        std::error_code ec;
        if (!std::filesystem::exists(std::filesystem::path(path), ec))
        {
            continue;
        }
        if (io.Fonts->AddFontFromFileTTF(path, 18.0F, &cfg, ranges) != nullptr)
        {
            return;
        }
    }

    io.Fonts->AddFontDefault();
    SDL_Log(
        "ImGui: не найден системный TTF с кириллицей — меню может отображаться как \"?\". Установите шрифт "
        "(например fonts-dejavu-core) или положите .ttf рядом с приложением.");
}

} // namespace

UiManager::UiManager()
{
    m_vertexScratch.resize(kShaderScratch, '\0');
    m_fragmentScratch.resize(kShaderScratch, '\0');
    m_converterOutScratch.resize(kConverterScratch, '\0');
}

UiManager::~UiManager()
{
    shutdown();
}

void UiManager::syncStringToBuffer(const std::string& s, std::vector<char>& buf)
{
    if (buf.empty())
    {
        buf.resize(std::max<std::size_t>(kShaderScratch, s.size() + 1U), '\0');
    }
    const std::size_t n = std::min(s.size(), buf.size() - 1U);
    std::memcpy(buf.data(), s.data(), n);
    buf[n] = '\0';
}

void UiManager::syncBufferToString(const std::vector<char>& buf, std::string& s)
{
    const std::size_t len = strnlen(buf.data(), buf.size());
    s.assign(buf.data(), len);
}

void UiManager::init(SDL_Window* window, void* glContext)
{
    if (m_initialized)
    {
        return;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    loadFontWithCyrillic(io);

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
#if defined(__ANDROID__)
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
    m_initialized = true;
}

void UiManager::shutdown()
{
    if (!m_initialized)
    {
        return;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui::DestroyContext();
    }
    m_initialized = false;
}

void UiManager::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void UiManager::endFrame()
{
    ImGui::Render();
}

void UiManager::draw(AppState& state, const UiCallbacks& cb)
{
    syncStringToBuffer(state.m_vertexSource, m_vertexScratch);
    syncStringToBuffer(state.m_fragmentSource, m_fragmentScratch);
    syncStringToBuffer(state.m_converterOutput, m_converterOutScratch);

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Файл"))
        {
            if (ImGui::MenuItem("Выход", "Alt+F4"))
            {
                SDL_Event ev;
                ev.type = SDL_QUIT;
                SDL_PushEvent(&ev);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Вид"))
        {
            ImGui::MenuItem("Панель визуализации", nullptr, &state.m_showVisualizationPanel);
            ImGui::MenuItem("Редактор шейдеров", nullptr, &state.m_showShaderEditor);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Настройки"))
        {
            if (ImGui::MenuItem("Параметры сцены…"))
            {
                state.m_showSettings = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (state.m_showShaderEditor)
    {
        ImGui::SetNextWindowSize(ImVec2(720.0F, 520.0F), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Редактор шейдеров", &state.m_showShaderEditor))
        {
            if (ImGui::Button("Компилировать"))
            {
                syncBufferToString(m_vertexScratch, state.m_vertexSource);
                syncBufferToString(m_fragmentScratch, state.m_fragmentSource);
                cb.onCompile();
            }
            ImGui::SameLine();
            if (ImGui::Button("Экспорт в C++"))
            {
                syncBufferToString(m_vertexScratch, state.m_vertexSource);
                syncBufferToString(m_fragmentScratch, state.m_fragmentSource);
                cb.onExportCpp();
            }
            ImGui::SameLine();
            if (ImGui::Button("Копировать экспорт"))
            {
                cb.onCopyExport();
            }
            ImGui::SameLine();
            if (ImGui::Button("Сохранить .h"))
            {
                cb.onSaveHeader();
            }

            ImGui::Separator();
            ImGui::TextUnformatted(state.m_shaderOk ? "Статус: OK" : "Статус: ошибка / не компилировали");
            ImGui::BeginChild("compileLog", ImVec2(0.0F, 120.0F), true);
            ImGui::TextUnformatted(state.m_compileLog.c_str());
            ImGui::EndChild();

            ImGui::TextUnformatted("Вершинный шейдер");
            ImGui::InputTextMultiline("##vs", m_vertexScratch.data(), m_vertexScratch.size(), ImVec2(-1.0F, 220.0F));
            ImGui::TextUnformatted("Фрагментный шейдер");
            ImGui::InputTextMultiline("##fs", m_fragmentScratch.data(), m_fragmentScratch.size(), ImVec2(-1.0F, 220.0F));
        }
        ImGui::End();
    }

    if (state.m_showSettings)
    {
        if (ImGui::Begin("Настройки сцены", &state.m_showSettings))
        {
            int plat = (state.m_shaderPlatform == ShaderPlatformChoice::DesktopGlsl330) ? 0 : 1;
            if (ImGui::Combo("Платформа шейдера (редактор)", &plat, "Desktop GLSL 330 core\0Mobile GLSL ES 300\0"))
            {
                state.m_shaderPlatform = plat == 0 ? ShaderPlatformChoice::DesktopGlsl330 : ShaderPlatformChoice::MobileGlslEs300;
            }
            ImGui::TextUnformatted("На ПК предпросмотр всегда через OpenGL 3.3; режим «Mobile» конвертирует в 330 для компиляции.");

            ImGui::Separator();
            ImGui::TextUnformatted("Путь для «Сохранить .h»");
            char pathBuf[512];
            std::memset(pathBuf, 0, sizeof(pathBuf));
            std::strncpy(pathBuf, state.m_exportHeaderPath.c_str(), sizeof(pathBuf) - 1U);
            if (ImGui::InputText("##exportpath", pathBuf, sizeof(pathBuf)))
            {
                state.m_exportHeaderPath.assign(pathBuf);
            }

            char presetPath[512];
            std::memset(presetPath, 0, sizeof(presetPath));
            std::strncpy(presetPath, state.m_presetFilePath.c_str(), sizeof(presetPath) - 1U);
            if (ImGui::InputText("Путь JSON пресета", presetPath, sizeof(presetPath)))
            {
                state.m_presetFilePath.assign(presetPath);
            }

            drawShaderConstructorPanel(state, cb);
        }
        ImGui::End();
    }

    if (state.m_showVisualizationPanel)
    {
        if (ImGui::Begin("Визуализация", &state.m_showVisualizationPanel))
        {
            ImGui::BulletText("Чёрное «небо» — прямоугольник по центру окна.");
            ImGui::BulletText("Квадрат движется по эллиптической траектории внутри неба.");
            ImGui::BulletText("Размеры и скорость задаются в «Настройки сцены…».");
        }
        ImGui::End();
    }

    if (ImGui::Begin("Конвертер шейдеров"))
    {
        const char* items = "Desktop 330 → GLES 300\0GLES 300 → Desktop 330\0В фрагмент MSL (черновик)\0В вершинный MSL (черновик)\0";
        ImGui::Combo("Направление", &m_convertTarget, items);

        if (ImGui::Button("Конвертировать текущие исходники"))
        {
            syncBufferToString(m_vertexScratch, state.m_vertexSource);
            syncBufferToString(m_fragmentScratch, state.m_fragmentSource);
            std::string vs = state.m_vertexSource;
            std::string fs = state.m_fragmentSource;
            switch (m_convertTarget)
            {
            case 0:
                state.m_converterOutput = ShaderConverter::desktopGlsl330ToGlslEs300(vs);
                state.m_converterOutput += "\n// ---- fragment ----\n";
                state.m_converterOutput += ShaderConverter::desktopGlsl330ToGlslEs300(fs);
                break;
            case 1:
                state.m_converterOutput = ShaderConverter::glslEs300ToDesktopGlsl330(vs);
                state.m_converterOutput += "\n// ---- fragment ----\n";
                state.m_converterOutput += ShaderConverter::glslEs300ToDesktopGlsl330(fs);
                break;
            case 2:
                state.m_converterOutput = ShaderConverter::glslToMslApprox(fs, true);
                break;
            default:
                state.m_converterOutput = ShaderConverter::glslToMslApprox(vs, false);
                break;
            }
            syncStringToBuffer(state.m_converterOutput, m_converterOutScratch);
        }
        ImGui::SameLine();
        if (ImGui::Button("В C++ raw strings"))
        {
            syncBufferToString(m_vertexScratch, state.m_vertexSource);
            syncBufferToString(m_fragmentScratch, state.m_fragmentSource);
            state.m_converterOutput = ShaderConverter::wrapAsCppRawStringLiterals(state.m_vertexSource, state.m_fragmentSource);
            syncStringToBuffer(state.m_converterOutput, m_converterOutScratch);
        }

        ImGui::InputTextMultiline("##convOut", m_converterOutScratch.data(), m_converterOutScratch.size(), ImVec2(-1.0F, 260.0F),
            ImGuiInputTextFlags_ReadOnly);
    }
    ImGui::End();

    syncBufferToString(m_vertexScratch, state.m_vertexSource);
    syncBufferToString(m_fragmentScratch, state.m_fragmentSource);
    syncBufferToString(m_converterOutScratch, state.m_converterOutput);
}

void UiManager::drawShaderConstructorPanel(AppState& state, const UiCallbacks& cb)
{
    ImGui::Separator();
    ImGui::TextUnformatted("Конструктор шейдеров");
    ShaderConstructorConfig& g = state.m_constructor;

    ImGui::SliderFloat("bgWidth", &g.m_bgWidth, 16.0F, 1024.0F);
    ImGui::SliderFloat("bgHeight", &g.m_bgHeight, 16.0F, 1024.0F);
    ImGui::SliderFloat("objWidth", &g.m_objWidth, 16.0F, 1024.0F);
    ImGui::SliderFloat("objHeight", &g.m_objHeight, 16.0F, 1024.0F);

    ImGui::SliderFloat("objPosX", &g.m_objPosX, -1.0F, 1.0F);
    ImGui::SliderFloat("objPosY", &g.m_objPosY, -1.0F, 1.0F);
    ImGui::Checkbox("Позиция в пикселях (иначе доля от размера неба)", &g.m_objPosPixelMode);

    ImGui::SliderFloat("animSpeed", &g.m_animSpeed, 0.0F, 5.0F);
    ImGui::SliderFloat("u_timeScale", &g.m_timeScale, 0.0F, 5.0F);

    ImGui::ColorEdit4("bgColor", g.m_bgColor.data());
    ImGui::ColorEdit4("objColor", g.m_objColor.data());
    ImGui::ColorEdit4("glowColor", g.m_glowColor.data());

    ImGui::SliderFloat("intensity", &g.m_intensity, 0.0F, 5.0F);
    ImGui::SliderFloat("density", &g.m_density, 0.0F, 5.0F);
    ImGui::SliderFloat("noiseScale", &g.m_noiseScale, 0.1F, 32.0F);
    ImGui::SliderFloat("edgeSoftness", &g.m_edgeSoftness, 0.001F, 0.5F);

    ImGui::Checkbox("enableFog", &g.m_enableFog);
    ImGui::Checkbox("enableTrail", &g.m_enableTrail);
    ImGui::Checkbox("enableGlow", &g.m_enableGlow);
    ImGui::Checkbox("enableDistortion", &g.m_enableDistortion);
    ImGui::Checkbox("animate", &g.m_animate);

    std::string presetLabels;
    for (const PresetDefinition& p : PresetLibrary::presets())
    {
        presetLabels += p.m_title;
        presetLabels.push_back('\0');
    }
    presetLabels.push_back('\0');
    if (ImGui::Combo("Пресет эффекта", &state.m_selectedPresetIndex, presetLabels.c_str()))
    {
        if (cb.onPresetChanged)
        {
            cb.onPresetChanged();
        }
    }

    const char* blendItems = "Replace\0Additive\0Alpha Blend\0Screen\0";
    ImGui::Combo("Режим наложения", &g.m_blendMode, blendItems);

    const char* noiseItems = "Simplex 2D\0Voronoi\0Perlin\0Gradient\0";
    ImGui::Combo("Тип шума", &g.m_noiseType, noiseItems);

    ImGui::Separator();
    ImGui::TextUnformatted("Доп. параметры пресета");
    for (const ExposedParamDef& def : g.m_activeExposedDefs)
    {
        if (def.m_type == ExposedParamType::Float)
        {
            std::array<float, 4>& slot = g.m_exposedValues[def.m_uniformName];
            ImGui::SliderFloat(def.m_label.c_str(), &slot[0], def.m_min[0], def.m_max[0]);
            if (def.m_uniformName == "u_extraWarp")
            {
                g.m_extraWarp = slot[0];
            }
        }
        else if (def.m_type == ExposedParamType::Bool)
        {
            int v = g.m_exposedInts[def.m_uniformName];
            bool b = v != 0;
            if (ImGui::Checkbox(def.m_label.c_str(), &b))
            {
                g.m_exposedInts[def.m_uniformName] = b ? 1 : 0;
            }
        }
        else if (def.m_type == ExposedParamType::Int)
        {
            ImGui::SliderInt(def.m_label.c_str(), &g.m_exposedInts[def.m_uniformName], static_cast<int>(def.m_min[0]),
                static_cast<int>(def.m_max[0]));
        }
    }

    if (ImGui::Button("Сбросить к дефолту"))
    {
        if (cb.onConstructorReset)
        {
            cb.onConstructorReset();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Сохранить пресет"))
    {
        if (cb.onSaveUserPresetJson)
        {
            cb.onSaveUserPresetJson();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Загрузить из файла"))
    {
        if (cb.onLoadUserPresetJson)
        {
            cb.onLoadUserPresetJson();
        }
    }

    state.syncSceneDimsFromConstructor();
}

} // namespace acs
