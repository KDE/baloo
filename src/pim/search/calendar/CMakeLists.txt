add_library(baloo_calendarsearchstore MODULE calendarsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(baloo_calendarsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5::BalooCore
    KF5::BalooXapian
)

install(FILES baloo_calendarsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_calendarsearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
