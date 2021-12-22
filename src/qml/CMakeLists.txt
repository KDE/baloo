set(balooplugin_SRCS
    balooplugin.cpp
    queryresultsmodel.cpp
    )

add_library(balooplugin SHARED ${balooplugin_SRCS})
target_link_libraries(balooplugin Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Qml KF5::Baloo)
install(TARGETS balooplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo)

add_subdirectory(experimental)
