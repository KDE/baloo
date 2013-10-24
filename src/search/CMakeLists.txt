include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

set(SEARCH_SRCS
    result.cpp
    resultiterator.cpp
    contactquery.cpp
)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions(-DQT_NO_KEYWORDS)

kde4_add_library(baloosearch SHARED ${SEARCH_SRCS})

target_link_libraries(baloosearch
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
    baloocore
)

set_target_properties(baloosearch PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS baloosearch ${INSTALL_TARGETS_DEFAULT_ARGS})
