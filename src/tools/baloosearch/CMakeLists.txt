set(SRCS main.cpp)

add_executable(baloosearch ${SRCS})

target_link_libraries(baloosearch
  
  baloocore
)

install(TARGETS baloosearch DESTINATION ${BIN_INSTALL_DIR})
