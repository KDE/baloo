MACRO(BALOO_ENGINE_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp
        TEST_NAME ${_testname}
        LINK_LIBRARIES Qt5::Test KF5::BalooEngine
    )
  ENDFOREACH()
ENDMACRO()

baloo_engine_auto_tests(
    querytest
    writetransactiontest
)
