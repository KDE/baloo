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
    Qt5::Core
    Qt5::Sql
    KF5::KDE4Support
    KF5::Solid
    ${XAPIAN_LIBRARIES}
    ${QJSON_LIBRARIES}
    baloocore
)

set_target_properties(baloofiles PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
)

install(TARGETS baloofiles EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    file.h
    filefetchjob.h
    filemodifyjob.h
    filemonitor.h
    taglistjob.h
    indexerconfig.h
    file_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(tests)
#add_subdirectory(autotests)
