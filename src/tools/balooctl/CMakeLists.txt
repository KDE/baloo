add_definitions(-DTRANSLATION_DOMAIN=\"balooctl\")

set(SRCS
    main.cpp
    indexer.cpp
    ${CMAKE_SOURCE_DIR}/src/file/extractor/result.cpp
    monitor.cpp
)

add_executable(balooctl ${SRCS})
target_compile_definitions(balooctl PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(balooctl
    Qt5::DBus
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::Baloo
    KF5::BalooEngine
    baloofilecommon
)

install(TARGETS balooctl DESTINATION ${BIN_INSTALL_DIR})
