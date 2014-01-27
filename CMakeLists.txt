project(baloo)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(Xapian REQUIRED)
find_package(QJSON REQUIRED)
find_package(KFileMetaData REQUIRED)
find_package(Xattr REQUIRED)

include(KDE4Defaults)
add_definitions(-DQT_NO_KEYWORDS)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${QJSON_INCLUDE_DIR}
  ${KFILEMETADATA_INCLUDE_DIR}
  ${XAPIAN_INCLUDE_DIR}
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

macro_display_feature_log()
