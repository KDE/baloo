add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(QUERYPARSER_SRCS
    completionproposal.cpp
    pass_comparators.cpp
    pass_dateperiods.cpp
    pass_datevalues.cpp
    pass_decimalvalues.cpp
    pass_filenames.cpp
    pass_filesize.cpp
    pass_numbers.cpp
    pass_periodnames.cpp
    pass_properties.cpp
    pass_splitunits.cpp
    pass_subqueries.cpp
    pass_typehints.cpp
    patternmatcher.cpp
    queryparser.cpp
    utils.cpp
)

add_library(balooqueryparser SHARED ${QUERYPARSER_SRCS})

target_link_libraries(balooqueryparser
    KF5::BalooCore
)

set_target_properties(balooqueryparser PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_VERSION_MAJOR}
)

install(TARGETS balooqueryparser EXPORT BalooLibraryTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    completionproposal.h
    queryparser.h
    queryparser_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(autotests)
