set(BALOO_LIB_SRCS
    term.cpp
    query.cpp
    queryrunnable.cpp
    resultiterator.cpp
    advancedqueryparser.cpp

    file.cpp
    filemonitor.cpp
    taglistjob.cpp

    indexerconfig.cpp
    ../file/fileindexerconfig.cpp
    ../file/storagedevices.cpp
    ../file/regexpcache.cpp
    ../file/fileexcludefilters.cpp
    ../file/baloodebug.cpp

    searchstore.cpp
)

add_library(KF5Baloo ${BALOO_LIB_SRCS})

add_library(KF5::Baloo ALIAS KF5Baloo)

target_link_libraries(KF5Baloo
    PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::FileMetaData

    PRIVATE
    KF5::ConfigCore
    Qt5::DBus
    KF5::Solid
    KF5::BalooEngine
)

set_target_properties(KF5Baloo PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME Baloo
)
target_include_directories(KF5Baloo INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Baloo>")

generate_export_header(KF5Baloo BASE_NAME BALOO_CORE EXPORT_FILE_NAME core_export.h)

ecm_generate_headers(KF5Baloo_CamelCase_HEADERS
    HEADER_NAMES
    Query
    QueryRunnable
    ResultIterator

    File
    FileMonitor
    TagListJob
    IndexerConfig

    PREFIX baloo
    REQUIRED_HEADERS KF5Baloo_HEADERS
)

install(TARGETS KF5Baloo EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/core_export.h
    ${KF5Baloo_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/baloo
    COMPONENT Devel
)

install(FILES
    ${KF5Baloo_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/Baloo
    COMPONENT Devel
)

if (NOT WIN32)
   configure_file (
      ${CMAKE_CURRENT_SOURCE_DIR}/Baloo.pc.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/Baloo.pc
      )
   install (
      FILES ${CMAKE_CURRENT_BINARY_DIR}/Baloo.pc
      DESTINATION ${KDE_INSTALL_LIBDIR}/pkgconfig
      )
endif ()
