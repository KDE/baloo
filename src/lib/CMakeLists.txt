set(DBUS_INTERFACES
  ${CMAKE_BINARY_DIR}/src/dbus/maininterface.cpp
)

# CMake <= 3.9 skipped automoc for generated files
# CMake >= 3.10 requires SKIP_AUTOMOC to actually skip automoc
SET_SOURCE_FILES_PROPERTIES(${DBUS_INTERFACES} PROPERTIES GENERATED 1 SKIP_AUTOMOC 1)

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

    ${DBUS_INTERFACES}
)

add_library(KF5Baloo ${BALOO_LIB_SRCS})
add_dependencies(KF5Baloo BalooDBusInterfaces)

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

if(BUILD_QCH)
    ecm_add_qch(
        KF5Baloo_QCH
        NAME Baloo
        BASE_NAME KF5Baloo
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KF5Baloo_HEADERS}
            "${CMAKE_SOURCE_DIR}/docs/distributing.md"
            "${CMAKE_SOURCE_DIR}/docs/faq.md"
            "${CMAKE_SOURCE_DIR}/docs/updatedb.md"
            "${CMAKE_SOURCE_DIR}/docs/development/build-instructions.md"
            "${CMAKE_SOURCE_DIR}/docs/development/coding-style.md"
            "${CMAKE_SOURCE_DIR}/docs/user/searching.md"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
            KF5CoreAddons_QCH
            KF5FileMetaData_QCH
        BLANK_MACROS
            BALOO_CORE_EXPORT
            BALOO_CORE_DEPRECATED
            BALOO_CORE_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

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
