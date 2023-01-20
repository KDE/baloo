add_definitions(-DTRANSLATION_DOMAIN=\"balooshow5\")

set(SRCS main.cpp)

add_executable(balooshow ${SRCS})
ecm_mark_nongui_executable(balooshow)
target_compile_definitions(balooshow PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(balooshow
    KF6::Baloo
    KF6::BalooEngine
    KF6::FileMetaData
    KF6::CoreAddons
    KF6::I18n
)

install(TARGETS balooshow DESTINATION ${KDE_INSTALL_BINDIR})
