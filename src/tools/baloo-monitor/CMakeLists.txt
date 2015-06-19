set(MONITOR_SOURCES
  main.cpp
  monitor.cpp
)

qt5_add_resources(RESOURCES qml.qrc)

add_executable(baloo-monitor ${MONITOR_SOURCES} ${RESOURCES})

target_link_libraries(baloo-monitor PUBLIC
  Qt5::Core
  Qt5::Quick
  Qt5::DBus
  KF5::BalooEngine
)

install(TARGETS baloo-monitor ${INSTALL_TARGETS_DEFAULT_ARGS})
install (FILES org.kde.baloo-monitor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
