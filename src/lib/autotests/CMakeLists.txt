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
ecm_add_test(advancedqueryparsertest.cpp ../advancedqueryparser.cpp
    TEST_NAME "advancedqueryparsertest"
    LINK_LIBRARIES Qt5::Test KF5::Baloo
)

#
# Fetch Job
#
set(filefetchjobtest_SRC filefetchjobtest.cpp
                         db.cpp
                         ../filemapping.cpp
                         ../file.cpp)

ecm_add_test(${filefetchjobtest_SRC}
    TEST_NAME "filefetchjobtest"
    LINK_LIBRARIES
        Qt5::Test
        KF5::Baloo
        KF5BalooEngine
)

# Pass CMAKE_CURRENT_BINARY_DIR so tests can use it to create some of the
# temporary files they need (as XattrDetector blacklists /tmp by default).
#set_target_properties(filefetchjobtest PROPERTIES COMPILE_FLAGS -DBUILDDIR="\\"${CMAKE_CURRENT_BINARY_DIR}/\\""\ -DBALOO_FILE_EXPORT=)

#
# TagList Job
#
ecm_add_test(taglistjobtest.cpp
    TEST_NAME "taglistjobtest"
    LINK_LIBRARIES
        Qt5::Test
        KF5::Baloo
)

#
# Search Store
#
#set(filesearchstore_SRCS filesearchstoretest.cpp
#                   ../filesearchstore.cpp
#                   ../pathfilterpostingsource.cpp
#                   ../wildcardpostingsource.cpp
#                   ../filemapping.cpp
#                   ../xapiansearchstore.cpp
#                   ../searchstore.cpp
#                   ../../file/database.cpp
#)
#
#ecm_add_test(${filesearchstore_SRCS}
#    TEST_NAME "filesearchstoretest"
#    LINK_LIBRARIES
#        Qt5::Core
#        Qt5::Test
#        ${XAPIAN_LIBRARIES}
#        KF5::FileMetaData
#        KF5::Baloo
#        KF5::BalooXapian
#)
#
#ecm_add_test(filemappingbenchmark.cpp
#    TEST_NAME "filemappingbenchmark"
#    LINK_LIBRARIES
#        Qt5::Test
#        KF5::Baloo
#        baloofilecommon
#)
