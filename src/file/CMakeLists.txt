add_definitions(${KDE4_ENABLE_EXCEPTIONS})

add_subdirectory(lib)
add_subdirectory(search)

set(file_SRCS
    # Filewatch
    filewatch.cpp
    updaterequest.cpp
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

kde4_add_executable(baloo_file ${file_SRCS})

target_link_libraries(baloo_file
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIDLETIME_LIBS}
    ${KDE4_SOLID_LIBS}
    ${XAPIAN_LIBRARIES}
    ${QJSON_LIBRARIES}
    baloocore
    baloofiles
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
