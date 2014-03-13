kde4_add_plugin(baloo_notesearchstore notesearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(baloo_notesearchstore
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
    baloocore
    balooxapian
)

install(FILES baloo_notesearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_notesearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
