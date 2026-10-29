include(FetchContent)
# Pinned, hash-verified upstream sources. Nothing is downloaded at runtime.
FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz
    URL_HASH SHA256=8e1bbc76c71d74fef2fb85db7e7ca8eba13d6a86623c54992b60162db554ffdb)
# Deterministic path also supplies SDLActivity's Java sources to Gradle.
FetchContent_Declare(SDL2
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.32.10.tar.gz
    URL_HASH SHA256=03f9d7c191a837525c9cda6406af2f2e48be02b5e7eb03d949cc9f1e9ca41c8b
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/out/deps/SDL")
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ${ANDROID} CACHE BOOL "" FORCE)
if(ANDROID)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
else()
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
endif()
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(imgui SDL2)
add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
add_library(imgui_sdl STATIC ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp)
target_include_directories(imgui_sdl PUBLIC ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui_sdl PUBLIC imgui SDL2::SDL2)
