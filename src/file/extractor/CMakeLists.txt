set(EXTRACTOR_SRCS
  main.cpp
  app.cpp
  result.cpp
  ../priority.cpp
  ../basicindexingjob.cpp
  ../database.cpp
  ../util.cpp
)

kde4_add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})

target_link_libraries(baloo_file_extractor
  ${QT_QTSQL_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KFILEMETADATA_LIBRARY}
  ${XAPIAN_LIBRARIES}
  ${QJSON_LIBRARIES}
  baloofiles
  balooxapian
)

install(TARGETS baloo_file_extractor DESTINATION ${BIN_INSTALL_DIR})
