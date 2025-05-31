#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Qt6::qwebengine_convert_dict" for configuration "RelWithDebInfo"
set_property(TARGET Qt6::qwebengine_convert_dict APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Qt6::qwebengine_convert_dict PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/libexec/qwebengine_convert_dict"
  )

list(APPEND _cmake_import_check_targets Qt6::qwebengine_convert_dict )
list(APPEND _cmake_import_check_files_for_Qt6::qwebengine_convert_dict "${_IMPORT_PREFIX}/libexec/qwebengine_convert_dict" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
