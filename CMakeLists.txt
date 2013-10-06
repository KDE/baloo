project(vizier)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
)

set( SRCS
  main.cpp
)

kde4_add_executable(vtest ${SRCS})

target_link_libraries(vtest
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
)

install(TARGETS vtest
        DESTINATION ${BIN_INSTALL_DIR})
