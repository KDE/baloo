include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(indexerconfigtest
               indexerconfigtest.cpp
               ../fileindexerconfig.cpp
               ../regexpcache.cpp
               ../fileexcludefilters.cpp)

target_link_libraries(indexerconfigtest
  Qt5::Core
  KF5::CoreAddons
  KF5::ConfigCore
  baloocore
)

add_executable(basicindexingqueuetest-manual
                    basicindexingqueuetest.cpp
                    ../fileindexerconfig.cpp
                    ../regexpcache.cpp
                    ../fileexcludefilters.cpp
                    ../basicindexingqueue.cpp
                    ../indexingqueue.cpp
                    ../basicindexingjob.cpp
                    ../database.cpp
                    ../lib/filemapping.cpp
                    ../commitqueue.cpp
)

target_link_libraries(basicindexingqueuetest-manual
  Qt5::Core
  Qt5::Sql
  KF5::FileMetaData
  KF5::KIOCore
  KF5::ConfigCore
  ${XAPIAN_LIBRARIES}
  baloocore
  balooxapian
)

add_executable(fileindexingqueuetest-manual
                    fileindexingqueuetest.cpp
                    fileindexer.cpp
                    ../fileindexerconfig.cpp
                    ../regexpcache.cpp
                    ../fileexcludefilters.cpp
                    ../basicindexingqueue.cpp
                    ../indexingqueue.cpp
                    ../basicindexingjob.cpp
                    ../database.cpp
                    ../lib/filemapping.cpp
                    ../commitqueue.cpp
)

target_link_libraries(fileindexingqueuetest-manual
  Qt5::Core
  Qt5::Sql
  KF5::FileMetaData
  KF5::KIOCore
  KF5::ConfigCore
  ${XAPIAN_LIBRARIES}
  baloocore
  balooxapian
)

if(BUILD_KINOTIFY)
  add_executable(inotifytest inotify.cpp ../kinotify.cpp)
  target_link_libraries(inotifytest
    Qt5::Core
  )
endif()
