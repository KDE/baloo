include(ECMAddTests)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  ecm_add_test(kinotifytest.cpp ../kinotify.cpp
    TEST_NAME "kinotifytest"
    LINK_LIBRARIES Qt5::Test KF5::KDE4Support
  )
endif()

ecm_add_test(activefilequeuetest.cpp ../activefilequeue.cpp
    TEST_NAME "activefilequeuetest"
    LINK_LIBRARIES Qt5::Test KF5::KDE4Support
)

#
# Metadata Mover
#

set(metadatamover_SRC
    metadatamovertest.cpp
    ../metadatamover.cpp
    ../database.cpp
    ../lib/filemapping.cpp)

ecm_add_test(${metadatamover_SRC}
    TEST_NAME "metadatamovertest"
    LINK_LIBRARIES Qt5::Test Qt5::Sql KF5::KDE4Support ${XAPIAN_LIBRARIES}
)

#
# File Monitor
#
set(fileMonitor_SRC filemonitortest.cpp)

ecm_add_test(${fileMonitor_SRC}
    TEST_NAME "filemonitortest"
    LINK_LIBRARIES Qt5::Test baloofiles KF5::KDE4Support
)

#
# File Indexing Config
#
set(indexingConfig_SRC
    fileindexerconfigtest.cpp
    ../fileindexerconfig.cpp
    ../regexpcache.cpp
    ../fileexcludefilters.cpp
)

ecm_add_test(${indexingConfig_SRC}
    TEST_NAME "fileindexerconfigtest"
    LINK_LIBRARIES Qt5::Test KF5::KDE4Support
)
