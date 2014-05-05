set(SRCS main.cpp)

kde4_add_executable(balooctl ${SRCS})

target_link_libraries(balooctl
    ${KDE4_KDECORE_LIBS}
    ${XAPIAN_LIBRARIES}
    baloofiles
    balooxapian
)

install(TARGETS balooctl DESTINATION ${BIN_INSTALL_DIR})
