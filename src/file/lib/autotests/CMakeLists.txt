#
# Fetch Job
#
set(filefetchjobtest_SRC filefetchjobtest.cpp
                         db.cpp
                         ../filefetchjob.cpp
                         ../filemapping.cpp
                         ../file.cpp
                         ../xattrdetector.cpp
                         ../filecustommetadata.cpp)

ecm_add_test(${filefetchjobtest_SRC}
    TEST_NAME "filefetchjobtest"
    LINK_LIBRARIES
        KF5::Solid
        KF5::KDE4Support
        Qt5::Test
        Qt5::Sql
        ${XAPIAN_LIBRARIES}
        baloocore
)

#
# Modify Job
#
set(filemodifyjobtest_SRC filemodifyjobtest.cpp
                          db.cpp
                          ../filemodifyjob.cpp
                          ../filemapping.cpp
                          ../file.cpp
                          ../xattrdetector.cpp
                          ../filecustommetadata.cpp)

ecm_add_test(${filemodifyjobtest_SRC}
    TEST_NAME "filemodifyjobtest"
    LINK_LIBRARIES
        KF5::Solid
        KF5::KDE4Support
        Qt5::Test
        Qt5::Sql
        ${XAPIAN_LIBRARIES}
        baloocore
)

# Pass CMAKE_CURRENT_BINARY_DIR so tests can use it to create some of the
# temporary files they need (as XattrDetector blacklists /tmp by default).
set_target_properties(filefetchjobtest PROPERTIES COMPILE_FLAGS -DBUILDDIR="\\"${CMAKE_CURRENT_BINARY_DIR}/\\"")
set_target_properties(filemodifyjobtest PROPERTIES COMPILE_FLAGS -DBUILDDIR="\\"${CMAKE_CURRENT_BINARY_DIR}/\\"")
