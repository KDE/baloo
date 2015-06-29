add_executable(indexerconfigtest indexerconfigtest.cpp)
target_link_libraries(indexerconfigtest baloofilecommon)

add_executable(inotifytest inotify.cpp)
target_link_libraries(inotifytest Qt5::Core baloofilecommon)

#
# Storage Devices
#
add_executable(storagedevicestest storagedevicestest.cpp)
target_link_libraries(storagedevicestest Qt5::Core KF5::Solid baloofilecommon)

add_executable(filtereddiriteratorbenchmark filtereddiriteratorbenchmark.cpp)
target_link_libraries(filtereddiriteratorbenchmark Qt5::Core baloofilecommon)
