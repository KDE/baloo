add_definitions(-DTRANSLATION_DOMAIN=\"baloodb5\")

set(SRCS main.cpp)

add_executable(baloodb ${SRCS})
ecm_mark_nongui_executable(baloodb)
target_compile_definitions(baloodb PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloodb
    KF5::Baloo
    KF5::BalooEngine
    KF5::CoreAddons
    KF5::I18n
)

install(TARGETS baloodb ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
