#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "zfp::zfp" for configuration "Debug"
set_property(TARGET zfp::zfp APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(zfp::zfp PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/zfp.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/zfp.dll"
  )

list(APPEND _cmake_import_check_targets zfp::zfp )
list(APPEND _cmake_import_check_files_for_zfp::zfp "${_IMPORT_PREFIX}/lib/zfp.lib" "${_IMPORT_PREFIX}/bin/zfp.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
