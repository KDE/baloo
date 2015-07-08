add_definitions(-DTRANSLATION_DOMAIN=\"balooctl\")

set(SRCS
    main.cpp
    indexer.cpp
    ${CMAKE_SOURCE_DIR}/src/file/extractor/result.cpp
    monitor.cpp
    ${CMAKE_BINARY_DIR}/src/dbus/extractor_interface.cpp
)

set_source_files_properties(${CMAKE_BINARY_DIR}/src/dbus/extractor_interface.cpp PROPERTIES GENERATED 1)

add_executable(balooctl ${SRCS})
add_dependencies(balooctl BalooDBusInterfaces)
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
