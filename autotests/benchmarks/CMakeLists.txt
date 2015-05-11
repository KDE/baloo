ecm_add_test(databasebenchmark.cpp
    TEST_NAME "databasebenchmark"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)

ecm_add_test(positioncodecbenchmark.cpp
    TEST_NAME "positioncodecbenchmark"
    LINK_LIBRARIES Qt5::Test KF5::BalooCodecs
)
