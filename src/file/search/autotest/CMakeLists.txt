include(ECMAddTests)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(filesearchstore_SRCS filesearchstoretest.cpp
                   ../filesearchstore.cpp
                   ../pathfilterpostingsource.cpp
                   ../wildcardpostingsource.cpp
                   ../../../lib/filemapping.cpp
                   ../../database.cpp
)

ecm_add_test(${filesearchstore_SRCS}
    TEST_NAME "filesearchstoretest"
    LINK_LIBRARIES
        Qt5::Core
        Qt5::Sql
        Qt5::Test
        ${XAPIAN_LIBRARIES}
        KF5::FileMetaData
        KF5::BalooCore
        KF5::BalooXapian
)
