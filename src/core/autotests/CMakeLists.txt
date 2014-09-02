#
# Query Serialization
#
set(querySerialization_SRC queryserializationtest.cpp)

ecm_add_test(${querySerialization_SRC}
    TEST_NAME "queryserializationtest"
    LINK_LIBRARIES Qt5::Test KF5::BalooCore
)

#
# Advanced Query Parser
#
ecm_add_test(advancedqueryparsertest.cpp
    TEST_NAME "advancedqueryparsertest"
    LINK_LIBRARIES Qt5::Test KF5::BalooCore
)
