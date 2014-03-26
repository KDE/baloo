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

kde4_add_executable(basicindexingqueuetest
                    basicindexingqueuetest.cpp
                    ../fileindexerconfig.cpp
                    ../regexpcache.cpp
                    ../fileexcludefilters.cpp
                    ../basicindexingqueue.cpp
                    ../indexingqueue.cpp
                    ../basicindexingjob.cpp
                    ../database.cpp
                    ../lib/filemapping.cpp
                    ../commitqueue.cpp
)

target_link_libraries(basicindexingqueuetest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBS}
  ${XAPIAN_LIBRARIES}
  ${KFILEMETADATA_LIBRARY}
  baloocore
  balooxapian
)
