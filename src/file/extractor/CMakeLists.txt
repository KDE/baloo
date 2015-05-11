remove_definitions(-DTRANSLATION_DOMAIN=\"baloo_file\")
add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file_extractor\")

set(EXTRACTOR_SRCS
  main.cpp
  app.cpp
  result.cpp
  ../priority.cpp
  ../basicindexingjob.cpp
  ../fileindexerconfig.cpp
  ../storagedevices.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
  ../baloodebug.cpp
)

add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})
target_compile_definitions(baloo_file_extractor PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloo_file_extractor
  Qt5::DBus
  KF5::FileMetaData
  KF5::I18n
  KF5::ConfigCore
  KF5::Solid
  KF5::BalooEngine
  KF5::Crash
)

install(TARGETS baloo_file_extractor DESTINATION ${BIN_INSTALL_DIR})

add_subdirectory(autotests)
