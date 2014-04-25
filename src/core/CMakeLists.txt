kde_enable_exceptions()

set(CORE_SRCS
    searchstore.cpp
    term.cpp
    query.cpp
    queryrunnable.cpp
    resultiterator.cpp
    result.cpp
)

add_library(baloocore SHARED ${CORE_SRCS})

target_link_libraries(baloocore
    Qt5::Core
    KF5::Service
    ${XAPIAN_LIBRARIES}
)

set_target_properties(baloocore PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
)

generate_export_header(baloocore BASE_NAME BALOO_CORE EXPORT_FILE_NAME core_export.h)

install(TARGETS baloocore EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    term.h
    query.h
    queryrunnable.h
    result.h
    resultiterator.h
    searchstore.h

    ${CMAKE_CURRENT_BINARY_DIR}/core_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

install(FILES baloosearchstore.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

add_subdirectory(tests)
add_subdirectory(autotests)
