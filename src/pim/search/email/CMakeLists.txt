add_library(baloo_emailsearchstore MODULE agepostingsource.cpp emailsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(baloo_emailsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    baloocore
    balooxapian
)

install(FILES baloo_emailsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_emailsearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
