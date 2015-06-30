set(MONITOR_SOURCES
  main.cpp
  monitor.cpp
)

qt5_add_resources(RESOURCES qml.qrc)

set(baloo_interface_xml ${CMAKE_SOURCE_DIR}/src/file/org.kde.baloo.file.indexer.xml)
qt5_add_dbus_interface(BALOO_INTERFACE ${baloo_interface_xml} baloo_interface)

set(extractor_interface_xml ${CMAKE_SOURCE_DIR}/src/file/extractor/org.kde.balooExtractor.xml)
qt5_add_dbus_interface(EXTRACTOR_INTERFACE ${extractor_interface_xml} extractor_interface)

add_executable(baloo-monitor ${MONITOR_SOURCES} ${RESOURCES} ${BALOO_INTERFACE} ${EXTRACTOR_INTERFACE})

target_link_libraries(baloo-monitor PUBLIC
  Qt5::Core
  Qt5::Quick
  Qt5::DBus
  KF5::BalooEngine
)

install(TARGETS baloo-monitor ${INSTALL_TARGETS_DEFAULT_ARGS})
install (FILES org.kde.baloo-monitor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
