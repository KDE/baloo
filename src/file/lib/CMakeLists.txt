set(FILE_LIB_SRCS
    file.cpp
    filefetchjob.cpp
    filemodifyjob.cpp
    filemapping.cpp
    filemonitor.cpp
    taglistjob.cpp
    db.cpp

    indexerconfig.cpp
    ../fileindexerconfig.cpp
    ../regexpcache.cpp
    ../fileexcludefilters.cpp
    xattrdetector.cpp
    filecustommetadata.cpp
)

add_library(baloofiles SHARED ${FILE_LIB_SRCS})

target_link_libraries(baloofiles
    KF5::FileMetaData
    Qt5::DBus
    Qt5::Sql
    KF5::Solid
    ${XAPIAN_LIBRARIES}
    baloocore
    balooxapian
)

set_target_properties(baloofiles PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
)

generate_export_header(baloofiles BASE_NAME BALOO_FILE EXPORT_FILE_NAME file_export.h)

install(TARGETS baloofiles EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    file.h
    filefetchjob.h
    filemodifyjob.h
    filemonitor.h
    taglistjob.h
    indexerconfig.h
    ${CMAKE_CURRENT_BINARY_DIR}/file_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(tests)
add_subdirectory(autotests)
