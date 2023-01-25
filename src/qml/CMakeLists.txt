set(balooplugin_SRCS
    balooplugin.cpp
    queryresultsmodel.cpp
    )

add_library(balooplugin SHARED ${balooplugin_SRCS})
target_link_libraries(balooplugin Qt6::Core Qt6::Qml KF6::Baloo)
install(TARGETS balooplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo)

add_subdirectory(experimental)
