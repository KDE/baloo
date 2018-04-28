if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  ecm_add_test(kinotifytest.cpp
    TEST_NAME "kinotifytest"
    LINK_LIBRARIES Qt5::Test baloofilecommon
  )
endif()

set (metadatamovertest_SOURCES
    metadatamovertest.cpp
)

qt5_add_dbus_adaptor(metadatamovertest_SOURCES
    ${CMAKE_SOURCE_DIR}/src/file/org.kde.BalooWatcherApplication.xml
    metadatamovertest.h MetadataMoverTestDBusSpy)

ecm_add_test(${metadatamovertest_SOURCES} TEST_NAME metadatamovertest LINK_LIBRARIES Qt5::Test baloofilecommon KF5::Baloo)

MACRO(BALOO_FILE_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp TEST_NAME ${_testname} LINK_LIBRARIES Qt5::Test baloofilecommon KF5::Baloo)
  ENDFOREACH()
ENDMACRO()

baloo_file_auto_tests(
    pendingfilequeuetest
    fileindexerconfigtest
    basicindexingjobtest
    regularexpcachebenchmark
    filtereddiriteratortest
    unindexedfileiteratortest
    fileinfotest
)


#
# File Watch
#
set(fileWatch_SRC filewatchtest.cpp ../lib/xattrdetector.cpp)
ecm_add_test(${fileWatch_SRC}
    TEST_NAME "filewatchtest"
    LINK_LIBRARIES Qt5::Test Qt5::DBus KF5::Baloo baloofilecommon
)

