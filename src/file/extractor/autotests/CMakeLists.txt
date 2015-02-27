set(EXTRACTOR_TEST_SRCS
    extractortest.cpp
)

ecm_add_test(${EXTRACTOR_TEST_SRCS}
    TEST_NAME "extractortest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)
