set(kio_timeline_SRCS
  kio_timeline.cpp
  timelinetools.cpp
)

add_library(kio_timeline MODULE ${kio_timeline_SRCS})

target_link_libraries(kio_timeline
  KF5::KIOWidgets
  KF5::I18n
  KF5::KDELibs4Support
  KF5::BalooCore
)

install(TARGETS kio_timeline DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES timeline.protocol DESTINATION ${SERVICES_INSTALL_DIR})
