add_executable(indexerconfigtest indexerconfigtest.cpp)
target_link_libraries(indexerconfigtest baloofilecommon)
ecm_mark_as_test(indexerconfigtest)

add_executable(inotifytest inotify.cpp)
target_link_libraries(inotifytest Qt5::Core baloofilecommon)
ecm_mark_as_test(inotifytest)

#
# Storage Devices
#
add_executable(storagedevicestest storagedevicestest.cpp)
target_link_libraries(storagedevicestest Qt5::Core KF5::Solid baloofilecommon)
ecm_mark_as_test(storagedevicestest)

# Benchmark, compiled, but not run automatically with ctest
add_executable(filtereddiriteratorbenchmark filtereddiriteratorbenchmark.cpp)
target_link_libraries(filtereddiriteratorbenchmark Qt5::Core baloofilecommon)

#
# Remaining Time //TODO: rewrite the test to use mainhub, instead of using the indexers directly
#
#[[set_source_files_properties(${CMAKE_BINARY_DIR}/src/dbus/extractor_interface.cpp PROPERTIES GENERATED 1)

 add_executable(remainingtimetest-manual
  remainingtimetest.cpp
  ${CMAKE_BINARY_DIR}/src/dbus/extractor_interface.cpp
)

add_dependencies(remainingtimetest-manual BalooDBusInterfaces)

target_link_libraries(remainingtimetest-manual
  Qt5::Core
  baloofilecommon
)

ecm_mark_as_test(remainingtimetest-manual)
]]
