set(SRCS main.cpp)

kde4_add_executable(balooshow ${SRCS})

target_link_libraries(balooshow
    ${KDE4_KDECORE_LIBS}
    baloocore
    baloofiles
)

install(TARGETS balooshow DESTINATION ${BIN_INSTALL_DIR})
