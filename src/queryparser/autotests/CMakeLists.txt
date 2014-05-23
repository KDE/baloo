#
# Query Parser
#
set(queryParser_SRC queryparsertest.cpp)

kde4_add_unit_test(queryparsertest NOGUI ${queryParser_SRC})

target_link_libraries(queryparsertest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  baloocore
  balooqueryparser
)
