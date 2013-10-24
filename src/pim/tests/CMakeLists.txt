include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_executable(emailindexer emailtest.cpp)

target_link_libraries(emailindexer
  ${QT_QTCORE_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  baloocore
  baloopim
)

kde4_add_executable(emailquery emailquerytest.cpp)

target_link_libraries(emailquery
  ${QT_QTCORE_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${XAPIAN_LIBRARIES}

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  baloocore
  baloopim
)
