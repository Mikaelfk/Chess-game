# CMakeList.txt : CMake project for ChessCMake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Chess")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
qt_standard_project_setup()

# Add source to this project's executable.
add_executable (Chess "ChessBackend/Board.cpp" "ChessBackend/Board.h"  "ChessBackend/Pieces/ChessBishop.cpp" "ChessBackend/Pieces/ChessBishop.cpp" "ChessBackend/Pieces/ChessBishop.h" "ChessBackend/Pieces/ChessKing.cpp" "ChessBackend/Pieces/ChessKing.h" "ChessBackend/Pieces/ChessKnight.h" "ChessBackend/Pieces/ChessKnight.h" "ChessBackend/Pieces/ChessPawn.cpp" "ChessBackend/Pieces/ChessPawn.h" "ChessBackend/Pieces/ChessPiece.cpp" "ChessBackend/Pieces/ChessPiece.h" "ChessBackend/Pieces/ChessQueen.cpp" "ChessBackend/Pieces/ChessQueen.h" "ChessBackend/Pieces/ChessRook.cpp" "ChessBackend/Pieces/ChessRook.h" "ChessBackend/Pieces/ChessKnight.cpp" "ChessApplication/main.cpp" "ChessApplication/ChessApplication.ui" "ChessApplication/ChessApplication.h" "ChessApplication/ChessApplication.cpp" "ChessApplication/Images/images.qrc")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Chess PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(Chess PRIVATE Qt6::Widgets Qt6::Gui)

set_target_properties(Chess PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

set_target_properties(Chess PROPERTIES
LINK_FLAGS /SUBSYSTEM:CONSOLE
)

