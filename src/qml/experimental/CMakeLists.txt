add_definitions(-DTRANSLATION_DOMAIN=\"baloomonitorplugin\")

set(DBUS_INTERFACES
  ${CMAKE_BINARY_DIR}/src/dbus/schedulerinterface.cpp
  ${CMAKE_BINARY_DIR}/src/dbus/fileindexerinterface.cpp
)

SET_SOURCE_FILES_PROPERTIES(${DBUS_INTERFACES} PROPERTIES GENERATED 1)

set(baloomonitorplugin_SRCS
    baloomonitorplugin.cpp
    monitor.cpp
    ${DBUS_INTERFACES}
)

add_library(baloomonitorplugin SHARED ${baloomonitorplugin_SRCS})
add_dependencies(baloomonitorplugin BalooDBusInterfaces)

target_link_libraries(baloomonitorplugin
    PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::DBus
    KF6::I18n
    KF6::CoreAddons
    PRIVATE
    KF6::Baloo
    KF6::BalooEngine
)
install(TARGETS baloomonitorplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)
