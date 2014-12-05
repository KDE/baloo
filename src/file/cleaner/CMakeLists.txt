set(CLEANER_SRCS
  main.cpp
  cleaner.cpp
  ../priority.cpp
  ../database.cpp
  ../commitqueue.cpp
  ../fileindexerconfig.cpp
  ../storagedevices.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
)

add_executable(baloo_file_cleaner ${CLEANER_SRCS})

target_link_libraries(baloo_file_cleaner
  Qt5::DBus
  KF5::KIOCore
  ${XAPIAN_LIBRARIES}
  KF5::BalooXapian
  KF5::Solid
)

install(TARGETS baloo_file_cleaner DESTINATION ${BIN_INSTALL_DIR})
