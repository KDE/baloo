add_library(baloo_filesearchstore MODULE pathfilterpostingsource.cpp filesearchstore.cpp)

target_link_libraries(baloo_filesearchstore
    Qt5::Core
    Qt5::Sql
    KF5::KDE4Support
    ${XAPIAN_LIBRARIES}
    ${KFILEMETADATA_LIBRARY}
    baloocore
    baloofiles
    balooxapian
)

install(FILES baloo_filesearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_filesearchstore DESTINATION ${PLUGIN_INSTALL_DIR})

add_subdirectory(autotest)
