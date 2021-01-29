add_definitions(-DTRANSLATION_DOMAIN=\"baloosearch5\")

set(BALOO_SEARCH_SRCS main.cpp)

add_executable(baloosearch ${BALOO_SEARCH_SRCS})
ecm_mark_nongui_executable(baloosearch)
target_compile_definitions(baloosearch PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloosearch
  KF5::I18n
  KF5::CoreAddons
  KF5::Baloo
)

install(TARGETS baloosearch DESTINATION ${KDE_INSTALL_BINDIR})
