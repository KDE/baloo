set(EXTRACTOR_SRCS
  main.cpp
  priority.cpp
)

kde4_add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})

target_link_libraries(baloo_file_extractor
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KFILEMETADATA_LIBRARIES}
  )

install(TARGETS baloo_file_extractor DESTINATION ${BIN_INSTALL_DIR})
