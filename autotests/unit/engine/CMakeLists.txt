MACRO(BALOO_ENGINE_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp singledbtest.cpp
        TEST_NAME ${_testname}
        LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::BalooEngine
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

    termgeneratortest
    queryparsertest

    # Query
    andpostingiteratortest
    orpostingiteratortest
    postingiteratortest
    phraseanditeratortest
    transactiontest
)
