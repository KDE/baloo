add_library(filesearchstore MODULE
    wildcardpostingsource.cpp
    pathfilterpostingsource.cpp
    filesearchstore.cpp)

target_link_libraries(filesearchstore
    Qt5::Core
    Qt5::Sql
    ${XAPIAN_LIBRARIES}
    KF5::FileMetaData
    KF5::BalooCore
    KF5::BalooFiles
    KF5::BalooXapian
)

install(TARGETS filesearchstore DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/baloo)

add_subdirectory(autotest)
