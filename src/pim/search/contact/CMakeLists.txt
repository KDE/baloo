kde4_add_plugin(baloo_contactsearchstore contactsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(baloo_contactsearchstore
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
    baloocore
    baloopim
    balooxapian
)

install(FILES baloo_contactsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_contactsearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
