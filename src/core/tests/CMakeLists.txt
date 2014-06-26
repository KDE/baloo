add_executable(querytest querytest.cpp)

target_link_libraries(querytest
  Qt5::Core
  KF5BalooCore
)
