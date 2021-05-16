#
# MainHub
#
set(mainhub_xml org.kde.baloo.main.xml)

qt_generate_dbus_interface(
    ${CMAKE_SOURCE_DIR}/src/file/mainhub.h
    ${mainhub_xml}
    OPTIONS -a
)

#
# FileIndexScheduler
#
set(scheduler_xml org.kde.baloo.scheduler.xml)

qt_generate_dbus_interface(
    ${CMAKE_SOURCE_DIR}/src/file/fileindexscheduler.h
    ${scheduler_xml}
    OPTIONS -a
)

#
# FileContentIndexer
#
set(contentindexer_xml org.kde.baloo.fileindexer.xml)

qt_generate_dbus_interface(
    ${CMAKE_SOURCE_DIR}/src/file/filecontentindexer.h
    ${contentindexer_xml}
    OPTIONS -a
)

set(
    dbus_interface_xmls
    ${CMAKE_CURRENT_BINARY_DIR}/${mainhub_xml}
    ${CMAKE_CURRENT_BINARY_DIR}/${scheduler_xml}
    ${CMAKE_CURRENT_BINARY_DIR}/${contentindexer_xml}
)

qt_add_dbus_interfaces(
    dbus_interfaces
    ${dbus_interface_xmls}
)

add_custom_target(
    BalooDBusInterfaces
    ALL
    DEPENDS ${dbus_interfaces}
)

install(FILES ${dbus_interface_xmls} DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})

add_subdirectory(fake)
