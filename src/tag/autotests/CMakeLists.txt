include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_unit_test(tagtest tagtests.cpp ../database.cpp)

target_link_libraries(tagtest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  viziercore
  viziertags
)
