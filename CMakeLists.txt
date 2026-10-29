# CMakeList.txt : CMake project for Chess, include source and define
# project specific logic here.
cmake_minimum_required (VERSION 3.16)

project("Chess")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(CHESS_BUILD_MOBILE_PREVIEW "Build the Android interface as a desktop preview" OFF)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Concurrent)
if(ANDROID OR CHESS_BUILD_MOBILE_PREVIEW)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2)
endif()
if(NOT ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
endif()
qt_standard_project_setup()

option(CHESS_ENABLE_COVERAGE "Instrument tests and generate GCC line coverage reports" OFF)
if(CHESS_ENABLE_COVERAGE)
    include(cmake/Coverage.cmake)
endif()

# Shared rules and AI search used by the application and tests.
add_library(chess_engine
    ChessBackend/Position.cpp
    ChessBackend/Position.h
    ChessBackend/ChessAI.cpp
    ChessBackend/ChessAI.h
)
set_target_properties(chess_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CHESS_ENABLE_COVERAGE)
    target_link_libraries(chess_engine PUBLIC chess_coverage)
endif()

add_library(chess_mobile Android/AndroidGame.cpp Android/AndroidGame.h)
target_link_libraries(chess_mobile PUBLIC chess_engine Qt6::Core Qt6::Concurrent)
if(ANDROID OR CHESS_BUILD_MOBILE_PREVIEW)
    add_subdirectory(Android)
endif()
if(ANDROID)
    # Native CTest executables and desktop runtime deployment do not apply to APKs.
    return()
endif()

# Main executable
qt_add_executable (Chess "ChessApplication/main.cpp" "ChessApplication/ChessApplication.h" "ChessApplication/ChessApplication.cpp" "ChessApplication/ChessApplication.ui" "ChessApplication/Images/images.qrc")
target_link_libraries(Chess PRIVATE chess_engine Qt6::Widgets Qt6::Gui Qt6::Concurrent)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Chess PROPERTY CXX_STANDARD 20)
endif()

set_target_properties(Chess PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

enable_testing()

if(CHESS_BUILD_MOBILE_PREVIEW)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(mobile_ui_test Tests/mobile_ui_test.cpp
        Android/mobile.qrc ChessApplication/Images/images.qrc)
    target_link_libraries(mobile_ui_test PRIVATE chess_mobile Qt6::Quick Qt6::QuickControls2 Qt6::Test)
    add_test(NAME Mobile_UI_Test COMMAND mobile_ui_test)
    set_tests_properties(Mobile_UI_Test PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
        TIMEOUT 20
    )
endif()

add_executable(mobile_game_test Tests/mobile_game_test.cpp)
target_link_libraries(mobile_game_test PRIVATE chess_mobile)
add_test(NAME Mobile_Game_Test COMMAND mobile_game_test)
set_tests_properties(Mobile_Game_Test PROPERTIES TIMEOUT 20)

add_executable(position_test Tests/position_test.cpp)
target_link_libraries(position_test PRIVATE chess_engine)
add_test(NAME Position_Test COMMAND position_test)

add_executable(sliding_piece_test Tests/sliding_piece_test.cpp)
target_link_libraries(sliding_piece_test PRIVATE chess_engine)
add_test(NAME Sliding_Piece_Test COMMAND sliding_piece_test)

add_executable(ai_test Tests/ai_test.cpp)
target_link_libraries(ai_test PRIVATE chess_engine)
add_test(NAME AI_Test COMMAND ai_test)
set_tests_properties(AI_Test PROPERTIES TIMEOUT 20)

add_executable(application_test
    Tests/application_test.cpp
    ChessApplication/ChessApplication.cpp
    ChessApplication/ChessApplication.h
    ChessApplication/ChessApplication.ui
    ChessApplication/Images/images.qrc
)
target_link_libraries(application_test PRIVATE chess_engine Qt6::Widgets Qt6::Gui Qt6::Concurrent)
add_test(NAME Application_Test COMMAND application_test)
set_tests_properties(Application_Test PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    TIMEOUT 45
)

if(CHESS_ENABLE_COVERAGE)
    chess_add_coverage_target()
endif()

# Keep normal developer builds compatible with Qt 6.3. Packaging uses Qt 6.5+.
include(GNUInstallDirs)
install(TARGETS Chess
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

option(CHESS_DEPLOY_RUNTIME "Bundle Qt and compiler runtime dependencies on install" OFF)
if(CHESS_DEPLOY_RUNTIME)
    if(Qt6_VERSION VERSION_LESS 6.5)
        message(FATAL_ERROR "Runtime packaging requires Qt 6.5 or newer")
    endif()
    if(UNIX AND NOT APPLE)
        set_target_properties(Chess PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    endif()
    if(MSVC)
        # App-local DLLs let users run the package without installing build tools.
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
        include(InstallRequiredSystemLibraries)
    endif()
    # Include the platform used by the Windows package startup check.
    qt_import_plugins(Chess INCLUDE Qt6::QOffscreenIntegrationPlugin)
    qt_generate_deploy_app_script(
        TARGET Chess
        OUTPUT_SCRIPT deploy_script
        NO_TRANSLATIONS
    )
    install(SCRIPT ${deploy_script})
endif()
