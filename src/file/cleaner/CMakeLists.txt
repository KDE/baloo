set(CLEANER_SRCS
  main.cpp
  cleaner.cpp
)

add_executable(baloo_file_cleaner ${CLEANER_SRCS})

target_link_libraries(baloo_file_cleaner baloofilecommon)
install(TARGETS baloo_file_cleaner DESTINATION ${BIN_INSTALL_DIR})
