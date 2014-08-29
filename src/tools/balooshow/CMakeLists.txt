add_definitions(-DTRANSLATION_DOMAIN=\"balooshow\")

set(SRCS main.cpp)

add_executable(balooshow ${SRCS})
target_compile_definitions(balooshow PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(balooshow
    KF5::BalooCore
    KF5::BalooFiles
    KF5::BalooXapian
    KF5::FileMetaData
    KF5::CoreAddons
    KF5::I18n
)

install(TARGETS balooshow DESTINATION ${BIN_INSTALL_DIR})
