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
#  KF5::Baloo
#  baloofilecommon
#)

add_executable(inotifytest inotify.cpp)
target_link_libraries(inotifytest Qt5::Core baloofilecommon)

#
# Storage Devices
#
add_executable(storagedevicestest storagedevicestest.cpp)
target_link_libraries(storagedevicestest Qt5::Core KF5::Solid baloofilecommon)

add_executable(filtereddiriteratorbenchmark filtereddiriteratorbenchmark.cpp)
target_link_libraries(filtereddiriteratorbenchmark Qt5::Core baloofilecommon)


#
# Remaining Time
#
add_executable(remainingtimetest-manual remainingtimetest.cpp)
target_link_libraries(remainingtimetest-manual
  Qt5::Core
  baloofilecommon
)
