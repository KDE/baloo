set(LUCENE_SEARCH_SRCS main.cpp)

add_executable(lucenesearch ${LUCENE_SEARCH_SRCS})

target_link_libraries(lucenesearch
  Qt5::Core
  ${LucenePlusPlus_LIBRARIES}
  KF5::BalooLucene
)

install(TARGETS lucenesearch DESTINATION ${BIN_INSTALL_DIR})
