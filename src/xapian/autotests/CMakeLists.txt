ecm_add_test(termgeneratortest.cpp
    TEST_NAME "termgeneratortest"
    LINK_LIBRARIES Qt5::Test KF5::BalooXapian
)

ecm_add_test(queryparsertest.cpp ../queryparser.cpp
    TEST_NAME "queryparsertest"
    LINK_LIBRARIES Qt5::Test KF5::BalooXapian
)
