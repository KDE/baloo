cmake_minimum_required(VERSION 2.8.12)

project(Baloo)
set(PROJECT_VERSION "5.6.90")

# ECM setup
find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

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
                        SOVERSION 1
)

# Build dependencies
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Widgets Quick DBus Test Sql)

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS I18n Config IdleTime Auth Crash Solid KIO KDELibs4Support DBusAddons)
find_package(KF5 ${PROJECT_VERSION} REQUIRED COMPONENTS FileMetaData)

find_package(Xapian CONFIG)
set_package_properties(Xapian PROPERTIES
    DESCRIPTION "The Xapian search engine library"
    URL "http://xapian.org"
    TYPE REQUIRED
)

# Compiler flags
add_definitions(-DQT_NO_KEYWORDS)

# Turn exceptions on
kde_enable_exceptions()

include_directories(
  ${XAPIAN_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/lib
  ${CMAKE_BINARY_DIR}/src/lib
  ${CMAKE_BINARY_DIR}/src/xapian
  ${CMAKE_SOURCE_DIR}/src/xapian
)

# Targets
add_subdirectory(src)
add_subdirectory(icons)

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
