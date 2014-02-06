set(CLEANER_SRCS
  main.cpp
  cleaner.cpp
  ../priority.cpp
  ../database.cpp
  ../commitqueue.cpp
  ../fileindexerconfig.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
)

kde4_add_executable(baloo_file_cleaner ${CLEANER_SRCS})

target_link_libraries(baloo_file_cleaner
  ${QT_QTSQL_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${XAPIAN_LIBRARIES}
)

install(TARGETS baloo_file_cleaner DESTINATION ${BIN_INSTALL_DIR})
