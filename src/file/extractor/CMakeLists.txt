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
  KF5::KDE4Support
  ${KFILEMETADATA_LIBRARY}
  ${XAPIAN_LIBRARIES}
  baloofiles
  balooxapian
)

install(TARGETS baloo_file_extractor DESTINATION ${BIN_INSTALL_DIR})
