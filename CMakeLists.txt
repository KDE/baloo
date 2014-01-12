project(baloo)
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Widgets Xml Test Sql)
find_package(KF5 REQUIRED NO_MODULE COMPONENTS KDE4Support I18n Service IdleTime KCMUtils Auth)
find_package(KFileMetaData REQUIRED)

# FIXME: Port Kdepimlibs?
#find_package(KdepimLibs REQUIRED)

find_package(Xapian REQUIRED)
find_package(QJSON REQUIRED)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
add_definitions(-DQT_NO_KEYWORDS)

include_directories(
  ${QJSON_INCLUDE_DIR}
  ${KFILEMETADATA_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/file/lib
  ${CMAKE_SOURCE_DIR}/src/xapian
)

add_subdirectory(src)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BalooConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/BalooConfig.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BalooConfig.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Baloo)
install(EXPORT BalooLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Baloo
        FILE BalooTargetsWithPrefix.cmake)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
