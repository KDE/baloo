include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_executable(indexerconfigtest
                    indexerconfigtest.cpp
                    ../fileindexerconfig.cpp
                    ../regexpcache.cpp
                    ../fileexcludefilters.cpp)

target_link_libraries(indexerconfigtest
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  baloocore
)
