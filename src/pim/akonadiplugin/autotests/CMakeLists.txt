include(ECMMarkAsTest)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(searchplugintest_SRCS
    searchplugintest.cpp
    ../searchplugin.cpp
    ../../agent/emailindexer.cpp
    ../../agent/contactindexer.cpp
    ../../agent/abstractindexer.cpp
    ../../agent/akonotesindexer.cpp
    ../../search/pimsearchstore.cpp
    ../../search/email/emailsearchstore.cpp
    ../../search/email/agepostingsource.cpp
    ../../search/contact/contactsearchstore.cpp
    ../../search/note/notesearchstore.cpp
)

add_definitions(-DBALOO_NO_PLUGINS=TRUE)

add_executable( searchplugintest ${searchplugintest_SRCS} )
add_test( searchplugintest searchplugintest )
ecm_mark_as_test(searchplugintest)

target_link_libraries(searchplugintest
    Qt5::Test
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Abc
    ${XAPIAN_LIBRARIES}
    KF5::PimUtils
    KF5::BalooCore
    KF5::BalooXapian
)
