project(baloo)

set(BALOO_VERSION_MAJOR 4)
set(BALOO_VERSION_MINOR 12)
set(BALOO_VERSION_RELEASE 81)
set(BALOO_VERSION "${BALOO_VERSION_MAJOR}.${BALOO_VERSION_MINOR}.${BALOO_VERSION_RELEASE}")


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(KdepimLibs 4.12.80 CONFIG)
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

find_package(Xattr)
set_package_properties(Xattr PROPERTIES
    URL "http://savannah.nongnu.org/projects/attr"
    TYPE REQUIRED
)

find_package(Akonadi 1.11.42 CONFIG)
set_package_properties(Akonadi PROPERTIES
    URL "http://akonadi-project.org"
    TYPE REQUIRED
)


add_definitions(-DQT_NO_KEYWORDS)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${QJSON_INCLUDE_DIR}
  ${KFILEMETADATA_INCLUDE_DIR}
  ${XAPIAN_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
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
