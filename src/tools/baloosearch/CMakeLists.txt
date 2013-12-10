set(SRCS main.cpp)

kde4_add_executable(baloosearch ${SRCS})

target_link_libraries(baloosearch
  ${KDE4_KDECORE_LIBS}
  baloocore
)

install(TARGETS baloosearch DESTINATION ${BIN_INSTALL_DIR})
