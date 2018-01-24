remove_definitions(-DTRANSLATION_DOMAIN=\"baloo_file5\")
add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file_extractor5\")

set(EXTRACTOR_SRCS
  main.cpp
  app.cpp
  result.cpp
  iohandler.cpp
  idlestatemonitor.cpp
  ../priority.cpp
  ../basicindexingjob.cpp
  ../fileindexerconfig.cpp
  ../storagedevices.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
)

ecm_qt_declare_logging_category(EXTRACTOR_SRCS HEADER baloodebug.h IDENTIFIER BALOO CATEGORY_NAME org.kde.baloo)

add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})
ecm_mark_nongui_executable(baloo_file_extractor)
target_compile_definitions(baloo_file_extractor PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloo_file_extractor
  Qt5::DBus
  Qt5::Widgets
  KF5::FileMetaData
  KF5::I18n
  KF5::ConfigCore
  KF5::Solid
  KF5::BalooEngine
  KF5::Crash
  KF5::IdleTime
)

install(TARGETS baloo_file_extractor DESTINATION ${BIN_INSTALL_DIR})

if(BUILD_TESTING)
  add_subdirectory(autotests)
endif()
