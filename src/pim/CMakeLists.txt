include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${KDEPIMLIBS_INCLUDE_DIRS}
)

set(PIM_SRCS
  database.cpp
  emailindexer.cpp
)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

kde4_add_library(vizierpim SHARED ${PIM_SRCS})

target_link_libraries(vizierpim
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${XAPIAN_LIBRARIES}
    viziercore
    akonadi-kmime
)

set_target_properties(vizierpim PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS vizierpim ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(autotests)
add_subdirectory(tests)
