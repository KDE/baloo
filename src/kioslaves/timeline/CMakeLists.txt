project(kioslave-timeline)
add_definitions(-DQT_NO_KEYWORDS)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KDE4_KIO_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(kio_timeline_SRCS
  kio_timeline.cpp
  timelinetools.cpp
  ../../file/filemapping.cpp
  ../../file/database.cpp
)

kde4_add_plugin(kio_timeline ${kio_timeline_SRCS})

target_link_libraries(kio_timeline
  ${KDE4_KIO_LIBS}
  ${XAPIAN_LIBRARIES}
  ${QT_QTSQL_LIBRARY}
  baloocore
)

install(TARGETS kio_timeline DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES timeline.protocol DESTINATION ${SERVICES_INSTALL_DIR})
