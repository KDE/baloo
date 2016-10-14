MACRO(BALOO_CODECS_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp
        TEST_NAME ${_testname}
        LINK_LIBRARIES Qt5::Test KF5::BalooCodecs
    )
  ENDFOREACH()
ENDMACRO()

baloo_codecs_auto_tests(
    doctermscodectest
    postingcodectest
    positioncodectest
)
