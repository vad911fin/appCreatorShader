#include "core/Application.h"

#include "core/GlDebug.h"
#include "core/GlLoader.h"
#include "core/ShaderDefaults.h"

#include "converter/ShaderConverter.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <SDL.h>

#include <cstdint>
#include <fstream>
#include <utility>

namespace acs
{

Application::Application() = default;

Application::~Application()
{
    if (!m_shutdownDone)
    {
        shutdown();
    }
}

bool Application::initSdlAndGl()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return false;
    }

#if defined(__ANDROID__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    constexpr int kW = 1280;
    constexpr int kH = 720;
    m_window = SDL_CreateWindow("appCreatorShader", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kW, kH,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (m_window == nullptr)
    {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (m_glContext == nullptr)
    {
        SDL_Log("SDL_GL_CreateContext: %s", SDL_GetError());
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        SDL_Quit();
        return false;
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1);

#if !defined(__ANDROID__)
    {
        std::string glErr;
        if (!loadDesktopGlProcs(glErr))
        {
            SDL_Log("loadDesktopGlProcs: %s", glErr.c_str());
            SDL_GL_DeleteContext(m_glContext);
            m_glContext = nullptr;
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            SDL_Quit();
            return false;
        }
    }
#endif

    int dw = 0;
    int dh = 0;
    SDL_GL_GetDrawableSize(m_window, &dw, &dh);
    m_renderer.resizeViewport(dw, dh);

    loadDefaultShadersForState();

    m_ui.init(m_window, m_glContext);

    std::string log;
    if (!m_program.build(m_state.m_vertexSource, m_state.m_fragmentSource, log))
    {
        m_state.m_compileLog = std::move(log);
        m_state.m_shaderOk = false;
    }
    else
    {
        m_state.m_compileLog = "Стартовая сборка шейдера: OK\n";
        m_state.m_shaderOk = true;
    }

    return true;
}

void Application::shutdown()
{
    if (m_shutdownDone)
    {
        return;
    }
    m_shutdownDone = true;

    m_ui.shutdown();
    m_program.release();
    m_renderer = Renderer{};

    if (m_glContext != nullptr)
    {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}

void Application::loadDefaultShadersForState()
{
#if defined(__ANDROID__)
    m_state.m_vertexSource = defaultMobileVertexSource();
    m_state.m_fragmentSource = defaultMobileFragmentSource();
    m_state.m_shaderPlatform = ShaderPlatformChoice::MobileGlslEs300;
#else
    if (m_state.m_shaderPlatform == ShaderPlatformChoice::MobileGlslEs300)
    {
        m_state.m_vertexSource = defaultMobileVertexSource();
        m_state.m_fragmentSource = defaultMobileFragmentSource();
    }
    else
    {
        m_state.m_vertexSource = defaultDesktopVertexSource();
        m_state.m_fragmentSource = defaultDesktopFragmentSource();
    }
#endif
}

void Application::tryCompile()
{
    std::string vs = m_state.m_vertexSource;
    std::string fs = m_state.m_fragmentSource;
    std::string adaptNote;
#if defined(__ANDROID__)
    ShaderConverter::adaptSourcesForGpuCompile(true, m_state.m_shaderPlatform, vs, fs, adaptNote);
#else
    ShaderConverter::adaptSourcesForGpuCompile(false, m_state.m_shaderPlatform, vs, fs, adaptNote);
#endif

    std::string log = adaptNote;
    if (m_program.build(vs, fs, log))
    {
        m_state.m_compileLog = adaptNote + "Компиляция и линковка: OK\n";
        m_state.m_shaderOk = true;
    }
    else
    {
        m_state.m_compileLog = adaptNote + log;
        m_state.m_shaderOk = false;
    }
}

void Application::exportCppSnippet()
{
    m_state.m_lastCppExport = ShaderConverter::wrapAsCppRawStringLiterals(m_state.m_vertexSource, m_state.m_fragmentSource);
    m_state.m_compileLog += "Сгенерирован фрагмент C++ (см. также буфер обмена по кнопке).\n";
}

void Application::copyExportToClipboard()
{
    if (m_state.m_lastCppExport.empty())
    {
        exportCppSnippet();
    }
    ImGui::SetClipboardText(m_state.m_lastCppExport.c_str());
}

void Application::saveExportHeader()
{
    if (m_state.m_lastCppExport.empty())
    {
        exportCppSnippet();
    }
    std::ofstream f(m_state.m_exportHeaderPath.c_str(), std::ios::binary | std::ios::trunc);
    if (!f)
    {
        m_state.m_compileLog += "Ошибка записи файла: " + m_state.m_exportHeaderPath + "\n";
        return;
    }
    f << "#ifndef APP_CREATOR_SHADER_EXPORT_SNIPPET_H\n";
    f << "#define APP_CREATOR_SHADER_EXPORT_SNIPPET_H\n\n";
    f << m_state.m_lastCppExport;
    f << "\n\n#endif\n";
}

void Application::pumpEvents()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&ev);
        if (ev.type == SDL_QUIT)
        {
            m_running = false;
        }
        if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            int dw = 0;
            int dh = 0;
            SDL_GL_GetDrawableSize(m_window, &dw, &dh);
            m_renderer.resizeViewport(dw, dh);
        }
    }
}

void Application::renderFrame()
{
    int dw = 0;
    int dh = 0;
    SDL_GL_GetDrawableSize(m_window, &dw, &dh);
    m_renderer.resizeViewport(dw, dh);

    m_ui.beginFrame();
    m_ui.draw(m_state, [this] { tryCompile(); }, [this] { exportCppSnippet(); }, [this] { copyExportToClipboard(); },
        [this] { saveExportHeader(); });
    m_ui.endFrame();

    glViewport(0, 0, dw, dh);
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_state.m_shaderOk)
    {
        const std::uint32_t ticks = static_cast<std::uint32_t>(SDL_GetTicks());
        m_renderer.render(m_program, m_state.m_skyWidthPx, m_state.m_skyHeightPx, m_state.m_squareWidthPx,
            m_state.m_squareHeightPx, m_state.m_moveSpeed, ticks);
    }

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_window);

    logGlError("after frame");
}

int Application::run()
{
    if (!initSdlAndGl())
    {
        if (!m_shutdownDone)
        {
            shutdown();
        }
        return 1;
    }

    while (m_running)
    {
        pumpEvents();
        renderFrame();
    }

    if (!m_shutdownDone)
    {
        shutdown();
    }
    return 0;
}

} // namespace acs
