add_definitions(-DTRANSLATION_DOMAIN=\"kio_timeline\")

set(timeline_SRCS
  kio_timeline.cpp
  timelinetools.cpp
)

add_library(timeline MODULE ${kio_timeline_SRCS})

target_link_libraries(timeline
  KF5::KIOWidgets
  KF5::I18n
  KF5::KDELibs4Support
  KF5::BalooCore
)

install(TARGETS timeline DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kio)
install(FILES timeline.protocol DESTINATION ${SERVICES_INSTALL_DIR})
