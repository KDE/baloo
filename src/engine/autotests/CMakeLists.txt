ecm_add_test(positiondbtest.cpp
    TEST_NAME "positiondbtest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)

ecm_add_test(postingdbtest.cpp
    TEST_NAME "postingdbtest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)

ecm_add_test(documentdbtest.cpp
    TEST_NAME "documentdbtest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)

ecm_add_test(databasetest.cpp
    TEST_NAME "databasetest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)

ecm_add_test(andpostingiteratortest.cpp
    TEST_NAME "andpostingiteratortest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)

ecm_add_test(documenturldbtest.cpp
    TEST_NAME "documenturldbtest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)

ecm_add_test(urldocumentdbtest.cpp
    TEST_NAME "urldocumentdbtest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)
