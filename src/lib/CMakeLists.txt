kde_enable_exceptions()

set(CORE_SRCS
    searchstore.cpp
    term.cpp
    query.cpp
    queryrunnable.cpp
    resultiterator.cpp
    result.cpp
    advancedqueryparser.cpp

    file.cpp
    filefetchjob.cpp
    filemapping.cpp
    filemonitor.cpp
    taglistjob.cpp
    db.cpp

    indexerconfig.cpp
    ../file/fileindexerconfig.cpp
    ../file/storagedevices.cpp
    ../file/regexpcache.cpp
    ../file/fileexcludefilters.cpp
)

add_library(KF5BalooCore ${CORE_SRCS})

add_library(KF5::BalooCore ALIAS KF5BalooCore)

target_link_libraries(KF5BalooCore
    PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::FileMetaData

    PRIVATE
    KF5::ConfigCore
    Qt5::DBus
    Qt5::Sql
    KF5::Solid
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5BalooCore PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooCore
)
target_include_directories(KF5BalooCore INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Baloo>")

generate_export_header(KF5BalooCore BASE_NAME BALOO_CORE EXPORT_FILE_NAME core_export.h)

ecm_generate_headers(KF5BalooCore_CamelCase_HEADERS
    HEADER_NAMES
    Term
    Query
    QueryRunnable
    Result
    ResultIterator
    SearchStore

    File
    FileFetchJob
    FileMonitor
    TagListJob
    IndexerConfig

    PREFIX baloo
    REQUIRED_HEADERS KF5BalooCore_HEADERS
)

install(TARGETS KF5BalooCore EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/core_export.h
    ${KF5BalooCore_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/baloo
    COMPONENT Devel
)

install(FILES
    ${KF5BalooCore_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/Baloo
    COMPONENT Devel
)

add_subdirectory(tests)
add_subdirectory(autotests)
