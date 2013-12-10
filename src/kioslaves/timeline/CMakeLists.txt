project(kioslave-timeline)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KDE4_KIO_INCLUDES}
)

set(kio_timeline_SRCS
  kio_timeline.cpp
  timelinetools.cpp
)

kde4_add_plugin(kio_timeline ${kio_timeline_SRCS})

target_link_libraries(kio_timeline
  ${KDE4_KIO_LIBS}
  baloocore
)

install(TARGETS kio_timeline DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES timeline.protocol DESTINATION ${SERVICES_INSTALL_DIR})
