set(balooplugin_SRCS
    balooplugin.cpp
    queryresultsmodel.cpp
    )

add_library(balooplugin SHARED ${balooplugin_SRCS})
target_link_libraries(balooplugin Qt5::Core Qt5::Qml KF5::Baloo)
install(TARGETS balooplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/baloo/)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/baloo)

add_subdirectory(experimental)
