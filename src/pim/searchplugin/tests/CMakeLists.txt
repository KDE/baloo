include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_executable(searchtest searchtest.cpp)

target_link_libraries(searchtest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}

  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}

  ${XAPIAN_LIBRARIES}
    baloocore
    baloopim
    balooxapian
)

install(TARGETS searchtest ${INSTALL_TARGETS_DEFAULT_ARGS})