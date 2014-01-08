include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_unit_test(filesearchstoretest filesearchstoretest.cpp
                   ../filesearchstore.cpp
                   ../pathfilterpostingsource.cpp
                   ../../lib/filemapping.cpp
                   ../../database.cpp
)

target_link_libraries(filesearchstoretest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${XAPIAN_LIBRARIES}
  baloocore
  balooxapian
)
