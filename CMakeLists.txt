cmake_minimum_required(VERSION 3.16)
project(Jasmine VERSION 0.1 LANGUAGES CXX)

option(FLATPAK_BUILD "Build for Flatpak" OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(FLATPAK_BUILD)
    message(STATUS "Building for Flatpak")

    # Set up paths
    set(WEBENGINE_BUNDLE_DIR "${CMAKE_SOURCE_DIR}/qwebengine-bundle")
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

    # Copy libraries to expected locations
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/libexec")

    # Copy all libraries
    file(GLOB WEBENGINE_LIBS "${WEBENGINE_BUNDLE_DIR}/lib/*")
    foreach(LIB ${WEBENGINE_LIBS})
        get_filename_component(LIB_NAME ${LIB} NAME)
        configure_file(${LIB} "${CMAKE_CURRENT_BINARY_DIR}/lib/${LIB_NAME}" COPYONLY)
    endforeach()

    # Copy executables
    configure_file("${WEBENGINE_BUNDLE_DIR}/libexec/QtWebEngineProcess"
                   "${CMAKE_CURRENT_BINARY_DIR}/libexec/QtWebEngineProcess" COPYONLY)
    configure_file("${WEBENGINE_BUNDLE_DIR}/libexec/qwebengine_convert_dict"
                   "${CMAKE_CURRENT_BINARY_DIR}/libexec/qwebengine_convert_dict" COPYONLY)
    #configure_file("${WEBENGINE_BUNDLE_DIR}/libexec/webenginedriver"
                   #"${CMAKE_CURRENT_BINARY_DIR}/libexec/webenginedriver" COPYONLY)

    # Set permissions
    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/libexec/QtWebEngineProcess" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/libexec/qwebengine_convert_dict" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    #file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/libexec/webenginedriver" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    link_directories("${CMAKE_CURRENT_BINARY_DIR}/lib")
    link_directories("${WEBENGINE_BUNDLE_DIR}/lib")

    # Find Qt6 (without WebEngine components first)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
    set(QT_VERSION_MAJOR 6)

    add_definitions(-DFLATPAK_BUILD)
else()
    # Normal build
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets WebEngineWidgets Svg)
endif()

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
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Jasmine
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        simple2fa.h simple2fa.cpp
        twofamanager.h twofamanager.cpp
        securitymanager.h securitymanager.cpp
        helpmenudialog.h helpmenudialog.cpp
    )
else()
    if(ANDROID)
        add_library(Jasmine SHARED ${PROJECT_SOURCES})
    else()
        add_executable(Jasmine ${PROJECT_SOURCES})
    endif()
endif()

if(FLATPAK_BUILD)
    add_compile_definitions(FLATHUB_BUILD)
endif()


# Link libraries
if(FLATPAK_BUILD)
    # For Flatpak, add the bundled include directories to the target
    target_include_directories(Jasmine PRIVATE
        "${WEBENGINE_BUNDLE_DIR}/include"
        "${WEBENGINE_BUNDLE_DIR}/include/QtWebEngineCore"
        "${WEBENGINE_BUNDLE_DIR}/include/QtWebEngineWidgets"
        "${WEBENGINE_BUNDLE_DIR}/include/QtWebEngineQuick"
    )

    # Link with Qt6 and add WebEngine libraries manually
    target_link_libraries(Jasmine PRIVATE
        Qt6::Widgets
        Qt6::Svg
    )

    # Add WebEngine libraries directly by filename
    find_library(WEBENGINE_CORE_LIB
        NAMES Qt6WebEngineCore libQt6WebEngineCore.so.6 libQt6WebEngineCore.so
        PATHS "${WEBENGINE_BUNDLE_DIR}/lib"
        NO_DEFAULT_PATH)

    find_library(WEBENGINE_WIDGETS_LIB
        NAMES Qt6WebEngineWidgets libQt6WebEngineWidgets.so.6 libQt6WebEngineWidgets.so
        PATHS "${WEBENGINE_BUNDLE_DIR}/lib"
        NO_DEFAULT_PATH)

    if(WEBENGINE_CORE_LIB AND WEBENGINE_WIDGETS_LIB)
        target_link_libraries(Jasmine PRIVATE ${WEBENGINE_CORE_LIB} ${WEBENGINE_WIDGETS_LIB})
        message(STATUS "Found WebEngine libraries: ${WEBENGINE_CORE_LIB}, ${WEBENGINE_WIDGETS_LIB}")
    else()
        # Fallback to direct path
        target_link_libraries(Jasmine PRIVATE
            "${WEBENGINE_BUNDLE_DIR}/lib/libQt6WebEngineCore.so"
            "${WEBENGINE_BUNDLE_DIR}/lib/libQt6WebEngineWidgets.so"
        )
        message(STATUS "Using direct library paths")
    endif()

    target_link_directories(Jasmine PRIVATE "${WEBENGINE_BUNDLE_DIR}/lib")
else()
    target_link_libraries(Jasmine PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::WebEngineWidgets
        Qt${QT_VERSION_MAJOR}::Svg
    )
endif()

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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Jasmine)
endif()

if(FLATPAK_BUILD)
    # Install bundled files
    install(DIRECTORY "${WEBENGINE_BUNDLE_DIR}/lib/"
            DESTINATION lib
            FILES_MATCHING PATTERN "*")

    install(PROGRAMS "${WEBENGINE_BUNDLE_DIR}/libexec/QtWebEngineProcess"
            DESTINATION libexec)
    #install(PROGRAMS "${WEBENGINE_BUNDLE_DIR}/libexec/webenginedriver"
            #DESTINATION libexec)
    install(PROGRAMS "${WEBENGINE_BUNDLE_DIR}/libexec/qwebengine_convert_dict"
            DESTINATION libexec)

    install(DIRECTORY "${WEBENGINE_BUNDLE_DIR}/resources/"
            DESTINATION share/qtwebengine)

    install(DIRECTORY "${WEBENGINE_BUNDLE_DIR}/include/"
            DESTINATION include)

    install(DIRECTORY "${WEBENGINE_BUNDLE_DIR}/cmake/"
            DESTINATION lib/cmake)
    # Install locale files
    #install(DIRECTORY "${WEBENGINE_BUNDLE_DIR}/share/qtwebengine/locales"
    #        DESTINATION share/qtwebengine)

endif()

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
