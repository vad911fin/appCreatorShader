#ifndef APP_CREATOR_SHADER_APP_STATE_H
#define APP_CREATOR_SHADER_APP_STATE_H

#include "core/ShaderConstructorConfig.h"

#include <string>

namespace acs
{

enum class ShaderPlatformChoice
{
    DesktopGlsl330,
    MobileGlslEs300
};

struct AppState
{
    std::string m_vertexSource;
    std::string m_fragmentSource;
    std::string m_compileLog;
    bool m_shaderOk{false};

    float m_skyWidthPx{400.0F};
    float m_skyHeightPx{400.0F};
    float m_squareWidthPx{64.0F};
    float m_squareHeightPx{64.0F};
    float m_moveSpeed{1.0F};

    ShaderConstructorConfig m_constructor;
    int m_selectedPresetIndex{0};
    std::string m_presetFilePath{"presets_user.json"};

    ShaderPlatformChoice m_shaderPlatform{ShaderPlatformChoice::DesktopGlsl330};
    bool m_showSettings{false};
    bool m_showShaderEditor{true};
    bool m_showVisualizationPanel{true};

    std::string m_exportHeaderPath{"export_shader.hpp"};
    std::string m_converterOutput;
    std::string m_lastCppExport;

    void syncSceneDimsFromConstructor();
    void syncConstructorDimsFromScene();
};

} // namespace acs

#endif
