if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  ecm_add_test(kinotifytest.cpp
    TEST_NAME "kinotifytest"
    LINK_LIBRARIES Qt6::Test baloofilecommon
  )
endif()

MACRO(BALOO_FILE_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp TEST_NAME ${_testname} LINK_LIBRARIES Qt6::Test baloofilecommon KF6::Baloo)
  ENDFOREACH()
ENDMACRO()

baloo_file_auto_tests(
    pendingfilequeuetest
    fileindexerconfigtest
    basicindexingjobtest
    filtereddiriteratortest
    unindexedfileiteratortest
    metadatamovertest
    extractorcommandpipetest
)


#
# File Watch
#
set(fileWatch_SRC filewatchtest.cpp)
ecm_add_test(${fileWatch_SRC}
    TEST_NAME "filewatchtest"
    LINK_LIBRARIES Qt6::Test Qt6::DBus KF6::Baloo KF6::FileMetaData baloofilecommon
)

#
# Property Serialization
#
ecm_add_test(propertyserializationtest.cpp
    TEST_NAME "propertyserializationtest"
    LINK_LIBRARIES Qt6::Test KF6::FileMetaData baloofilecommon
)

# Management of external extractor process
ecm_add_test(
    extractorprocesstest.cpp
    TEST_NAME "extractorprocesstest"
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test baloofilecommon
)
# Dummy process for extractor process testing
add_executable(extractorprocess_fake
	extractorprocess_fake.cpp
)
target_link_libraries(extractorprocess_fake
	baloofilecommon
	KF6::Crash
)

configure_file(testsconfig.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testsconfig.h @ONLY)

# Dummy process for extractor pipe testing
add_executable(extractorcommandpipe_worker
	extractorcommandpipe_worker.cpp
)
target_link_libraries(extractorcommandpipe_worker
	baloofilecommon
)

# Indexing tracked and untracked files on modification
ecm_add_test(
    modifiedfileindexertest.cpp
    TEST_NAME "modifiedfileindexertest"
    LINK_LIBRARIES Qt6::Test baloofilecommon
)

# Processing of document content
add_executable(documentcontenttest_bin
    documentcontenttest.cpp
    ../../../src/file/extractor/result.cpp
)
target_link_libraries(documentcontenttest_bin
    Qt6::Test
    baloofilecommon
)
ecm_mark_as_test(documentcontenttest_bin)

# Run the benchmarks with just 1 iteration during CI, so we known it works
add_test(NAME documentcontenttest COMMAND documentcontenttest_bin "-iterations" "1")
