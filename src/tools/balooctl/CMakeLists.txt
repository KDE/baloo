set(SRCS main.cpp)

add_executable(balooctl ${SRCS})

target_link_libraries(balooctl
    Qt5::DBus
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    ${XAPIAN_LIBRARIES}
    baloofiles
    balooxapian
)

install(TARGETS balooctl DESTINATION ${BIN_INSTALL_DIR})
