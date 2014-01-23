kde4_add_plugin(krunner_baloosearchrunner baloosearchrunner.cpp)

target_link_libraries(krunner_baloosearchrunner
  ${KDE4_KDECORE_LIBS}
  ${KDE4_PLASMA_LIBS}
  ${KDE4_KIO_LIBS}
  baloocore
)

install(
  TARGETS krunner_baloosearchrunner
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
  FILES plasma-runner-baloosearch.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)
