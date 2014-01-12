include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(querytest querytest.cpp)

target_link_libraries(querytest
  Qt5::Core
  KF5::KDE4Support
  baloocore
)
