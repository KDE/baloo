add_definitions(-DTRANSLATION_DOMAIN=\"balooctl5\")

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

SET_SOURCE_FILES_PROPERTIES(${DBUS_INTERFACES} PROPERTIES GENERATED 1)

add_executable(balooctl ${SRCS} ${DBUS_INTERFACES})
ecm_mark_nongui_executable(balooctl)
add_dependencies(balooctl BalooDBusInterfaces)
target_compile_definitions(balooctl PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(balooctl
    Qt6::DBus
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::Baloo
    KF6::BalooEngine
    baloofilecommon
)

install(TARGETS balooctl DESTINATION ${KDE_INSTALL_BINDIR})
