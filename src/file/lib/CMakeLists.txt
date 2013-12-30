set(FILE_LIB_SRCS
    file.cpp
    filefetchjob.cpp
    filemapping.cpp
    filemonitor.cpp
    db.cpp
)

kde4_add_library(baloofiles SHARED ${FILE_LIB_SRCS})

target_link_libraries(baloofiles
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
    ${QJSON_LIBRARIES}
    baloocore
)

set_target_properties(baloofiles PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS baloofiles ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    file.h
    filefetchjob.h
    filemonitor.h
    file_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)
