add_library(notesearchstore MODULE notesearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(notesearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5::BalooCore
    KF5::BalooXapian
)

install(TARGETS notesearchstore DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/baloo)
