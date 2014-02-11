set(kcm_file_SRCS
  kcm.cpp
  ../fileexcludefilters.cpp
  folderselectionwidget.cpp
)

kde4_add_ui_files(kcm_file_SRCS
  configwidget.ui
)

kde4_add_plugin(kcm_baloofile ${kcm_file_SRCS})

target_link_libraries(kcm_baloofile
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_SOLID_LIBS}
)


install(FILES kcm_baloofile.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kcm_baloofile DESTINATION ${PLUGIN_INSTALL_DIR})
