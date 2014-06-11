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

add_library(KF5BalooFiles SHARED ${FILE_LIB_SRCS})
add_library(KF5::BalooFiles ALIAS KF5BalooFiles)


target_link_libraries(KF5BalooFiles PUBLIC
    Qt5::Core
    KF5::CoreAddons
)
target_link_libraries(KF5BalooFiles PRIVATE
    KF5::FileMetaData
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

install(TARGETS KF5BalooFiles ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS KF5BalooFiles EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KF5BalooFiles_HEADERS
    HEADER_NAMES
    File
    FileFetchJob
    FileModifyjob
    FileMonitor
    TagListJob
    IndexerConfig

    REQUIRED_HEADERS KF5BalooFiles_HEADERS
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/file_export.h
    ${KF5BalooFiles_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo COMPONENT Devel
)

add_subdirectory(tests)
add_subdirectory(autotests)
