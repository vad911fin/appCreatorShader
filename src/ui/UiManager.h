#ifndef APP_CREATOR_SHADER_UI_MANAGER_H
#define APP_CREATOR_SHADER_UI_MANAGER_H

#include "core/AppState.h"

#include <functional>
#include <vector>

struct SDL_Window;

namespace acs
{

struct UiCallbacks
{
    std::function<void()> onCompile;
    std::function<void()> onExportCpp;
    std::function<void()> onCopyExport;
    std::function<void()> onSaveHeader;
    std::function<void()> onPresetChanged;
    std::function<void()> onConstructorReset;
    std::function<void()> onSaveUserPresetJson;
    std::function<void()> onLoadUserPresetJson;
    std::function<void()> onEffectLibraryChanged;
};

class UiManager
{
public:
    UiManager();
    ~UiManager();

    UiManager(const UiManager&) = delete;
    UiManager& operator=(const UiManager&) = delete;

    void init(SDL_Window* window, void* glContext);
    void shutdown();

    void beginFrame();
    void draw(AppState& state, const UiCallbacks& cb);
    void endFrame();

private:
    std::vector<char> m_vertexScratch;
    std::vector<char> m_fragmentScratch;
    std::vector<char> m_converterOutScratch;
    int m_convertTarget{0};

    static void syncStringToBuffer(const std::string& s, std::vector<char>& buf);
    static void syncBufferToString(const std::vector<char>& buf, std::string& s);

    void drawShaderConstructorPanel(AppState& state, const UiCallbacks& cb);

    bool m_initialized{false};
};

} // namespace acs

#endif
