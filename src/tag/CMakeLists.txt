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

kde4_add_library(viziertags SHARED ${TAGS_SRCS})

target_link_libraries(viziertags
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    viziercore
)

set_target_properties(viziertags PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS viziertags ${INSTALL_TARGETS_DEFAULT_ARGS})

#install(FILES
#    tag.h
#    tagfetchjob.h
#
#    tagrelation.h
#
#    DESTINATION ${INCLUDE_INSTALL_DIR}/vizier COMPONENT Devel
#)

add_subdirectory(autotests)
add_subdirectory(tests)
