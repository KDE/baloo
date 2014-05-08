add_definitions(${KDE4_ENABLE_EXCEPTIONS})

include_directories(${KDEPIMLIBS_INCLUDE_DIRS})
kde4_add_plugin(akonadi_baloo_searchplugin
    searchplugin.cpp
)

target_link_libraries(akonadi_baloo_searchplugin
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    ${XAPIAN_LIBRARIES}
    baloocore
    baloopim
    balooxapian
)

install(TARGETS akonadi_baloo_searchplugin DESTINATION ${PLUGIN_INSTALL_DIR}/akonadi)
install(FILES akonadibaloosearchplugin.desktop
        DESTINATION ${PLUGIN_INSTALL_DIR}/akonadi)

add_subdirectory(autotests)
