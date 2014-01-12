include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${KDEPIMLIBS_INCLUDE_DIRS}
)

add_definitions(-fexceptions)

add_executable(akonadi_baloo_indexer
    abstractindexer.cpp
    agent.cpp
    emailindexer.cpp
    contactindexer.cpp
)

target_link_libraries(akonadi_baloo_indexer
    Qt5::Core
    KF5::KDE4Support
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    ${XAPIAN_LIBRARIES}
)

install(TARGETS akonadi_baloo_indexer ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadibalooindexingagent.desktop
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

add_subdirectory(tests)
