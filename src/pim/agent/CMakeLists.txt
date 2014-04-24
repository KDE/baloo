include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${KDEPIMLIBS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-fexceptions)

find_package(KF5KDELibs4Support REQUIRED)
find_package(KF5Abc REQUIRED)
find_package(KF5KMime REQUIRED)
find_package(KF5PimUtils REQUIRED)
find_package(KF5AkonadiKMime REQUIRED)
#find_package(KF5ItemModels REQUIRED)


set(akonadi_baloo_indexer_SRCS
    abstractindexer.cpp
    agent.cpp
    emailindexer.cpp
    contactindexer.cpp
    akonotesindexer.cpp
    ../../file/priority.cpp
)

qt5_add_dbus_adaptor(akonadi_baloo_indexer_SRCS org.freedesktop.Akonadi.BalooIndexer.xml agent.h BalooIndexingAgent)

add_executable(akonadi_baloo_indexer ${akonadi_baloo_indexer_SRCS})

target_link_libraries(akonadi_baloo_indexer
    Qt5::Core
    KF5::KDELibs4Support
    KF5::AkonadiCore
    KF5::AkonadiKMime
    KF5::KMime
    KF5::Abc
    KF5::AkonadiAgentBase
    ${XAPIAN_LIBRARIES}
    KF5::PimUtils
    KF5::ItemModels
    balooxapian
)

install(TARGETS akonadi_baloo_indexer ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadibalooindexingagent.desktop
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

#Reactivate
add_subdirectory(tests)
add_subdirectory(autotests)
