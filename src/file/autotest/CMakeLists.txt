include(ECMAddTests)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  ecm_add_test(kinotifytest.cpp ../kinotify.cpp
    TEST_NAME "kinotifytest"
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons
  )
endif()

ecm_add_test(activefilequeuetest.cpp ../activefilequeue.cpp
    TEST_NAME "activefilequeuetest"
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
    LINK_LIBRARIES Qt5::Test Qt5::DBus KF5::BalooFiles
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
# Basic Indexing Queue Test
#

set(basicIndexingQueue_SRC
    basicindexingqueuetest.cpp
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
set(regexpcache_SRCS
    regularexpcachebenchmark.cpp
)

ecm_add_test(${regexpcache_SRCS}
    TEST_NAME "regexpcachebenchmark"
    LINK_LIBRARIES Qt5::Test KF5::ConfigCore KF5::CoreAddons baloofilecommon
)

add_subdirectory(fileindexingjob)
add_subdirectory(scheduler)
