include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(tagrelationwatcher relationwatcher.cpp)

target_link_libraries(tagrelationwatcher
  Qt5::Core
  Qt5::Sql
  Qt5::Test
  
  baloocore
  balootags
)

add_executable(tagwatcher tagwatcher.cpp)

target_link_libraries(tagwatcher
  Qt5::Core
  Qt5::Sql
  Qt5::Test
  
  baloocore
  balootags
)
