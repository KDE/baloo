add_library(kio_baloosearch MODULE kio_search.cpp)

target_link_libraries(kio_baloosearch
  KF5::KIOWidgets
  KF5::I18n
  baloocore
)

install(TARGETS kio_baloosearch DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES baloosearch.protocol DESTINATION ${SERVICES_INSTALL_DIR})
