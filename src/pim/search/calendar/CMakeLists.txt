kde4_add_plugin(baloo_calendarsearchstore calendarsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(baloo_calendarsearchstore
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${XAPIAN_LIBRARIES}
    baloocore
    balooxapian
)

install(FILES baloo_calendarsearchstore.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS baloo_calendarsearchstore DESTINATION ${PLUGIN_INSTALL_DIR})
