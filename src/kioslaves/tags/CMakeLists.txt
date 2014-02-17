kde4_add_plugin(kio_tags kio_tags.cpp)

target_link_libraries(kio_tags
  ${KDE4_KIO_LIBS}
  baloocore
  baloofiles
)

install(TARGETS kio_tags DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES tags.protocol DESTINATION ${SERVICES_INSTALL_DIR})
