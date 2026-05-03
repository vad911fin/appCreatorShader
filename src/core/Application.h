#ifndef APP_CREATOR_SHADER_APPLICATION_H
#define APP_CREATOR_SHADER_APPLICATION_H

#include "core/AppState.h"
#include "core/Renderer.h"
#include "core/ShaderProgram.h"
#include "ui/UiManager.h"

struct SDL_Window;

namespace acs
{

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    SDL_Window* m_window{nullptr};
    void* m_glContext{nullptr};
    bool m_running{true};
    bool m_shutdownDone{false};

    AppState m_state;
    ShaderProgram m_program;
    Renderer m_renderer;
    UiManager m_ui;

    bool initSdlAndGl();
    void shutdown();

    void pumpEvents();
    void renderFrame();

    void tryCompile();
    void exportCppSnippet();
    void copyExportToClipboard();
    void saveExportHeader();

    void loadDefaultShadersForState();
};

} // namespace acs

#endif
