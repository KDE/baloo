include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(indexer_SRCS
    ../emailindexer.cpp
    ../contactindexer.cpp
    ../akonotesindexer.cpp
    ../calendarindexer.cpp
    ../abstractindexer.cpp
    ../../search/pimsearchstore.cpp
    ../../search/email/emailsearchstore.cpp
    ../../search/email/agepostingsource.cpp
    ../../search/contact/contactsearchstore.cpp
)

set(indexer_LIBS
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${XAPIAN_LIBRARIES}
    ${KDEPIMLIBS_KPIMUTILS_LIBS}
    baloocore
    balooxapian
)

set(indexertest_SRCS
    indexertest.cpp
    ${indexer_SRCS}
)

add_definitions(-DBALOO_NO_PLUGINS=TRUE)

kde4_add_unit_test(indexertest NOGUI ${indexertest_SRCS})
target_link_libraries(indexertest
    ${indexer_LIBS}
)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(AkonadiMacros)
set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

set(scheduler_SRCS
    ../scheduler.cpp
    ../index.cpp
    ../collectionindexingjob.cpp
    ${indexer_SRCS}
)

add_akonadi_isolated_test_advanced(schedulertest.cpp "${scheduler_SRCS}" "${indexer_LIBS}")
add_akonadi_isolated_test_advanced(collectionindexingjobtest.cpp "${scheduler_SRCS}" "${indexer_LIBS}")
