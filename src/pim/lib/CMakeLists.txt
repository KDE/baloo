include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

set(BALOO_PIM_LIB_SRCS
    query.cpp
    result.cpp
    resultiterator.cpp
    contactquery.cpp
    emailquery.cpp
    contactcompleter.cpp
)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions(-DQT_NO_KEYWORDS)

kde4_add_library(baloopim SHARED ${BALOO_PIM_LIB_SRCS})

target_link_libraries(baloopim
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${QJSON_LIBRARIES}
    ${XAPIAN_LIBRARIES}
)

set_target_properties(baloopim PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS baloopim EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    query.h
    contactquery.h
    emailquery.h
    contactcompleter.h

    result.h
    resultiterator.h
    pim_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo/pim COMPONENT Devel
)

add_subdirectory(tests)
