add_library(contactsearchstore MODULE contactsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(contactsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5::BalooCore
    KF5::BalooXapian
)

install(FILES baloo_contactsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS contactsearchstore DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/baloo)
