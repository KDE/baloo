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
    ../storagedevices.cpp
    ../regexpcache.cpp
    ../fileexcludefilters.cpp
)

add_library(KF5BalooFiles ${FILE_LIB_SRCS})
add_library(KF5::BalooFiles ALIAS KF5BalooFiles)


target_link_libraries(KF5BalooFiles
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
    KF5::BalooCore
    KF5::BalooXapian
)

set_target_properties(KF5BalooFiles PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooFiles
)
target_include_directories(KF5BalooFiles INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Baloo>")

generate_export_header(KF5BalooFiles BASE_NAME BALOO_FILE EXPORT_FILE_NAME file_export.h)

install(TARGETS KF5BalooFiles EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KF5BalooFiles_CamelCase_HEADERS
    HEADER_NAMES
    File
    FileFetchJob
    FileModifyjob
    FileMonitor
    TagListJob
    IndexerConfig

    PREFIX baloo
    REQUIRED_HEADERS KF5BalooFiles_HEADERS
)

install(FILES
    ${KF5BalooFiles_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/Baloo
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/file_export.h
    ${KF5BalooFiles_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/baloo
    COMPONENT Devel
)

add_subdirectory(autotests)
