add_definitions(-DTRANSLATION_DOMAIN=\"baloosearch5\")

set(BALOO_SEARCH_SRCS main.cpp)

add_executable(baloosearch ${BALOO_SEARCH_SRCS})
target_compile_definitions(baloosearch PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloosearch
  KF5::I18n
  KF5::CoreAddons
  KF5::Baloo
)

install(TARGETS baloosearch DESTINATION ${BIN_INSTALL_DIR})
