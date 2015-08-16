set(baloo_interface_xml ${CMAKE_SOURCE_DIR}/src/file/org.kde.baloo.file.indexer.xml)

set_source_files_properties(${baloo_interface_xml}
  PROPERTIES
    NO_NAMESPACE true
)

qt5_add_dbus_interface(BALOO_INTERFACE ${baloo_interface_xml} baloo_interface)

add_custom_target(BalooDBusInterfaces
  DEPENDS ${BALOO_INTERFACE}
)
