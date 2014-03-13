kde4_add_plugin(baloo_emailsearchstore agepostingsource.cpp emailsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(baloo_emailsearchstore
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
    baloocore
    balooxapian
)

install(FILES baloo_emailsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_emailsearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
