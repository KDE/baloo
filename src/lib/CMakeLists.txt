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
    ../file/propertydata.cpp

    searchstore.cpp
)
ecm_qt_declare_logging_category(BALOO_LIB_SRCS
    HEADER baloodebug.h
    IDENTIFIER BALOO
    CATEGORY_NAME kf.baloo
    OLD_CATEGORY_NAMES org.kde.baloo
    DESCRIPTION "Baloo File debug"
    EXPORT BALOO
)

kconfig_add_kcfg_files(BALOO_LIB_SRCS baloosettings.kcfgc GENERATE_MOC)
add_library(KF5Baloo ${BALOO_LIB_SRCS})

add_library(KF5::Baloo ALIAS KF5Baloo)

target_link_libraries(KF5Baloo
    PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    KF5::CoreAddons
    KF5::FileMetaData

    PRIVATE
    KF5::ConfigCore
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::Solid
    KF5::BalooEngine
    BalooDBusMainInterface
)

set_target_properties(KF5Baloo PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME Baloo
)
target_include_directories(KF5Baloo INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Baloo>")

ecm_generate_export_header(KF5Baloo
    EXPORT_FILE_NAME core_export.h
    BASE_NAME BALOO_CORE
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.55 5.69
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

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

install(TARGETS KF5Baloo EXPORT KF5BalooTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/core_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/baloosettings.h
    ${KF5Baloo_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Baloo/baloo
    COMPONENT Devel
)

install(FILES
    ${KF5Baloo_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Baloo/Baloo
    COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5Baloo_QCH
        NAME Baloo
        BASE_NAME KF5Baloo
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KF5Baloo_HEADERS}
            "${CMAKE_SOURCE_DIR}/docs/distributing.md"
            "${CMAKE_SOURCE_DIR}/docs/development/build-instructions.md"
            "${CMAKE_SOURCE_DIR}/docs/user/searching.md"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
            KF5CoreAddons_QCH
            KF5FileMetaData_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            BALOO_CORE_EXPORT
            BALOO_CORE_DEPRECATED
            BALOO_CORE_DEPRECATED_EXPORT
            "BALOO_DEPRECATED_VERSION(x, y, t)"
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

include(ECMGeneratePriFile)
ecm_generate_pri_file(
    BASE_NAME Baloo
    LIB_NAME KF5Baloo
    DEPS "core KCoreAddons KFileMetaData"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/Baloo
)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
