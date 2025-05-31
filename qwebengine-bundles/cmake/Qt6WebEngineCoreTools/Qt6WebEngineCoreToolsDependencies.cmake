# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Find "ModuleTools" dependencies, which are other ModuleTools packages.
set(Qt6WebEngineCoreTools_FOUND FALSE)
set(__qt_WebEngineCoreTools_tool_deps "")
foreach(__qt_WebEngineCoreTools_target_dep ${__qt_WebEngineCoreTools_tool_deps})
    list(GET __qt_WebEngineCoreTools_target_dep 0 __qt_WebEngineCoreTools_pkg)
    list(GET __qt_WebEngineCoreTools_target_dep 1 __qt_WebEngineCoreTools_version)

    if (NOT ${__qt_WebEngineCoreTools_pkg}_FOUND)
        find_dependency(${__qt_WebEngineCoreTools_pkg} ${__qt_WebEngineCoreTools_version})
    endif()
endforeach()

set(Qt6WebEngineCoreTools_FOUND TRUE)
