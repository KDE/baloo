set(kcm_file_SRCS
  kcm.cpp
  fileexcludefilters.cpp
  folderselectionmodel.cpp
  indexfolderselectiondialog.cpp
  excludefilterselectiondialog.cpp
  detailswidget.cpp
)

qt5_add_dbus_interface(kcm_file_SRCS ../org.kde.baloo.file.indexer.xml fileindexerinterface)

qt5_wrap_ui(kcm_file_SRCS
  configwidget.ui
  indexfolderselectionwidget.ui
  excludefilterselectiondialog.ui)

add_library(kcm_baloofile MODULE ${kcm_file_SRCS})

target_link_libraries(kcm_baloofile
  KF5::KIOWidgets
  KF5::KDE4Support
  KF5::KCMUtils
)


install(FILES kcm_baloofile.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kcm_baloofile DESTINATION ${PLUGIN_INSTALL_DIR})
