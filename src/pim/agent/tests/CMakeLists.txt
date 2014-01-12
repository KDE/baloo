include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(emailindexer emailtest.cpp ../emailindexer.cpp ../abstractindexer.cpp)

target_link_libraries(emailindexer
  Qt5::Core
  Qt5::Test
  

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  ${XAPIAN_LIBRARIES}
)
