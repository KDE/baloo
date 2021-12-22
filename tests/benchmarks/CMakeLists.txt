add_executable(unindexedfileiteratorbenchmark unindexedfileiteratortest.cpp)
target_link_libraries(unindexedfileiteratorbenchmark
    Qt${QT_MAJOR_VERSION}::Core
    baloofilecommon
)
ecm_mark_as_test(unindexedfileiteratorbenchmark)
