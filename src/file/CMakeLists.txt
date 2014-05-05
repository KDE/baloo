kde_enable_exceptions()

add_subdirectory(lib)
add_subdirectory(search)

set(file_SRCS
    # Filewatch
    filewatch.cpp
    activefilequeue.cpp
    metadatamover.cpp
    database.cpp

    # File Indexer
    fileindexer.cpp
    fileindexerconfig.cpp
    indexscheduler.cpp
    indexingqueue.cpp
    basicindexingqueue.cpp
    fileindexingqueue.cpp
    commitqueue.cpp
    basicindexingjob.cpp
    fileindexingjob.cpp
    eventmonitor.cpp
    util.cpp

    # Common
    priority.cpp
    regexpcache.cpp
    fileexcludefilters.cpp
    # removabledeviceindexnotification.cpp
    # removablemediacache.cpp

    main.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(file_SRCS
        ${file_SRCS}
        kinotify.cpp
    )
    add_definitions(-DBUILD_KINOTIFY)
endif()

add_executable(baloo_file ${file_SRCS})

target_link_libraries(baloo_file
    Qt5::Sql
    Qt5::DBus
    KF5::KIOWidgets
    KF5::IdleTime
    KF5::I18n
    KF5::Solid
    KF5::Auth
    KF5::FileMetaData
    KF5::Crash
    KF5::ConfigCore
    ${XAPIAN_LIBRARIES}
    baloocore
    baloofiles
    balooxapian
)

install(TARGETS baloo_file ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES baloo_file.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES org.kde.baloo.file.indexer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_subdirectory(autotest)
add_subdirectory(tests)
add_subdirectory(extractor)
add_subdirectory(kcm)
add_subdirectory(inotify)
add_subdirectory(cleaner)
