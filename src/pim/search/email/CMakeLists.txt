add_library(emailsearchstore MODULE agepostingsource.cpp emailsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(emailsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5::BalooCore
    KF5::BalooXapian
)

install(FILES baloo_emailsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS emailsearchstore DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/baloo)
