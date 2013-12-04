kde4_add_plugin(kio_baloosearch kio_search.cpp)

target_link_libraries(kio_baloosearch
  ${KDE4_KIO_LIBS}
  baloocore
)

install(TARGETS kio_baloosearch DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES baloosearch.protocol DESTINATION ${SERVICES_INSTALL_DIR})
