#
# Query Parser
#
set(queryParser_SRC naturalqueryparsertest.cpp)

ecm_add_test(naturalqueryparsertest.cpp
    TEST_NAME "naturalqueryparsertest"
    LINK_LIBRARIES Qt5::Test KF5::Baloo KF5::BalooNaturalQueryParser
)
