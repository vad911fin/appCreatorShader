#include "converter/ShaderConverter.h"

#include <sstream>

namespace acs
{

namespace
{

void replaceAll(std::string& s, const std::string& from, const std::string& to)
{
    if (from.empty())
    {
        return;
    }
    std::size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos)
    {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}

void replaceVersionTo(const std::string& needleVersion, std::string& s, const std::string& newFirstLines)
{
    const std::size_t at = s.find(needleVersion);
    if (at != std::string::npos)
    {
        const std::size_t lineEnd = s.find('\n', at);
        if (lineEnd != std::string::npos)
        {
            s.replace(at, lineEnd - at + 1U, newFirstLines);
        }
        else
        {
            s.replace(at, std::string::npos, newFirstLines);
        }
        return;
    }

    s.insert(0, newFirstLines);
}

void ensureVertexPrecisionHighp(std::string& s)
{
    if (s.find("precision ") != std::string::npos)
    {
        return;
    }
    const std::size_t p = s.find('\n');
    if (p != std::string::npos)
    {
        s.insert(p + 1U, "precision highp float;\n");
    }
    else
    {
        s.insert(0, "precision highp float;\n");
    }
}

void ensureFragmentPrecisionMediump(std::string& s)
{
    if (s.find("precision ") != std::string::npos)
    {
        return;
    }
    const std::size_t p = s.find('\n');
    if (p != std::string::npos)
    {
        s.insert(p + 1U, "precision mediump float;\n");
    }
    else
    {
        s.insert(0, "precision mediump float;\n");
    }
}

} // namespace

bool ShaderConverter::looksLikeDesktop330(const std::string& src)
{
    return src.find("330") != std::string::npos && src.find("core") != std::string::npos;
}

bool ShaderConverter::looksLikeEs300(const std::string& src)
{
    return src.find("300 es") != std::string::npos;
}

std::string ShaderConverter::desktopGlsl330ToGlslEs300(const std::string& src)
{
    std::string out = src;
    replaceAll(out, "\r\n", "\n");
    replaceVersionTo("#version 330 core", out, "#version 300 es\n");

    if (out.find("#version 300 es") == std::string::npos && looksLikeDesktop330(out))
    {
        replaceVersionTo("#version 330", out, "#version 300 es\n");
    }

    const bool fragmentLike = out.find("fragColor") != std::string::npos || out.find("gl_FragColor") != std::string::npos
        || out.find("FragColor") != std::string::npos || out.find("out vec4") != std::string::npos;

    if (fragmentLike)
    {
        replaceAll(out, "out vec4 FragColor;", "out vec4 fragColor;");
        replaceAll(out, "FragColor", "fragColor");
        replaceAll(out, "gl_FragColor", "fragColor");
        ensureFragmentPrecisionMediump(out);
    }
    else
    {
        ensureVertexPrecisionHighp(out);
    }

    replaceAll(out, "texture2D(", "texture(");
    replaceAll(out, "texture2D (", "texture(");
    return out;
}

std::string ShaderConverter::glslEs300ToDesktopGlsl330(const std::string& src)
{
    std::string out = src;
    replaceAll(out, "\r\n", "\n");
    replaceVersionTo("#version 300 es", out, "#version 330 core\n");

    replaceAll(out, "precision highp float;", "");
    replaceAll(out, "precision mediump float;", "");
    replaceAll(out, "precision lowp float;", "");

    replaceAll(out, "out vec4 fragColor;", "out vec4 FragColor;");
    replaceAll(out, "fragColor", "FragColor");
    replaceAll(out, "\n\n\n", "\n\n");
    return out;
}

std::string ShaderConverter::glslToMslApprox(const std::string& glsl, bool fragmentStage)
{
    std::ostringstream oss;
    oss << "#include <metal_stdlib>\nusing namespace metal;\n\n";
    oss << "// Rule-based preview: verify with SPIRV-Cross / Xcode for production.\n";
    oss << "// texture(sampler2D) in GLSL maps to .sample(sampler, uv) in MSL.\n";
    if (fragmentStage)
    {
        oss << "[[stage(fragment)]]\n";
        oss << "fragment float4 converted_fs_main(/* v2f inputs */) {\n";
        oss << "    // Replace in/out with struct fields; convert vec* -> float*.\n";
        oss << "    return float4(0.0, 0.0, 0.0, 1.0);\n";
        oss << "}\n\n";
    }
    else
    {
        oss << "[[stage(vertex)]]\n";
        oss << "vertex float4 converted_vs_main(/* attributes */ [[stage_in]]) {\n";
        oss << "    return float4(0.0, 0.0, 0.0, 1.0);\n";
        oss << "}\n\n";
    }

    oss << "/* ---- Original GLSL (reference) ----\n";
    oss << glsl << "\n---- */\n";
    return oss.str();
}

std::string ShaderConverter::wrapAsCppRawStringLiterals(const std::string& vertex, const std::string& fragment)
{
    std::ostringstream oss;
    oss << "const char* vertexShaderSrc = R\"glsl(\n";
    oss << vertex;
    if (!vertex.empty() && vertex.back() != '\n')
    {
        oss << '\n';
    }
    oss << ")glsl\";\n\n";
    oss << "const char* fragmentShaderSrc = R\"glsl(\n";
    oss << fragment;
    if (!fragment.empty() && fragment.back() != '\n')
    {
        oss << '\n';
    }
    oss << ")glsl\";\n";
    return oss.str();
}

void ShaderConverter::adaptSourcesForGpuCompile(bool runningOnAndroid, ShaderPlatformChoice editorPlatform,
    std::string& vertexOutInOut, std::string& fragmentOutInOut, std::string& noteOut)
{
    noteOut.clear();
    if (runningOnAndroid)
    {
        if (looksLikeDesktop330(vertexOutInOut))
        {
            vertexOutInOut = desktopGlsl330ToGlslEs300(vertexOutInOut);
            noteOut += "Вершинный шейдер автоматически переведён 330→300 es для Android.\n";
        }
        if (looksLikeDesktop330(fragmentOutInOut))
        {
            fragmentOutInOut = desktopGlsl330ToGlslEs300(fragmentOutInOut);
            noteOut += "Фрагментный шейдер автоматически переведён 330→300 es для Android.\n";
        }
        if (!looksLikeEs300(vertexOutInOut) && !looksLikeDesktop330(vertexOutInOut))
        {
            ensureVertexPrecisionHighp(vertexOutInOut);
        }
        if (!looksLikeEs300(fragmentOutInOut) && !looksLikeDesktop330(fragmentOutInOut))
        {
            ensureFragmentPrecisionMediump(fragmentOutInOut);
        }
        return;
    }

    if (editorPlatform == ShaderPlatformChoice::MobileGlslEs300)
    {
        if (looksLikeEs300(vertexOutInOut))
        {
            vertexOutInOut = glslEs300ToDesktopGlsl330(vertexOutInOut);
            noteOut += "Для предпросмотра на ПК: вершинный шейдер 300 es→330 core.\n";
        }
        if (looksLikeEs300(fragmentOutInOut))
        {
            fragmentOutInOut = glslEs300ToDesktopGlsl330(fragmentOutInOut);
            noteOut += "Для предпросмотра на ПК: фрагментный шейдер 300 es→330 core.\n";
        }
    }
}

} // namespace acs
