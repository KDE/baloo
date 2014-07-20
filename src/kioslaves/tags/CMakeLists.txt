add_definitions(-DTRANSLATION_DOMAIN=\"kio_tags\")

add_library(kio_tags MODULE kio_tags.cpp)

target_link_libraries(kio_tags
  KF5::KIOWidgets
  KF5::FileMetaData
  KF5::I18n
  KF5::BalooCore
  KF5::BalooFiles
)

install(TARGETS kio_tags DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES tags.protocol DESTINATION ${SERVICES_INSTALL_DIR})
