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


#
# Remaining Time
#
set_source_files_properties(${CMAKE_BINARY_DIR}/src/dbus/extractor_interface.cpp PROPERTIES GENERATED 1)

add_executable(remainingtimetest-manual
  remainingtimetest.cpp
  ${CMAKE_BINARY_DIR}/src/dbus/extractor_interface.cpp
)

add_dependencies(remainingtimetest-manual BalooDBusInterfaces)

target_link_libraries(remainingtimetest-manual
  Qt5::Core
  baloofilecommon
)
