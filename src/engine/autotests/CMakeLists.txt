ecm_add_test(andpostinglisttest.cpp ../andpostinglist.cpp
    TEST_NAME "andpostinglisttest"
    LINK_LIBRARIES Qt5::Test
)

ecm_add_test(postingdbtest.cpp ../postingdb.cpp ../andpostinglist.cpp
    TEST_NAME "postingdbtest"
    LINK_LIBRARIES Qt5::Test ${LMDB_LIBRARIES}
)

ecm_add_test(documentdbtest.cpp ../documentdb.cpp ../andpostinglist.cpp
    TEST_NAME "documentdbtest"
    LINK_LIBRARIES Qt5::Test ${LMDB_LIBRARIES}
)

ecm_add_test(databasetest.cpp ../database.cpp ../document.cpp ../documentdb.cpp ../postingdb.cpp ../andpostinglist.cpp
    TEST_NAME "databasetest"
    LINK_LIBRARIES Qt5::Test ${LMDB_LIBRARIES}
)

ecm_add_test(andpostingiteratortest.cpp
    TEST_NAME "andpostingiteratortest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)
