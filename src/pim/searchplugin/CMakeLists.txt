add_definitions(${KDE4_ENABLE_EXCEPTIONS})

kde4_add_plugin(akonadi_baloo_searchplugin
    searchplugin.cpp
)

target_link_libraries(akonadi_baloo_searchplugin
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${XAPIAN_LIBRARIES}
    baloocore
    baloopim
    balooxapian
)

install(TARGETS akonadi_baloo_searchplugin DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/kde4/plugins")
install(FILES akonadibaloosearchplugin.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(tests)