qt_add_executable(ChessMobile MANUAL_FINALIZATION
    main.cpp mobile.qrc ../ChessApplication/Images/images.qrc
)
target_link_libraries(ChessMobile PRIVATE chess_mobile Qt6::Quick Qt6::QuickControls2)
if(ANDROID)
    set_target_properties(ChessMobile PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/package"
        QT_ANDROID_PACKAGE_NAME "com.mikaelfk.chess"
        QT_ANDROID_VERSION_NAME "1.0.0"
        QT_ANDROID_VERSION_CODE 1
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 35
    )
endif()
qt_finalize_executable(ChessMobile)
