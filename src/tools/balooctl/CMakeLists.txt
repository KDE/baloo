add_definitions(-DTRANSLATION_DOMAIN=\"balooctl\")

set(SRCS
    main.cpp
    indexer.cpp
    command.cpp
    configcommand.cpp
    statuscommand.cpp
    monitorcommand.cpp
    ${CMAKE_SOURCE_DIR}/src/file/extractor/result.cpp
)

set(DBUS_INTERFACES
  ${CMAKE_BINARY_DIR}/src/dbus/maininterface.cpp
  ${CMAKE_BINARY_DIR}/src/dbus/schedulerinterface.cpp
  ${CMAKE_BINARY_DIR}/src/dbus/fileindexerinterface.cpp
)

set_source_files_properties(${DBUS_INTERFACES} PROPERTIES GENERATED 1)

add_executable(balooctl ${SRCS} ${DBUS_INTERFACES})
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
