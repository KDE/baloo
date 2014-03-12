include(ECMAddTests)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(filesearchstore_SRCS filesearchstoretest.cpp
                   ../filesearchstore.cpp
                   ../pathfilterpostingsource.cpp
                   ../../lib/filemapping.cpp
                   ../../database.cpp
)

<<<<<<< HEAD
ecm_add_test(${filesearchstore_SRCS}
    TEST_NAME "filesearchstoretest"
    LINK_LIBRARIES
        Qt5::Core
        Qt5::Sql
        Qt5::Test
        ${XAPIAN_LIBRARIES}
        baloocore
        balooxapian
=======
target_link_libraries(filesearchstoretest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${XAPIAN_LIBRARIES}
  ${KFILEMETADATA_LIBRARY}
  baloocore
  balooxapian
>>>>>>> master
)
