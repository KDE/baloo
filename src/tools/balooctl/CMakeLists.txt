add_definitions(-DTRANSLATION_DOMAIN=\"balooctl\")

set(SRCS main.cpp filestatistics.cpp)

add_executable(balooctl ${SRCS})

target_link_libraries(balooctl
    Qt5::DBus
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    ${XAPIAN_LIBRARIES}
    KF5::BalooFiles
    KF5::BalooXapian
)

install(TARGETS balooctl DESTINATION ${BIN_INSTALL_DIR})
