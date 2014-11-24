include(ECMAddTests)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  ecm_add_test(kinotifytest.cpp
    TEST_NAME "kinotifytest"
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons baloofilecommon
  )
endif()

ecm_add_test(pendingfilequeuetest.cpp ../pendingfilequeue.cpp ../pendingfile.cpp
    TEST_NAME "pendingfilequeuetest"
    LINK_LIBRARIES Qt5::Test
)

#
# Metadata Mover
#

set(metadatamover_SRC
    metadatamovertest.cpp
    ../metadatamover.cpp
)

ecm_add_test(${metadatamover_SRC}
    TEST_NAME "metadatamovertest"
    LINK_LIBRARIES Qt5::Test Qt5::Sql ${XAPIAN_LIBRARIES} KF5::BalooXapian baloofilecommon
)

#
# File Monitor
#
set(fileMonitor_SRC filemonitortest.cpp)

ecm_add_test(${fileMonitor_SRC}
    TEST_NAME "filemonitortest"
    LINK_LIBRARIES Qt5::Test Qt5::DBus KF5::Baloo
)

#
# File Watch
#
set(fileWatch_SRC filewatchtest.cpp ../../lib/autotests/xattrdetector.cpp)

ecm_add_test(${fileWatch_SRC}
    TEST_NAME "filewatchtest"
    LINK_LIBRARIES Qt5::Test Qt5::DBus KF5::Baloo baloofilecommon
)

#
# File Indexing Config
#
set(indexingConfig_SRC
    fileindexerconfigtest.cpp
)

ecm_add_test(${indexingConfig_SRC}
    TEST_NAME "fileindexerconfigtest"
    LINK_LIBRARIES Qt5::Test KF5::ConfigCore KF5::CoreAddons baloofilecommon
)

#
# Basic Indexing Job Test
#
set(basicIndexingJob_SRC
    basicindexingjobtest.cpp
)

ecm_add_test(${basicIndexingJob_SRC}
    TEST_NAME "basicfileindexingjobtest"
    LINK_LIBRARIES Qt5::Test ${XAPIAN_LIBRARIES}
    KF5::BalooXapian baloofilecommon
)

#
# Basic Indexing Queue Test
#

set(basicIndexingQueue_SRC
    basicindexingqueuetest.cpp
    ../../lib/autotests/xattrdetector.cpp
)

ecm_add_test(${basicIndexingQueue_SRC}
    TEST_NAME basicindexingqueuetest
    LINK_LIBRARIES Qt5::Test Qt5::Sql KF5::FileMetaData KF5::ConfigCore
                   KF5::CoreAddons ${XAPIAN_LIBRARIES} KF5::BalooXapian
                   baloofilecommon
)

#
# Regexp cahce benchmark
#
ecm_add_test(regularexpcachebenchmark.cpp
    TEST_NAME "regexpcachebenchmark"
    LINK_LIBRARIES Qt5::Test KF5::ConfigCore KF5::CoreAddons baloofilecommon
)

#
# Filtered Dir Iterator
#
ecm_add_test(filtereddiriteratortest.cpp
    TEST_NAME "filtereddiriteratortest"
    LINK_LIBRARIES Qt5::Test KF5::ConfigCore KF5::CoreAddons baloofilecommon
)

add_subdirectory(fileindexingjob)
add_subdirectory(scheduler)
