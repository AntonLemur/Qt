cmake_minimum_required(VERSION 3.16)

project(courierapp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) # Для сигналов и слотов C++
set(CMAKE_AUTORCC ON) # Для ресурсов (.qrc)
set(CMAKE_AUTOUIC ON) # Для .ui файлов (если есть)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appcourierapp
    main.cpp qml.qrc
)

qt_add_qml_module(appcourierapp
    URI courierapp
    QML_FILES
    Main.qml
    RESOURCES qml.qrc
    SOURCES order.h
    SOURCES ordermodel.h
    QML_FILES OrderListPage.qml
    QML_FILES OrderDetailPage.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appcourierapp PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appcourierapp
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appcourierapp
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appcourierapp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
