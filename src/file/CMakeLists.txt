add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file\")

set(file_static_lib_SRCS
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

    # Common
    priority.cpp
    regexpcache.cpp
    fileexcludefilters.cpp
    ../lib/filemapping.cpp
    storagedevices.cpp
    filtereddiriterator.cpp

     # File Watcher
#     filewatch.cpp
    pendingfilequeue.cpp
#     metadatamover.cpp
    pendingfile.cpp
)

set(BUILD_KINOTIFY False)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(BUILD_KINOTIFY True)
    set(file_static_lib_SRCS
        ${file_static_lib_SRCS}
        kinotify.cpp
    )
    add_definitions(-DBUILD_KINOTIFY)
endif()


add_library(baloofilecommon STATIC ${file_static_lib_SRCS})
target_link_libraries(baloofilecommon
    Qt5::DBus
    KF5::KIOWidgets
    KF5::IdleTime
    KF5::I18n
    KF5::Solid
    KF5::Auth
    KF5::FileMetaData
    KF5::Crash
    KF5::ConfigCore
    KF5::BalooEngine
    KF5::KDELibs4Support # For Solid
)
#
# set(file_SRCS
#     main.cpp
# )
#
# add_executable(baloo_file ${file_SRCS})
# target_compile_definitions(baloo_file PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
#
# target_link_libraries(baloo_file
#     Qt5::DBus
#     KF5::KIOWidgets
#     KF5::IdleTime
#     KF5::I18n
#     KF5::Solid
#     KF5::Auth
#     KF5::FileMetaData
#     KF5::Crash
#     KF5::ConfigCore
#     ${XAPIAN_LIBRARIES}
#     KF5::Baloo
#     KF5::BalooXapian
#     KF5::KDELibs4Support # For Solid
#     baloofilecommon
# )
#
# install(TARGETS baloo_file ${INSTALL_TARGETS_DEFAULT_ARGS})
# install(FILES baloo_file.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
# install(FILES org.kde.baloo.file.indexer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
#
add_subdirectory(autotest)
add_subdirectory(tests)
add_subdirectory(extractor)
#add_subdirectory(inotify)
#add_subdirectory(cleaner)
