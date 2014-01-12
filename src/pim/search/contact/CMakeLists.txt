add_library(baloo_contactsearchstore MODULE contactsearchstore.cpp)

target_link_libraries(baloo_contactsearchstore
    Qt5::Core
    KF5::KDE4Support
    ${XAPIAN_LIBRARIES}
    baloocore
    baloopim
    balooxapian
)

install(FILES baloo_contactsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_contactsearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
