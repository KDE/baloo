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
