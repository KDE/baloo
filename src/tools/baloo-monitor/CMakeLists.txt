set(MONITOR_SOURCES
  main.cpp
  monitor.cpp
)

set(DBUS_INTERFACES
  ${CMAKE_BINARY_DIR}/src/dbus/schedulerinterface.cpp
  ${CMAKE_BINARY_DIR}/src/dbus/fileindexerinterface.cpp
)

SET_SOURCE_FILES_PROPERTIES(${DBUS_INTERFACES} PROPERTIES GENERATED 1)

qt5_add_resources(RESOURCES qml.qrc)

add_executable(baloo-monitor ${MONITOR_SOURCES} ${RESOURCES} ${DBUS_INTERFACES})
add_dependencies(baloo-monitor BalooDBusInterfaces)

target_link_libraries(baloo-monitor PUBLIC
  Qt5::Core
  Qt5::Quick
  Qt5::DBus
  KF5::BalooEngine
)

install(TARGETS baloo-monitor ${INSTALL_TARGETS_DEFAULT_ARGS})
