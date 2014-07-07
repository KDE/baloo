include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(indexerconfigtest
               indexerconfigtest.cpp)

target_link_libraries(indexerconfigtest
  Qt5::Core
  KF5::CoreAddons
  KF5::ConfigCore
  KF5::BalooCore
  baloofilecommon
)

add_executable(basicindexingqueuetest-manual
                    basicindexingqueuetest.cpp
)

target_link_libraries(basicindexingqueuetest-manual
  Qt5::Core
  Qt5::Sql
  KF5::FileMetaData
  KF5::KIOCore
  KF5::ConfigCore
  ${XAPIAN_LIBRARIES}
  KF5::BalooCore
  KF5::BalooXapian
  baloofilecommon
)

add_executable(fileindexingqueuetest-manual
                    fileindexingqueuetest.cpp
                    fileextractor.cpp
)

target_link_libraries(fileindexingqueuetest-manual
  Qt5::Core
  Qt5::Sql
  KF5::FileMetaData
  KF5::KIOCore
  KF5::ConfigCore
  ${XAPIAN_LIBRARIES}
  KF5::BalooCore
  KF5::BalooXapian
  baloofilecommon
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(BUILD_KINOTIFY)
      add_executable(inotifytest inotify.cpp ../kinotify.cpp)
      target_link_libraries(inotifytest
        Qt5::Core
      )
    endif()
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

