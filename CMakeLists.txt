# set minimum version requirements
cmake_minimum_required(VERSION 3.0)
set(REQUIRED_QT_VERSION 5.7.0)
set(KF5_VERSION "5.39.0") # handled by release scripts
set(KF5_DEP_VERSION "5.39.0") # handled by release scripts

# set up project
project(Baloo VERSION ${KF5_VERSION})

# set up extra-cmake-modules before trying anything else
include(FeatureSummary)

find_package(ECM ${KF5_DEP_VERSION} NO_MODULE)
set_package_properties(ECM
    PROPERTIES
        TYPE REQUIRED
        DESCRIPTION "Extra CMake Modules"
        URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules"
)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

# we found extra-cmake-modules, so use it now
set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR}
)

include(ECMSetupVersion)

# set up baloo versions
ecm_setup_version(PROJECT
    VARIABLE_PREFIX BALOO
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloo_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfigVersion.cmake"
    SOVERSION 5
)

# include optional and extra cmake stuff
include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddTests)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")


# set up build dependencies
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core DBus Widgets Qml Quick Test)
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS CoreAddons Config DBusAddons I18n IdleTime Solid FileMetaData Crash KIO)

find_package(LMDB)
set_package_properties(LMDB
    PROPERTIES
        DESCRIPTION "Lightning Memory-Mapped Database (LMDB)"
        URL "http://symas.com/mdb"
        TYPE REQUIRED
)
if(${LMDB_FOUND})
    include_directories(${LMDB_INCLUDE_DIRS})
endif()

# compiler flags and build system
add_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Inotify)
set_package_properties(Inotify PROPERTIES
    PURPOSE "Filesystem alteration notifications using inotify")
set(BUILD_KINOTIFY ${Inotify_FOUND})

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}

    # we'll configure the rest here so that we don't have to
    # specify them in the source and test directories separately
    ${CMAKE_SOURCE_DIR}/src/lib
    ${CMAKE_BINARY_DIR}/src/lib
    ${CMAKE_BINARY_DIR}/src/codecs
    ${CMAKE_SOURCE_DIR}/src/codecs
    ${CMAKE_BINARY_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_BINARY_DIR}/src/file
    ${CMAKE_SOURCE_DIR}/src/file
    ${CMAKE_BINARY_DIR}/src/dbus
    ${CMAKE_SOURCE_DIR}/src/dbus
)

# targets
add_subdirectory(src)
add_subdirectory(icons)

if (BUILD_TESTING AND BUILD_KINOTIFY)
   add_subdirectory(tests)
   add_subdirectory(autotests)
endif()

# i18n
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
   ki18n_install(po)
endif()

# config files
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Baloo")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Baloo_QCH
        FILE KF5BalooQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5BalooQchTargets.cmake\")")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5BalooConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooConfig.cmake"
    PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

# install targets
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

# and we're done
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
