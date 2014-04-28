include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_unit_test(filesearchstoretest NOGUI filesearchstoretest.cpp
                   ../filesearchstore.cpp
                   ../pathfilterpostingsource.cpp
                   ../../lib/filemapping.cpp
                   ../../database.cpp
)

set_target_properties(filesearchstoretest PROPERTIES COMPILE_FLAGS -DBALOO_FILE_EXPORT=)

target_link_libraries(filesearchstoretest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${XAPIAN_LIBRARIES}
  ${KFILEMETADATA_LIBRARY}
  baloocore
  balooxapian
)
