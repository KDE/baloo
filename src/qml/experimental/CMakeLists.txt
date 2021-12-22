add_definitions(-DTRANSLATION_DOMAIN=\"baloomonitorplugin\")

set(DBUS_INTERFACES
  ${CMAKE_BINARY_DIR}/src/dbus/schedulerinterface.cpp
  ${CMAKE_BINARY_DIR}/src/dbus/fileindexerinterface.cpp
)

# CMake <= 3.9 skipped automoc for generated files
# CMake >= 3.10 requires SKIP_AUTOMOC to actually skip automoc
SET_SOURCE_FILES_PROPERTIES(${DBUS_INTERFACES} PROPERTIES GENERATED 1 SKIP_AUTOMOC 1)

set(baloomonitorplugin_SRCS
    baloomonitorplugin.cpp
    monitor.cpp
    ${DBUS_INTERFACES}
)

add_library(baloomonitorplugin SHARED ${baloomonitorplugin_SRCS})
add_dependencies(baloomonitorplugin BalooDBusInterfaces)

target_link_libraries(baloomonitorplugin
    PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::I18n
    KF5::CoreAddons
    PRIVATE
    KF5::Baloo
    KF5::BalooEngine
)
install(TARGETS baloomonitorplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)
