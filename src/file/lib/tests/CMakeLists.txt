include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_executable(xattrdetectortest
                    xattrdetectortest.cpp
                    ../xattrdetector.cpp
)

target_link_libraries(xattrdetectortest
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_SOLID_LIBS}
)
