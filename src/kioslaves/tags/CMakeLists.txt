add_definitions(-DTRANSLATION_DOMAIN=\"kio5_tags\")

set(tags_LIB_SRCS
   kio_tags.cpp
)

ecm_qt_declare_logging_category(tags_LIB_SRCS
                                HEADER kio_tags_debug.h
                                IDENTIFIER Baloo::KIO_TAGS
                                DEFAULT_SEVERITY Warning
                                CATEGORY_NAME kf5.kio.kio_tags
)

add_library(tags MODULE ${tags_LIB_SRCS})

target_link_libraries(tags
  KF5::KIOWidgets
  KF5::FileMetaData
  KF5::I18n
  KF5::Baloo
)

install(TARGETS tags DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kio)
install( FILES tags.protocol DESTINATION ${SERVICES_INSTALL_DIR})
