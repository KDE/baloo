add_definitions(-DTRANSLATION_DOMAIN=\"baloomonitorplugin\")

set(baloomonitorplugin_SRCS
    baloomonitorplugin.cpp
    monitor.cpp
)

add_library(baloomonitorplugin SHARED ${baloomonitorplugin_SRCS})

target_link_libraries(baloomonitorplugin
    PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::DBus
    KF6::I18n
    KF6::CoreAddons
    PRIVATE
    KF6::Baloo
    KF6::BalooEngine
    BalooDBusSchedulerInterface
    BalooDBusFileIndexerInterface
)
install(TARGETS baloomonitorplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)
