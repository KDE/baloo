add_library(kio_tags MODULE kio_tags.cpp)

target_link_libraries(kio_tags
  KF5::KIOWidgets
  KF5::KDELibs4Support
  KF5::FileMetaData
  baloocore
  baloofiles
)

install(TARGETS kio_tags DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES tags.protocol DESTINATION ${SERVICES_INSTALL_DIR})
