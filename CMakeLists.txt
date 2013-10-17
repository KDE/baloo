project(vizier)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(Xapian REQUIRED)

include(KDE4Defaults)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
)

add_subdirectory(src)
