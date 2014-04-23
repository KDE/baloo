include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${KDEPIMLIBS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-fexceptions)

find_package(KF5KDELibs4Support REQUIRED)


add_executable(akonadi_baloo_indexer
    abstractindexer.cpp
    agent.cpp
    emailindexer.cpp
    contactindexer.cpp
    akonotesindexer.cpp
    ../../file/priority.cpp
)

qt4_add_dbus_adaptor(akonadi_baloo_indexer_SRCS org.freedesktop.Akonadi.BalooIndexer.xml agent.h BalooIndexingAgent)

add_executable(akonadi_baloo_indexer ${akonadi_baloo_indexer_SRCS})

target_link_libraries(akonadi_baloo_indexer
    Qt5::Core
    KF5::KDE4Support
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    ${XAPIAN_LIBRARIES}
    ${KDEPIMLIBS_KPIMUTILS_LIBS}
    balooxapian
)

install(TARGETS akonadi_baloo_indexer ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadibalooindexingagent.desktop
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

add_subdirectory(tests)
add_subdirectory(autotests)
