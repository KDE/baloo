set(EXTRACTOR_SRCS
  main.cpp
  app.cpp
  result.cpp
  ../priority.cpp
  ../basicindexingjob.cpp
  ../database.cpp
  ../util.cpp
)

add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})

target_link_libraries(baloo_file_extractor
  Qt5::Sql
  Qt5::Widgets
  KF5::FileMetaData
  KF5::I18n
  ${XAPIAN_LIBRARIES}
  baloofiles
  balooxapian
)

install(TARGETS baloo_file_extractor DESTINATION ${BIN_INSTALL_DIR})
