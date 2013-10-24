project(baloo)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(Xapian REQUIRED)
find_package(QJSON REQUIRED)

include(KDE4Defaults)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${QJSON_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
)

add_subdirectory(src)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/BalooConfig.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Baloo)

macro_display_feature_log()
