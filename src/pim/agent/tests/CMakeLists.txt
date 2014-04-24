include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(emailindexer emailtest.cpp ../emailindexer.cpp ../abstractindexer.cpp)

target_link_libraries(emailindexer
  Qt5::Core
  Qt5::Test
  KF5::AkonadiCore
  KF5::AkonadiKMime
  KF5::KMime
  KF5::PimUtils
  KF5::ItemModels
  KF5::KDELibs4Support
  ${XAPIAN_LIBRARIES}
)
