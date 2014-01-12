set(FILE_LIB_SRCS
    file.cpp
    filefetchjob.cpp
    filemodifyjob.cpp
    filemapping.cpp
    filemonitor.cpp
    taglistjob.cpp
    db.cpp
)

add_library(baloofiles SHARED ${FILE_LIB_SRCS})

target_link_libraries(baloofiles
    Qt5::Core
    Qt5::Sql
    KF5::KDE4Support
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
    filemodifyjob.h
    filemonitor.h
    taglistjob.h
    file_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(autotests)
