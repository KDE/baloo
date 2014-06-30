set(extractor_test_SRCS extractortest.cpp)
kde4_add_unit_test(extractortest NOGUI ${extractor_test_SRCS})
target_link_libraries(extractortest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
)
