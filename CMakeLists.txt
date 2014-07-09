project(baloo)

set(BALOO_VERSION_MAJOR 4)
set(BALOO_VERSION_MINOR 14)
set(BALOO_VERSION_RELEASE 60)
set(BALOO_VERSION "${BALOO_VERSION_MAJOR}.${BALOO_VERSION_MINOR}.${BALOO_VERSION_RELEASE}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(KdepimLibs 4.13.80 CONFIG)
set_package_properties(KdepimLibs PROPERTIES
    DESCRIPTION "The KDEPIM libraries"
    URL "http://www.kde.org"
    TYPE REQUIRED
)

find_package(Xapian CONFIG)
set_package_properties(Xapian PROPERTIES
    DESCRIPTION "The Xapian search engine library"
    URL "http://xapian.org"
    TYPE REQUIRED
)

find_package(KFileMetaData CONFIG)
set_package_properties(KFileMetaData PROPERTIES
    DESCRIPTION "A library for extracting file metadata"
    URL "https://projects.kde.org/kfilemetadata"
    TYPE REQUIRED
)

find_package(QJSON)
set_package_properties(QJSON PROPERTIES
    DESCRIPTION "Qt library for handling JSON data"
    URL "http://qjson.sourceforge.net/"
    TYPE REQUIRED
)

find_package(Akonadi 1.12.90 CONFIG)
set_package_properties(Akonadi PROPERTIES
    URL "http://akonadi-project.org"
    TYPE REQUIRED
)

include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


include_directories(
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDES}
  ${QJSON_INCLUDE_DIR}
  ${KFILEMETADATA_INCLUDE_DIR}
  ${XAPIAN_INCLUDE_DIR}
  ${KDEPIMLIBS_INCLUDE_DIRS}
  ${AKONADI_INCLUDE_DIR}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/file/lib
  ${CMAKE_SOURCE_DIR}/src/xapian
)

add_subdirectory(src)
add_subdirectory(icons)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BalooConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/BalooConfig.cmake @ONLY)

# this file is used by to check if the installed version can be used.
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/BalooConfigVersion.cmake
                                     ${BALOO_VERSION_MAJOR} ${BALOO_VERSION_MINOR} ${BALOO_VERSION_RELEASE})


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BalooConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/BalooConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Baloo)
install(EXPORT BalooLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Baloo
        FILE BalooTargetsWithPrefix.cmake)

feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)
