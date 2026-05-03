#ifndef APP_CREATOR_SHADER_SHADER_CONVERTER_H
#define APP_CREATOR_SHADER_SHADER_CONVERTER_H

#include "core/AppState.h"

#include <string>

namespace acs
{

class ShaderConverter
{
public:
    [[nodiscard]] static std::string desktopGlsl330ToGlslEs300(const std::string& src);
    [[nodiscard]] static std::string glslEs300ToDesktopGlsl330(const std::string& src);
    [[nodiscard]] static std::string glslToMslApprox(const std::string& glsl, bool fragmentStage);

    [[nodiscard]] static std::string wrapAsCppRawStringLiterals(const std::string& vertex, const std::string& fragment);

    [[nodiscard]] static bool looksLikeDesktop330(const std::string& src);
    [[nodiscard]] static bool looksLikeEs300(const std::string& src);

    static void adaptSourcesForGpuCompile(bool runningOnAndroid, ShaderPlatformChoice editorPlatform,
        std::string& vertexOutInOut, std::string& fragmentOutInOut, std::string& noteOut);
};

} // namespace acs

#endif
