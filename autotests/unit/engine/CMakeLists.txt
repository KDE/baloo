MACRO(BALOO_ENGINE_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp dbtest.cpp
        TEST_NAME ${_testname}
        LINK_LIBRARIES Qt6::Test KF6::BalooEngine
    )
  ENDFOREACH()
ENDMACRO()

baloo_engine_auto_tests(
    positiondbtest
    postingdbtest
    documentdbtest
    documenturldbtest
    documentiddbtest
    documentdatadbtest
    documenttimedbtest
    idtreedbtest
    idfilenamedbtest
    mtimedbtest

    # Query
    andpostingiteratortest
    orpostingiteratortest
    postingiteratortest
    phraseanditeratortest
)

add_executable(termgeneratortest_bin
    termgeneratortest.cpp
)
target_link_libraries(termgeneratortest_bin
    Qt6::Test
    KF6::BalooEngine
)
ecm_mark_as_test(termgeneratortest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME termgeneratortest COMMAND termgeneratortest_bin "-iterations" "1")
