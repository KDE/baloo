add_definitions(-DTRANSLATION_DOMAIN=\"baloomonitorplugin\")

set(baloomonitorplugin_SRCS
    baloomonitorplugin.cpp
    monitor.cpp
)

add_library(baloomonitorplugin SHARED ${baloomonitorplugin_SRCS})

target_link_libraries(baloomonitorplugin
    PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::I18n
    KF5::CoreAddons
    PRIVATE
    KF5::Baloo
    KF5::BalooEngine
    BalooDBusSchedulerInterface
    BalooDBusFileIndexerInterface
)
install(TARGETS baloomonitorplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/baloo/experimental)
