include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_executable(contactsearch contactsearchtest.cpp)

target_link_libraries(contactsearch
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  baloosearch
)

kde4_add_executable(emailquery emailquerytest.cpp)

target_link_libraries(emailquery
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  baloocore
  baloosearch
)

kde4_add_executable(contactcompleter contactcompletiontest.cpp)

target_link_libraries(contactcompleter
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  baloosearch
)
