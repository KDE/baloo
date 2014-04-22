project(Baloo)
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 0.0.12 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Widgets DBus Test Sql)
find_package(KF5 REQUIRED NO_MODULE COMPONENTS I18n Config Service IdleTime KCMUtils Auth Runner Crash Solid KIO Auth)
find_package(KFileMetaData 4.97 REQUIRED)

# FIXME: Port Kdepimlibs?
#find_package(KdepimLibs REQUIRED)

include(ECMSetupVersion)
include(ECMAddTests)
include(ECMInstallIcons)

include(GenerateExportHeader)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

##find_package(KdepimLibs 4.60.43 CONFIG)
##set_package_properties(KdepimLibs PROPERTIES
##    DESCRIPTION "The KDEPIM libraries"
#    URL "http://www.kde.org"
#    TYPE OPTIONAL #Can't be required yet, as it's not ported
#)

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

#find_package(Akonadi 1.19.42 CONFIG)
#set_package_properties(Akonadi PROPERTIES
#    URL "http://akonadi-project.org"
#    TYPE REQUIRED
#)

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-fexceptions)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX BALOO
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloo_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/BalooConfigVersion.cmake")

include_directories(
  ${XAPIAN_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/file/lib
  ${CMAKE_BINARY_DIR}/src/file/lib
  ${CMAKE_SOURCE_DIR}/src/xapian
  ${CMAKE_BINARY_DIR}/src/xapian
)

add_subdirectory(src)
add_subdirectory(icons)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BalooConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/BalooConfig.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BalooConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/BalooConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Baloo)
install(EXPORT BalooLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Baloo
        FILE BalooTargetsWithPrefix.cmake)


feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)
