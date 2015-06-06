set(EXTRACTOR_IO_TEST_SRCS
    iohandlertest.cpp
    ../iohandler.cpp
)

ecm_add_test(${EXTRACTOR_IO_TEST_SRCS}
    TEST_NAME "extractorIOTest"
    LINK_LIBRARIES Qt5::Core Qt5::Test
)
