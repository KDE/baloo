add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(CORE_SRCS
    item.cpp
    relation.cpp

    itemfetchjob.cpp
    itemcreatejob.cpp
    itemsavejob.cpp
    itemremovejob.cpp

    relationfetchjob.cpp
    relationcreatejob.cpp
    relationremovejob.cpp

    datastore.cpp

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
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS baloocore EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    item.h
    itemfetchjob.h
    itemcreatejob.h
    itemsavejob.h
    itemremovejob.h

    relation.h
    relationfetchjob.h
    relationremovejob.h
    relationcreatejob.h

    datastore.h

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
