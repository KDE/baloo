kde_enable_exceptions()

find_package(KF5KDELibs4Support REQUIRED)
find_package(KF5AkonadiMime REQUIRED)
find_package(KF5Abc REQUIRED)
find_package(KF5PimUtils REQUIRED)
add_library(akonadi_baloo_searchplugin
 MODULE 
    searchplugin.cpp
)

target_link_libraries(akonadi_baloo_searchplugin
    KF5::AkonadiCore
    KF5::AkonadiMime
    ${XAPIAN_LIBRARIES}
    KF5::BalooCore
    KF5::BalooXapian
    baloopim
    KF5::KDELibs4Support
    KF5::ItemModels
    KF5::Abc
    )

install(TARGETS akonadi_baloo_searchplugin DESTINATION ${PLUGIN_INSTALL_DIR}/akonadi)
install(FILES akonadibaloosearchplugin.desktop
        DESTINATION ${PLUGIN_INSTALL_DIR}/akonadi)


#REACTIVATE
#add_subdirectory(autotests)
