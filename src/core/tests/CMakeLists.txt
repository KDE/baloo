include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_executable(querytest querytest.cpp)

target_link_libraries(querytest
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  baloocore
)
