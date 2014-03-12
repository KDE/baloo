kauth_install_actions(org.kde.baloo.filewatch org.kde.baloo.filewatch.actions)
add_executable(kde_baloo_filewatch_raiselimit raiselimit.cpp)

target_link_libraries(kde_baloo_filewatch_raiselimit ${KDE4_KDECORE_LIBS})
install(TARGETS kde_baloo_filewatch_raiselimit DESTINATION ${LIBEXEC_INSTALL_DIR})

kauth_install_helper_files(kde_baloo_filewatch_raiselimit org.kde.baloo.filewatch root)
