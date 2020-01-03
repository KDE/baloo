include_directories(${Inotify_INCLUDE_DIRS})
add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file5\")

set(file_static_lib_SRCS
    # File Indexer
    mainhub.cpp
    mainadaptor.cpp
    fileindexerconfig.cpp
    basicindexingjob.cpp
    powerstatemonitor.cpp

    fileindexscheduler.cpp

    firstrunindexer.cpp
    newfileindexer.cpp
    xattrindexer.cpp
    modifiedfileindexer.cpp
    unindexedfileindexer.cpp

    filecontentindexer.cpp
    filecontentindexerprovider.cpp
    extractorprocess.cpp
    timeestimator.cpp

    indexcleaner.cpp

    # Common
    priority.cpp
    regexpcache.cpp
    fileexcludefilters.cpp
    storagedevices.cpp
    filtereddiriterator.cpp
    unindexedfileiterator.cpp
    migrator.cpp
    fileinfo.cpp

     # File Watcher
    filewatch.cpp
    pendingfilequeue.cpp
    metadatamover.cpp
    org.kde.BalooWatcherApplication.xml
    pendingfile.cpp
    kinotify.cpp

    propertydata.cpp
)

ecm_qt_declare_logging_category(file_static_lib_SRCS HEADER baloodebug.h IDENTIFIER BALOO CATEGORY_NAME org.kde.baloo)

qt5_add_dbus_interface(file_static_lib_SRCS org.kde.BalooWatcherApplication.xml baloowatcherapplication_interface)
kconfig_add_kcfg_files(file_static_lib_SRCS ../lib/baloosettings.kcfgc GENERATE_MOC)

add_library(baloofilecommon STATIC ${file_static_lib_SRCS})
target_link_libraries(baloofilecommon
    Qt5::DBus
    KF5::I18n
    KF5::Solid
    KF5::FileMetaData
    KF5::Crash
    KF5::ConfigCore
    KF5::BalooEngine
    ${Inotify_LIBRARIES}
)

set(file_SRCS
    main.cpp
)

add_executable(baloo_file ${file_SRCS})
ecm_mark_nongui_executable(baloo_file)
target_compile_definitions(baloo_file PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloo_file
    baloofilecommon
)

install(TARGETS baloo_file ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES baloo_file.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

add_subdirectory(extractor)

install(FILES
    org.kde.BalooWatcherApplication.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
