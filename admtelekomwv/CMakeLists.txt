cmake_minimum_required(VERSION 3.16)
# include(FetchContent)
project(admtelekomwv VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_FIND_DEBUG_MODE ON)

find_package(Qt6 REQUIRED COMPONENTS Quick WebView Multimedia WebSockets GuiPrivate)

set(VLC_PATH "/home/anton/Projects/Qt/admtelekomwv/VLC")

message("Путь к vlc: ${VLC_PATH}")

message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")


qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appadmtelekomwv
    main.cpp qml.qrc
)

qt_add_qml_module(appadmtelekomwv
    URI admtelekomwv
    QML_FILES
        Main.qml
        RESOURCES
        RESOURCES qml.qrc
        SOURCES
        SOURCES httpreader.h
        SOURCES httpreader.cpp
        SOURCES vlcplayer.h
        SOURCES vlcplayer.cpp
        SOURCES frameworker.h
        SOURCES thumbnailprovider.h thumbnailprovider.cpp

)

qt_add_ios_ffmpeg_libraries(appadmtelekomwv)

add_subdirectory(Config)
add_subdirectory(MediaControls)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appadmtelekomwv PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appadmtelekomwv
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set_target_properties(appadmtelekomwv PROPERTIES
 QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
 QT_ANDROID_PACKAGE_NAME "io.qt.admtelekom"
 QT_ANDROID_APP_NAME "Admtelekom"
 QT_ANDROID_TARGET_SDK_VERSION 35
 QT_ANDROID_MIN_SDK_VERSION 28
 QT_ANDROID_VERSION_NAME "1.0"
 QT_ANDROID_VERSION_CODE 10
)

set_target_properties(appadmtelekomwv PROPERTIES
 QT_ANDROID_APP_ICON "@mipmap/ic_launcher_foreground"
)

target_include_directories(appadmtelekomwv PRIVATE "VLC/include")
target_link_directories(appadmtelekomwv PRIVATE "VLC/lib/arm64-v8a")

target_link_libraries(appadmtelekomwv PRIVATE Qt6::Quick Qt6::WebView Qt6::Multimedia Qt6::WebSockets Qt6::GuiPrivate
    vlc
    vlcjni
    mla
    MediaControlsplugin Configplugin)

include(GNUInstallDirs)
install(TARGETS appadmtelekomwv
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if (ANDROID)
    include(/home/anton/Qt/android_openssl-master/CMakeLists.txt)
endif()
