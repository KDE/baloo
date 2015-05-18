cmake_minimum_required(VERSION 2.8.12)

project(Baloo)
set(PROJECT_VERSION "5.9.90")

# ECM setup
find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(ECMInstallIcons)
include(ECMAddTests)

set(KF5_VERSION "5.0.0")

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX BALOO
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloo_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfigVersion.cmake"
                        SOVERSION 2
)

# Build dependencies
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Widgets Quick DBus Test)

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS I18n Config IdleTime Auth Crash Solid KIO KDELibs4Support DBusAddons)
find_package(KF5 ${PROJECT_VERSION} REQUIRED COMPONENTS FileMetaData)

find_package(LMDB)
set_package_properties(LMDB PROPERTIES
    DESCRIPTION "Lightning Memory-Mapped Database (LMDB)"
    URL "http://symas.com/mdb"
    TYPE REQUIRED
)

# Compiler flags
add_definitions(-DQT_NO_KEYWORDS)

set(BUILD_KINOTIFY False)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(BUILD_KINOTIFY True)
endif()

include_directories(
  ${LMDB_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/lib
  ${CMAKE_BINARY_DIR}/src/lib
  ${CMAKE_SOURCE_DIR}/src/codecs
  ${CMAKE_BINARY_DIR}/src/engine
  ${CMAKE_SOURCE_DIR}/src/engine
  ${CMAKE_BINARY_DIR}/src/file
  ${CMAKE_SOURCE_DIR}/src/file
)

# Targets
add_subdirectory(src)
add_subdirectory(icons)

if (${BUILD_KINOTIFY})
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# Config files
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Baloo")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5BalooConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT devel
)

install(EXPORT KF5BalooTargets
        NAMESPACE KF5::
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5BalooTargets.cmake)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/baloo_version.h"
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)
