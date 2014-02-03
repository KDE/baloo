include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${KDEPIMLIBS_INCLUDE_DIRS}
)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

kde4_add_executable(akonadi_baloo_indexer
    abstractindexer.cpp
    agent.cpp
    emailindexer.cpp
    contactindexer.cpp
)

target_link_libraries(akonadi_baloo_indexer
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
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
add_subdirectory(autotests)