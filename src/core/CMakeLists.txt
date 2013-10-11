set(CORE_SRCS
    item.cpp
    relation.cpp

    itemfetchjob.cpp
    itemcreatejob.cpp
    itemsavejob.cpp
    itemremovejob.cpp
)

kde4_add_library(viziercore SHARED ${CORE_SRCS})

target_link_libraries(viziercore
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
)

set_target_properties(viziercore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS viziercore ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    item.h
    itemfetchjob.h

    relation.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/vizier COMPONENT Devel
)
