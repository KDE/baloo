#
# Query Serialization
#
set(querySerialization_SRC queryserializationtest.cpp)
set(queryParser_SRC queryparsertest.cpp)

kde4_add_unit_test(queryserializationtest NOGUI ${querySerialization_SRC})
kde4_add_unit_test(queryparsertest NOGUI ${queryParser_SRC})

target_link_libraries(queryserializationtest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  baloocore
)
target_link_libraries(queryparsertest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  baloocore
)
