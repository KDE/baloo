# get from the full path to OkularConfig.cmake up to the base dir dir:
get_filename_component( _currentDir  ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component( _currentDir  ${_currentDir} PATH)
get_filename_component( _currentDir  ${_currentDir} PATH)
get_filename_component( _currentDir  ${_currentDir} PATH)


# find the full paths to the library and the includes:
find_path(BALOO_INCLUDE_DIR baloo/core_export.h
          HINTS ${_currentDir}/include
          NO_DEFAULT_PATH)

find_library(BALOO_CORE_LIBRARY baloocore 
             HINTS ${_currentDir}/lib
             NO_DEFAULT_PATH)

find_library(BALOO_SEARCH_LIBRARY baloosearch
             HINTS ${_currentDir}/lib
             NO_DEFAULT_PATH)

find_library(BALOO_TAG_LIBRARY balootags
             HINTS ${_currentDir}/lib
             NO_DEFAULT_PATH)

set(BALOO_LIBRARIES ${BALOO_CORE_LIBRARY} ${BALOO_SEARCH_LIBRARY} ${BALOO_TAG_LIBRARY})

set(BALOO_BEAR "Bhalu!!")
if(BALOO_INCLUDE_DIR AND BALOO_CORE_LIBRARY)
    set(BALOO_FOUND TRUE)
endif()

