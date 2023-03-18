add_definitions(-DTRANSLATION_DOMAIN=\"balooctl6\")

add_executable(balooctl)
target_sources(balooctl PRIVATE
    main.cpp
    indexer.cpp
    command.cpp
    configcommand.cpp
    statuscommand.cpp
    monitorcommand.cpp
    ${CMAKE_SOURCE_DIR}/src/file/extractor/result.cpp
)

ecm_mark_nongui_executable(balooctl)
target_compile_definitions(balooctl PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(balooctl
    Qt6::DBus
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::Baloo
    KF6::BalooEngine
    baloofilecommon
    BalooDBusMainInterface
    BalooDBusSchedulerInterface
    BalooDBusFileIndexerInterface
)

install(TARGETS balooctl DESTINATION ${KDE_INSTALL_BINDIR})
