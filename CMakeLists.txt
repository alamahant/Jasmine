cmake_minimum_required(VERSION 3.16)
project(Jasmine VERSION 0.1 LANGUAGES CXX)

option(FLATPAK_BUILD "Build for Flatpak" OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components (WebEngine is provided by BaseApp)
find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets Svg)
set(QT_VERSION_MAJOR 6)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    websitelistmodel.h
    websitelistmodel.cpp
    resources.qrc
    downloadmanager.h
    downloadmanager.cpp
    downloaditem.h
    downloaditem.cpp
    downloadwindow.h
    downloadwindow.cpp
    simple2fa.h simple2fa.cpp
    twofamanager.h twofamanager.cpp
    securitymanager.h securitymanager.cpp
    helpmenudialog.h helpmenudialog.cpp
)

qt_add_executable(Jasmine
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

if(FLATPAK_BUILD)
    add_compile_definitions(FLATPAK_BUILD)
endif()

# Link libraries (WebEngine provided by BaseApp)
target_link_libraries(Jasmine PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Svg
)

set_target_properties(Jasmine PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Jasmine
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(Jasmine)

# Install desktop files and icons
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.Jasmine.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.Jasmine.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.Jasmine.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/favicon/16x16/io.github.alamahant.Jasmine.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/16x16/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/favicon/32x32/io.github.alamahant.Jasmine.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/favicon/192x192/io.github.alamahant.Jasmine.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/192x192/apps)

