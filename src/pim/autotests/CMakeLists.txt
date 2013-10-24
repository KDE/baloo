include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_unit_test(databasetest databasetests.cpp ../database.cpp)

target_link_libraries(databasetest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${XAPIAN_LIBRARIES}
  baloocore
)
