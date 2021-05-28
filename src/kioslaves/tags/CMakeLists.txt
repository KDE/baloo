add_definitions(-DTRANSLATION_DOMAIN=\"kio5_tags\")

set(tags_LIB_SRCS
   kio_tags.cpp
)

ecm_qt_declare_logging_category(tags_LIB_SRCS
    HEADER kio_tags_debug.h
    IDENTIFIER Baloo::KIO_TAGS
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME kf.kio.slaves.tags
    OLD_CATEGORY_NAMES kf5.kio.kio_tags
    DESCRIPTION "Baloo Tags (KIO)"
    EXPORT BALOO
)

add_library(tags MODULE ${tags_LIB_SRCS})

target_link_libraries(tags
  KF5::KIOCore
  KF5::FileMetaData
  KF5::I18n
  KF5::Baloo
)

install(TARGETS tags DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio)
