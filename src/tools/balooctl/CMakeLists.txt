set(SRCS main.cpp)

kde4_add_executable(balooctl ${SRCS})

target_link_libraries(balooctl
    ${KDE4_KDECORE_LIBS}
    baloofiles
)

install(TARGETS balooctl DESTINATION ${BIN_INSTALL_DIR})
