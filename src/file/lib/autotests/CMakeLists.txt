#
# Fetch Job
#
set(filefetchjobtest_SRC filefetchjobtest.cpp db.cpp ../filefetchjob.cpp ../filemapping.cpp ../file.cpp)

ecm_add_test(${filefetchjobtest_SRC}
    TEST_NAME "filefetchjobtest"
    LINK_LIBRARIES
        Qt5::Test
        Qt5::Sql
        ${XAPIAN_LIBRARIES}
        ${QJSON_LIBRARIES}
        baloocore
)

#
# Modify Job
#
set(filemodifyjobtest_SRC filemodifyjobtest.cpp db.cpp ../filemodifyjob.cpp ../filemapping.cpp ../file.cpp)

ecm_add_test(${filemodifyjobtest_SRC}
    TEST_NAME "filemodifyjobtest"
    LINK_LIBRARIES
        Qt5::Test
        Qt5::Sql
        ${XAPIAN_LIBRARIES}
        ${QJSON_LIBRARIES}
        baloocore
)
