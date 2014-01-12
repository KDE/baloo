include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_unit_test(tagtest tagtests.cpp)

target_link_libraries(tagtest
  Qt5::Core
  Qt5::Sql
  Qt5::Test
  
  baloocore
  balootags
)
