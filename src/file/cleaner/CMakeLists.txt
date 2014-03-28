set(CLEANER_SRCS
  main.cpp
  cleaner.cpp
  ../priority.cpp
  ../database.cpp
  ../commitqueue.cpp
  ../fileindexerconfig.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
)

kde4_add_executable(baloo_file_cleaner ${CLEANER_SRCS})

target_link_libraries(baloo_file_cleaner
  Qt5::Sql

  KF5::KDE4Support
  KF5::Runner
  ${XAPIAN_LIBRARIES}
  balooxapian
)

install(TARGETS baloo_file_cleaner DESTINATION ${BIN_INSTALL_DIR})
