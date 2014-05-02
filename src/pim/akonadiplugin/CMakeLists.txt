add_definitions(${KDE4_ENABLE_EXCEPTIONS})

find_package(KF5KDELibs4Support REQUIRED)
find_package(KF5AkonadiMime REQUIRED)

add_library(akonadi_baloo_searchplugin
 MODULE 
    searchplugin.cpp
)

target_link_libraries(akonadi_baloo_searchplugin
    KF5::AkonadiCore
    KF5::AkonadiMime
    ${XAPIAN_LIBRARIES}
    baloocore
    baloopim
    balooxapian
    KF5::KDELibs4Support
    KF5::ItemModels
)

install(TARGETS akonadi_baloo_searchplugin DESTINATION ${PLUGIN_INSTALL_DIR}/akonadi)
install(FILES akonadibaloosearchplugin.desktop
        DESTINATION ${PLUGIN_INSTALL_DIR}/akonadi)

add_subdirectory(autotests)
