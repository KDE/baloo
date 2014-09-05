set (KDED_BALOOSEARCH_SRCS
     baloosearchmodule.cpp
)

add_library(kded_baloosearch_kio MODULE ${KDED_BALOOSEARCH_SRCS})

target_link_libraries(kded_baloosearch_kio
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::KIOCore
)

install(TARGETS kded_baloosearch_kio DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES baloosearchfolderupdater.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
