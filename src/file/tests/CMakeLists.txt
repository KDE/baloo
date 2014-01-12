include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(indexerconfigtest
               indexerconfigtest.cpp
               ../fileindexerconfig.cpp
               ../regexpcache.cpp
               ../fileexcludefilters.cpp)

target_link_libraries(indexerconfigtest
  Qt5::Core
  KF5::KDE4Support
  baloocore
)
