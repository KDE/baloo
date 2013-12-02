kde4_add_plugin(baloo_filesearchstore filesearchstore.cpp)

target_link_libraries(baloo_filesearchstore
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
    baloocore
    baloofiles
)

install(FILES baloo_filesearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_filesearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
