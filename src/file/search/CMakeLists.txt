add_library(baloo_filesearchstore MODULE
    wildcardpostingsource.cpp
    pathfilterpostingsource.cpp
    filesearchstore.cpp)

target_link_libraries(baloo_filesearchstore
    Qt5::Core
    Qt5::Sql
    ${XAPIAN_LIBRARIES}
    KF5::FileMetaData
    KF5::BalooCore
    KF5::BalooFiles
    KF5::BalooXapian
)

install(FILES baloo_filesearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_filesearchstore DESTINATION ${PLUGIN_INSTALL_DIR})

add_subdirectory(autotest)
