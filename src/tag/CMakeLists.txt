set(TAGS_SRCS
    tag.cpp
    tagfetchjob.cpp
    tagcreatejob.cpp
    tagsavejob.cpp
    tagremovejob.cpp

    tagrelation.cpp
    tagrelationfetchjob.cpp
    tagrelationcreatejob.cpp
    tagrelationremovejob.cpp

    tagrelationwatcher.cpp
    tagstore.cpp

    database.cpp
)

kde4_add_library(balootags SHARED ${TAGS_SRCS})

target_link_libraries(balootags
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    baloocore
)

set_target_properties(balootags PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS balootags EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    tag.h
    tagfetchjob.h
    tagsavejob.h
    tagcreatejob.h
    tagremovejob.h

    tagrelation.h
    tagrelationcreatejob.h
    tagrelationfetchjob.h
    tagrelationremovejob.h

    tagrelationwatcher.h
    tagstore.h
    tag_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(autotests)
add_subdirectory(tests)
