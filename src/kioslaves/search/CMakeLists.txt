add_definitions(-DTRANSLATION_DOMAIN=\"kio5_baloosearch\")

add_library(kio_baloosearch MODULE kio_search.cpp)

target_link_libraries(kio_baloosearch
  KF5::KIOCore
  KF5::I18n
  KF5::Baloo
)

set_target_properties(kio_baloosearch PROPERTIES OUTPUT_NAME "baloosearch")

install(TARGETS kio_baloosearch DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kio)
install(FILES baloosearch.protocol DESTINATION ${SERVICES_INSTALL_DIR})
