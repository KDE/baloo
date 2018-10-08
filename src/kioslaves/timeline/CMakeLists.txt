add_definitions(-DTRANSLATION_DOMAIN=\"kio5_timeline\")

set(kio_timeline_SRCS
  kio_timeline.cpp
  timelinetools.cpp
)

ecm_qt_declare_logging_category(kio_timeline_SRCS
                                HEADER kio_timeline_debug.h
                                IDENTIFIER Baloo::KIO_TIMELINE
                                DEFAULT_SEVERITY Warning
                                CATEGORY_NAME kf5.kio.kio_timeline
)

add_library(timeline MODULE ${kio_timeline_SRCS})

target_link_libraries(timeline
  KF5::KIOWidgets
  KF5::I18n
  KF5::Baloo
)

install(TARGETS timeline DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kio)
install(FILES timeline.protocol DESTINATION ${SERVICES_INSTALL_DIR})
