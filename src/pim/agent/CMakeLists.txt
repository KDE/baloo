add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_baloo_indexer\")

kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)


set(akonadi_baloo_indexer_SRCS
    abstractindexer.cpp
    agent.cpp
    emailindexer.cpp
    contactindexer.cpp
    akonotesindexer.cpp
    calendarindexer.cpp
    ../../file/priority.cpp
    scheduler.cpp
    collectionindexingjob.cpp
    index.cpp
)

qt5_add_dbus_adaptor(akonadi_baloo_indexer_SRCS org.freedesktop.Akonadi.BalooIndexer.xml agent.h BalooIndexingAgent)

add_executable(akonadi_baloo_indexer ${akonadi_baloo_indexer_SRCS})

target_link_libraries(akonadi_baloo_indexer
    Qt5::Core
    KF5::KDELibs4Support
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Abc
    KF5::AkonadiAgentBase
    KF5::PimUtils
    KF5::ItemModels
    KF5::BalooXapian
    KF5::PimUtils
    KF5::CalendarCore
)

install(TARGETS akonadi_baloo_indexer ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadibalooindexingagent.desktop
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
#reactivate
#add_subdirectory(tests)
#REACTIVATE
#add_subdirectory(autotests)
