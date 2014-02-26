add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(CORE_SRCS
    searchstore.cpp
    term.cpp
    query.cpp
    queryrunnable.cpp
    resultiterator.cpp
    result.cpp
)

kde4_add_library(baloocore SHARED ${CORE_SRCS})

target_link_libraries(baloocore
    ${QT_QTCORE_LIBRARY}
    ${QJSON_LIBRARIES}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
)

set_target_properties(baloocore PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_VERSION_MAJOR}
)

install(TARGETS baloocore EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    term.h
    query.h
    queryrunnable.h
    result.h
    resultiterator.h
    searchstore.h

    core_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

install(FILES baloosearchstore.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

add_subdirectory(tests)
add_subdirectory(autotests)
