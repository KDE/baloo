set(EXTRACTOR_TEST_SRCS
    extractortest.cpp
)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

ecm_add_test(${EXTRACTOR_TEST_SRCS}
    TEST_NAME "extractortest"
    LINK_LIBRARIES Qt5::Test KF5::BalooEngine
)
