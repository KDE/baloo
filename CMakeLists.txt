cmake_minimum_required(VERSION 2.8.12)

project(Baloo)

include(FeatureSummary)
find_package(ECM 5.16.0 REQUIRED NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(ECMInstallIcons)
include(ECMAddTests)

set(KF5_VERSION "5.16.0") # handled by release scripts
set(KF5_DEP_VERSION "5.16.0") # handled by release scripts

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX BALOO
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloo_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfigVersion.cmake"
                        SOVERSION 5
)

# Build dependencies
set(REQUIRED_QT_VERSION 5.3.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core DBus Widgets Qml Quick Test)

find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS CoreAddons Config DBusAddons I18n IdleTime Solid FileMetaData Crash KIO)

find_package(LMDB)
set_package_properties(LMDB PROPERTIES
    DESCRIPTION "Lightning Memory-Mapped Database (LMDB)"
    URL "http://symas.com/mdb"
    TYPE REQUIRED
)

# Compiler flags
add_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

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
  ${CMAKE_BINARY_DIR}/src/dbus
)

# Targets
add_subdirectory(src)
add_subdirectory(icons)

if (${BUILD_KINOTIFY})
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
   ki18n_install(po)
endif()

# Config files
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Baloo")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5BalooConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfig.cmake"
    PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
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
