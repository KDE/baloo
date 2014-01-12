include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(contactsearch contactsearchtest.cpp)

target_link_libraries(contactsearch
  Qt5::Core
  

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  baloopim
)

add_executable(emailquery emailquerytest.cpp)

target_link_libraries(emailquery
  Qt5::Core
  

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  baloocore
  baloopim
)

add_executable(contactcompleter contactcompletiontest.cpp)

target_link_libraries(contactcompleter
  Qt5::Core
  
  baloopim
)
