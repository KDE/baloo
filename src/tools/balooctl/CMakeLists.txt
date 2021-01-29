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


# CMake <= 3.9 skipped automoc for generated files
# CMake >= 3.10 requires SKIP_AUTOMOC to actually skip automoc
SET_SOURCE_FILES_PROPERTIES(${DBUS_INTERFACES} PROPERTIES GENERATED 1 SKIP_AUTOMOC 1)

add_executable(balooctl ${SRCS} ${DBUS_INTERFACES})
ecm_mark_nongui_executable(balooctl)
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

install(TARGETS balooctl DESTINATION ${KDE_INSTALL_BINDIR})
