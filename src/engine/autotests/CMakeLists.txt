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
