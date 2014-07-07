set(schedulertest_SRC
    schedulertest.cpp
)

ecm_add_test(${schedulertest_SRC}
    TEST_NAME "schedulertest"
    LINK_LIBRARIES Qt5::Test Qt5::Sql ${XAPIAN_LIBRARIES} KF5::BalooXapian baloofilecommon
)
