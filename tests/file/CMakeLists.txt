add_executable(indexerconfigtest indexerconfigtest.cpp)
target_link_libraries(indexerconfigtest baloofilecommon)
ecm_mark_as_test(indexerconfigtest)

add_executable(inotifytest inotify.cpp)
target_link_libraries(inotifytest Qt${QT_MAJOR_VERSION}::Core baloofilecommon)
ecm_mark_as_test(inotifytest)

#
# Storage Devices
#
add_executable(storagedevicestest storagedevicestest.cpp)
target_link_libraries(storagedevicestest Qt${QT_MAJOR_VERSION}::Core KF5::Solid baloofilecommon)
ecm_mark_as_test(storagedevicestest)

# Benchmark, compiled, but not run automatically with ctest
add_executable(filtereddiriteratorbenchmark filtereddiriteratorbenchmark.cpp)
target_link_libraries(filtereddiriteratorbenchmark Qt${QT_MAJOR_VERSION}::Core baloofilecommon)

#
# Remaining Time //TODO: rewrite the test to use mainhub, instead of using the indexers directly
#
#[[
add_executable(remainingtimetest-manual
  remainingtimetest.cpp
)

target_link_libraries(remainingtimetest-manual
  Qt${QT_MAJOR_VERSION}::Core
  baloofilecommon
  BalooDBusExtractorInterface
)

ecm_mark_as_test(remainingtimetest-manual)
]]
