add_definitions(-DTRANSLATION_DOMAIN=\"kio5_tags\")

add_library(tags MODULE kio_tags.cpp)

target_link_libraries(tags
  KF5::KIOWidgets
  KF5::FileMetaData
  KF5::I18n
  KF5::Baloo
)

install(TARGETS tags DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kio)
install( FILES tags.protocol DESTINATION ${SERVICES_INSTALL_DIR})
