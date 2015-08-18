set(header_file ${CMAKE_SOURCE_DIR}/src/file/mainhub.h)
set(baloo_interface_xml org.kde.baloo.file.indexer.xml)

set_source_files_properties(${baloo_interface_xml}
  PROPERTIES
    NO_NAMESPACE true
)

qt5_generate_dbus_interface(
    ${header_file}
    ${baloo_interface_xml}
    OPTIONS -a
)

qt5_add_dbus_interface(BALOO_INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/${baloo_interface_xml}
    baloo_interface
)

add_custom_target(BalooDBusInterfaces
  DEPENDS ${BALOO_INTERFACE}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${baloo_interface_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
