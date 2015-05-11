add_executable(unindexedfileiteratorbenchmark unindexedfileiteratortest.cpp)
target_link_libraries(unindexedfileiteratorbenchmark
    Qt5::Core
    baloofilecommon
)
ecm_mark_as_test(unindexedfileiteratorbenchmark)
