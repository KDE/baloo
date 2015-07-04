set(baloo_interface_xml ${CMAKE_SOURCE_DIR}/src/file/org.kde.baloo.file.indexer.xml)
set(extractor_interface_xml ${CMAKE_SOURCE_DIR}/src/file/extractor/org.kde.baloo.extractor.xml)

set_source_files_properties(${baloo_interface_xml} ${extractor_interface_xml}
  PROPERTIES NO_NAMESPACE true
)

qt5_add_dbus_interface(EXTRACTOR_INTERFACE ${extractor_interface_xml} extractor_interface)
qt5_add_dbus_interface(BALOO_INTERFACE ${baloo_interface_xml} baloo_interface)


add_library(BalooDBusInterfaces STATIC
  ${BALOO_INTERFACE}
  ${EXTRACTOR_INTERFACE}
)

target_link_libraries(BalooDBusInterfaces
  Qt5::Core
  Qt5::DBus
)
