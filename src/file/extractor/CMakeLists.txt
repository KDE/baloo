remove_definitions(-DTRANSLATION_DOMAIN=\"baloo_file5\")
add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file_extractor5\")

set(EXTRACTOR_SRCS
  main.cpp
  app.cpp
  result.cpp
  commandpipe.cpp
  ../priority.cpp
  ../basicindexingjob.cpp
  ../fileindexerconfig.cpp
  ../storagedevices.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
  ../propertydata.cpp
)

ecm_qt_declare_logging_category(EXTRACTOR_SRCS
    HEADER baloodebug.h
    IDENTIFIER BALOO
    CATEGORY_NAME kf.baloo
)
kconfig_add_kcfg_files(EXTRACTOR_SRCS ../baloosettings.kcfgc GENERATE_MOC)

add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})
ecm_mark_nongui_executable(baloo_file_extractor)
target_compile_definitions(baloo_file_extractor PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloo_file_extractor
  Qt5::Gui
  KF5::FileMetaData
  KF5::ConfigCore
  KF5::Solid
  KF5::BalooEngine
  KF5::Crash
  KF5::IdleTime
)

install(TARGETS baloo_file_extractor DESTINATION ${KDE_INSTALL_BINDIR})

