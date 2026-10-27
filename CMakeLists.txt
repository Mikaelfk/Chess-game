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

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
qt_standard_project_setup()

# All the backend logic in one library
add_library(backend
    ChessBackend/Board.cpp
    ChessBackend/Board.h
    ChessBackend/Pieces/ChessBishop.cpp
    ChessBackend/Pieces/ChessBishop.h
    ChessBackend/Pieces/ChessKing.cpp
    ChessBackend/Pieces/ChessKing.h
    ChessBackend/Pieces/ChessKnight.cpp
    ChessBackend/Pieces/ChessKnight.h
    ChessBackend/Pieces/ChessPawn.cpp
    ChessBackend/Pieces/ChessPawn.h
    ChessBackend/Pieces/ChessPiece.cpp
    ChessBackend/Pieces/ChessPiece.h
    ChessBackend/Pieces/ChessQueen.cpp
    ChessBackend/Pieces/ChessQueen.h
    ChessBackend/Pieces/ChessRook.cpp
    ChessBackend/Pieces/ChessRook.h
)

# Main executable
qt_add_executable (Chess "ChessApplication/main.cpp" "ChessApplication/ChessApplication.h" "ChessApplication/ChessApplication.cpp" "ChessApplication/ChessApplication.ui" "ChessApplication/Images/images.qrc")
target_link_libraries(Chess PRIVATE backend Qt6::Widgets Qt6::Gui)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Chess PROPERTY CXX_STANDARD 20)
endif()

set_target_properties(Chess PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# creating test binaries
add_executable(board_test "Tests/board_test.cpp")
target_link_libraries(board_test PRIVATE backend)

add_executable(move_test "Tests/move_test.cpp")
target_link_libraries(move_test PRIVATE backend)

add_executable(check_test "Tests/check_test.cpp")
target_link_libraries(check_test PRIVATE backend)

add_executable(checkmate_test "Tests/checkmate_test.cpp")
target_link_libraries(checkmate_test PRIVATE backend)

add_executable(stalemate_test "Tests/stalemate_test.cpp")
target_link_libraries(stalemate_test PRIVATE backend)

# defining the tests
enable_testing()
add_test(NAME Board_Test
        COMMAND $<TARGET_FILE:board_test>
)
add_test(NAME Move_Test
        COMMAND $<TARGET_FILE:move_test>
)
add_test(NAME Check_Test
        COMMAND $<TARGET_FILE:check_test>
)
add_test(NAME Checkmate_Test
        COMMAND $<TARGET_FILE:checkmate_test>
)
add_test(NAME Stalemate_Test
        COMMAND $<TARGET_FILE:stalemate_test>
)



add_executable(sliding_piece_test Tests/sliding_piece_test.cpp)
target_link_libraries(sliding_piece_test PRIVATE backend)
add_test(NAME Sliding_Piece_Test COMMAND sliding_piece_test)

add_executable(application_test
    Tests/application_test.cpp
    ChessApplication/ChessApplication.cpp
    ChessApplication/ChessApplication.h
    ChessApplication/ChessApplication.ui
    ChessApplication/Images/images.qrc
)
target_link_libraries(application_test PRIVATE backend Qt6::Widgets Qt6::Gui)
add_test(NAME Application_Test COMMAND application_test)
set_tests_properties(Application_Test PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    TIMEOUT 30
)

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
