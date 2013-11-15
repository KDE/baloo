project(kioslave-nepomuk-timeline)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KDE4_KIO_INCLUDES}
  ${NEPOMUK_CORE_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${nepomuk_kio_slaves_SOURCE_DIR}/common
)

########### next target ###############

set(kio_timeline_PART_SRCS
  kio_timeline.cpp
  ../common/timelinetools.cpp)

qt4_add_dbus_interface(kio_timeline_PART_SRCS ${NEPOMUK_CORE_DBUS_INTERFACES_DIR}/org.kde.nepomuk.ServiceControl.xml nepomukservicecontrolinterface)

kde4_add_plugin(kio_timeline ${kio_timeline_PART_SRCS})

target_link_libraries(kio_timeline
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  ${SOPRANO_LIBRARIES}
)

install(TARGETS kio_timeline  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES timeline.protocol  DESTINATION  ${SERVICES_INSTALL_DIR})
