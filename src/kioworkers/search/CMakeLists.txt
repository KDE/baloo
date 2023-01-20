add_definitions(-DTRANSLATION_DOMAIN=\"kio5_baloosearch\")

add_library(kio_baloosearch MODULE kio_search.cpp)

target_link_libraries(kio_baloosearch
  KF6::KIOCore
  KF6::I18n
  KF6::Baloo
  Qt${QT_MAJOR_VERSION}::Network
)

set_target_properties(kio_baloosearch PROPERTIES OUTPUT_NAME "baloosearch")

install(TARGETS kio_baloosearch DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/kio)
