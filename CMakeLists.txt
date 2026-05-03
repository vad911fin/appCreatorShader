cmake_minimum_required(VERSION 3.24)
project(appCreatorShader LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LIBS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/libs")

include(FetchContent)

# --- SDL2: prefer готовая копия в src/libs/SDL2, иначе FetchContent, иначе find_package ---
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

if(EXISTS "${LIBS_ROOT}/SDL2/CMakeLists.txt")
  message(STATUS "SDL2: локальная копия ${LIBS_ROOT}/SDL2")
  add_subdirectory("${LIBS_ROOT}/SDL2" "${CMAKE_BINARY_DIR}/_build/SDL2")
elseif(NOT ANDROID)
  find_package(SDL2 2.0.18 QUIET CONFIG)
endif()

if(NOT TARGET SDL2::SDL2)
  message(STATUS "SDL2: FetchContent → ${LIBS_ROOT}/SDL2 (нужен git/сеть)")
  FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.10
    SOURCE_DIR "${LIBS_ROOT}/SDL2"
    BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/SDL2-build"
    SUBBUILD_DIR "${CMAKE_BINARY_DIR}/_deps/SDL2-subbuild"
  )
  FetchContent_MakeAvailable(SDL2)
endif()

if(NOT TARGET SDL2::SDL2)
  message(FATAL_ERROR "SDL2 не найден: положите исходники в src/libs/SDL2 или установите libsdl2-dev и пересоберите.")
endif()

# --- Dear ImGui ---
if(EXISTS "${LIBS_ROOT}/imgui/imgui.cpp")
  message(STATUS "ImGui: локальная копия ${LIBS_ROOT}/imgui")
else()
  message(STATUS "ImGui: FetchContent → ${LIBS_ROOT}/imgui")
  FetchContent_Declare(
    imgui_fc
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
    SOURCE_DIR "${LIBS_ROOT}/imgui"
    BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/imgui-build"
    SUBBUILD_DIR "${CMAKE_BINARY_DIR}/_deps/imgui-subbuild"
  )
  FetchContent_MakeAvailable(imgui_fc)
endif()

if(NOT EXISTS "${LIBS_ROOT}/imgui/imgui.cpp")
  message(FATAL_ERROR "ImGui не найден: положите репозиторий в src/libs/imgui или включите сеть для FetchContent.")
endif()

set(IMGUI_DIR "${LIBS_ROOT}/imgui")
set(IMGUI_SOURCES
  "${IMGUI_DIR}/imgui.cpp"
  "${IMGUI_DIR}/imgui_draw.cpp"
  "${IMGUI_DIR}/imgui_tables.cpp"
  "${IMGUI_DIR}/imgui_widgets.cpp"
  "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
  "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

set(APP_SOURCES
  main.cpp
  src/core/AppState.cpp
  src/core/Application.cpp
  src/core/GlLoader.cpp
  src/core/PresetLibrary.cpp
  src/core/ShaderConstructorConfig.cpp
  src/core/ShaderDefaults.cpp
  src/core/ShaderExport.cpp
  src/core/ShaderProgram.cpp
  src/core/Renderer.cpp
  src/core/UniformRegistry.cpp
  src/ui/UiManager.cpp
  src/converter/ShaderConverter.cpp
  ${IMGUI_SOURCES}
)

function(acs_link_sdl2_main target)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(${target} PRIVATE SDL2::SDL2main)
  endif()
endfunction()

if(ANDROID)
  add_library(main SHARED ${APP_SOURCES})
  target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
  )
  target_compile_definitions(main PRIVATE __ANDROID__ IMGUI_IMPL_OPENGL_ES3)
  target_link_libraries(main PRIVATE SDL2::SDL2 GLESv3 EGL android log)
  acs_link_sdl2_main(main)
  set_target_properties(main PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
else()
  add_executable(appCreatorShader ${APP_SOURCES})
  target_include_directories(appCreatorShader PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
  )
  if(WIN32)
    target_compile_options(appCreatorShader PRIVATE /utf-8)
    target_link_libraries(appCreatorShader PRIVATE SDL2::SDL2 opengl32)
    acs_link_sdl2_main(appCreatorShader)
  elseif(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
    target_link_libraries(appCreatorShader PRIVATE
      SDL2::SDL2
      ${COCOA_FRAMEWORK}
      ${IOKIT_FRAMEWORK}
      ${OPENGL_FRAMEWORK}
    )
    acs_link_sdl2_main(appCreatorShader)
  else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(appCreatorShader PRIVATE SDL2::SDL2 OpenGL::GL)
    acs_link_sdl2_main(appCreatorShader)
  endif()
  set_target_properties(appCreatorShader PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
endif()
