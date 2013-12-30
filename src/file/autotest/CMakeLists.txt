project(filewatch_tests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(kinotifytest_SRC kinotifytest.cpp ../kinotify.cpp)
  kde4_add_unit_test(kinotifytest TESTNAME nepomuk-kinotifytest NOGUI ${kinotifytest_SRC})
  target_link_libraries(kinotifytest nepomukcore
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
  )
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

set(activefilequeuetest_SRC activefilequeuetest.cpp ../activefilequeue.cpp)
kde4_add_unit_test(activefilequeuetest TESTNAME nepomuk-activefilequeuetest NOGUI ${activefilequeuetest_SRC})
target_link_libraries(activefilequeuetest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
)

#
# Metadata Mover
#
set(metadatamover_SRC
    metadatamovertest.cpp
    ../metadatamover.cpp
    ../database.cpp
    ../lib/filemapping.cpp)

kde4_add_unit_test(metadatamovertest NOGUI ${metadatamover_SRC})

target_link_libraries(metadatamovertest
  ${QT_QTTEST_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${XAPIAN_LIBRARIES}
)

#
# File Monitor
#
set(fileMonitor_SRC filemonitortest.cpp)

kde4_add_unit_test(filemonitortest NOGUI ${fileMonitor_SRC})

target_link_libraries(filemonitortest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  baloofiles
)

#
# Query Serialization
#
set(querySerialization_SRC queryserializationtest.cpp)

kde4_add_unit_test(queryserializationtest NOGUI ${querySerialization_SRC})

target_link_libraries(queryserializationtest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  baloocore
)
