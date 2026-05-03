#ifndef APP_CREATOR_SHADER_UI_MANAGER_H
#define APP_CREATOR_SHADER_UI_MANAGER_H

#include "core/AppState.h"

#include <functional>
#include <vector>

struct SDL_Window;

namespace acs
{

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
    void draw(AppState& state, const std::function<void()>& onCompile, const std::function<void()>& onExportCpp,
        const std::function<void()>& onCopyExport, const std::function<void()>& onSaveHeader);
    void endFrame();

private:
    std::vector<char> m_vertexScratch;
    std::vector<char> m_fragmentScratch;
    std::vector<char> m_converterOutScratch;
    int m_convertTarget{0};

    static void syncStringToBuffer(const std::string& s, std::vector<char>& buf);
    static void syncBufferToString(const std::vector<char>& buf, std::string& s);

    bool m_initialized{false};
};

} // namespace acs

#endif
