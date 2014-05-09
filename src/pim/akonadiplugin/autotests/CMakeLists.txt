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

kde4_add_unit_test(searchplugintest NOGUI ${searchplugintest_SRCS})
target_link_libraries(searchplugintest
    ${QT_QTTEST_LIBRARY}
    
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Abc
    ${XAPIAN_LIBRARIES}
    KF5::PimUtils
    baloocore
    balooxapian
)
