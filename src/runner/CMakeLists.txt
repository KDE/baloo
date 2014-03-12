kde4_add_plugin(krunner_baloosearchrunner baloosearchrunner.cpp)

target_link_libraries(krunner_baloosearchrunner
  KF5::Runner
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
