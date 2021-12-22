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
  Qt${QT_MAJOR_VERSION}::Gui
  KF5::FileMetaData
  KF5::ConfigCore
  KF5::Solid
  KF5::BalooEngine
  KF5::Crash
  KF5::IdleTime
)

# KF6 TODO - remove compatibility symlink to bin/baloo_file_extractor
add_custom_command(TARGET baloo_file_extractor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${KDE_INSTALL_FULL_LIBEXECDIR}/baloo_file_extractor baloo_file_extractor)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/baloo_file_extractor DESTINATION ${KDE_INSTALL_FULL_BINDIR})

install(TARGETS baloo_file_extractor DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})

