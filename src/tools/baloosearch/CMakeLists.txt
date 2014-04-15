set(BALOO_SEARCH_SRCS main.cpp)

add_executable(baloosearch ${BALOO_SEARCH_SRCS})

target_link_libraries(baloosearch
  KF5::I18n
  KF5::CoreAddons
  baloocore
)

install(TARGETS baloosearch DESTINATION ${BIN_INSTALL_DIR})
