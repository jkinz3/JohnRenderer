#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "assimp::zlibstatic" for configuration "Release"
set_property(TARGET assimp::zlibstatic APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(assimp::zlibstatic PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/zlibstatic.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS assimp::zlibstatic )
list(APPEND _IMPORT_CHECK_FILES_FOR_assimp::zlibstatic "${_IMPORT_PREFIX}/lib/zlibstatic.lib" )

# Import target "assimp::assimp" for configuration "Release"
set_property(TARGET assimp::assimp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(assimp::assimp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX;RC"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/assimp.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS assimp::assimp )
list(APPEND _IMPORT_CHECK_FILES_FOR_assimp::assimp "${_IMPORT_PREFIX}/lib/assimp.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
