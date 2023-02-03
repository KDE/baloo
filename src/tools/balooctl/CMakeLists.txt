add_definitions(-DTRANSLATION_DOMAIN=\"balooctl5\")

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
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::Baloo
    KF5::BalooEngine
    baloofilecommon
    BalooDBusMainInterface
    BalooDBusSchedulerInterface
    BalooDBusFileIndexerInterface
)

install(TARGETS balooctl DESTINATION ${KDE_INSTALL_BINDIR})
