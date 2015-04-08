#
# Query Serialization
#
set(querySerialization_SRC queryserializationtest.cpp)

ecm_add_test(${querySerialization_SRC}
    TEST_NAME "queryserializationtest"
    LINK_LIBRARIES Qt5::Test KF5::Baloo
)

#
# Advanced Query Parser
#
ecm_add_test(advancedqueryparsertest.cpp ../../../src/lib/advancedqueryparser.cpp ../../../src/lib/term.cpp
    TEST_NAME "advancedqueryparsertest"
    LINK_LIBRARIES Qt5::Test KF5::Baloo
)

#
# Fetch Job
#
set(filefetchjobtest_SRC filefetchjobtest.cpp db.cpp ../../../src/lib/file.cpp)
ecm_add_test(${filefetchjobtest_SRC}
    TEST_NAME "filefetchjobtest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine KF5::FileMetaData
)

