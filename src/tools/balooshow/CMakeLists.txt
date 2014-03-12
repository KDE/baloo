set(SRCS main.cpp)

add_executable(balooshow ${SRCS})

target_link_libraries(balooshow
    baloocore
    baloofiles

    KF5::FileMetaData
)

install(TARGETS balooshow DESTINATION ${BIN_INSTALL_DIR})
