remove_definitions(-DTRANSLATION_DOMAIN=\"baloo_file\")
add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file_extractor\")

set(EXTRACTOR_SRCS
  main.cpp
  app.cpp
  result.cpp
  ../priority.cpp
  ../basicindexingjob.cpp
  ../database.cpp
  ../util.cpp
  ../fileindexerconfig.cpp
  ../storagedevices.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
)

add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})

target_link_libraries(baloo_file_extractor
  Qt5::Sql
  Qt5::DBus
  KF5::FileMetaData
  KF5::I18n
  KF5::ConfigCore
  KF5::Solid
  ${XAPIAN_LIBRARIES}
  KF5::BalooFiles
  KF5::BalooXapian
)

install(TARGETS baloo_file_extractor DESTINATION ${BIN_INSTALL_DIR})

add_subdirectory(autotests)
