ecm_add_test(termgeneratortest.cpp ../termgenerator.cpp
    TEST_NAME "termgeneratortest"
    LINK_LIBRARIES Qt5::Test ${XAPIAN_LIBRARIES}
)
