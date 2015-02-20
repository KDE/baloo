include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(indexerconfigtest
               indexerconfigtest.cpp)

target_link_libraries(indexerconfigtest
  Qt5::Core
  KF5::CoreAddons
  KF5::ConfigCore
  baloofilecommon
)

add_executable(basicindexingqueuetest-manual
                    basicindexingqueuetest.cpp
)

target_link_libraries(basicindexingqueuetest-manual
  Qt5::Core
  baloofilecommon
)

#add_executable(fileindexingqueuetest-manual
#                    fileindexingqueuetest.cpp
#                    fileextractor.cpp
#)
#
#target_link_libraries(fileindexingqueuetest-manual
#  Qt5::Core
#  KF5::FileMetaData
#  KF5::KIOCore
#  KF5::ConfigCore
#  ${XAPIAN_LIBRARIES}
#  KF5::Baloo
#  KF5::BalooXapian
#  baloofilecommon
#)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(BUILD_KINOTIFY)
      add_executable(inotifytest inotify.cpp)
      target_link_libraries(inotifytest
        Qt5::Core baloofilecommon
      )
    endif()
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

#
# Storage Devices
#
add_executable(storagedevicestest storagedevicestest.cpp)
target_link_libraries(storagedevicestest Qt5::Core KF5::Solid baloofilecommon)

add_executable(filtereddiriteratorbenchmark filtereddiriteratorbenchmark.cpp)
target_link_libraries(filtereddiriteratorbenchmark Qt5::Core baloofilecommon)
