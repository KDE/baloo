add_definitions(-DTRANSLATION_DOMAIN=\"baloo_queryparser\")

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
    pass_propertyinfo.cpp
    pass_splitunits.cpp
    pass_subqueries.cpp
    pass_typehints.cpp
    patternmatcher.cpp
    naturalqueryparser.cpp
    naturalfilequeryparser.cpp
    utils.cpp
)

add_library(baloonaturalqueryparser ${QUERYPARSER_SRCS})

target_link_libraries(baloonaturalqueryparser
    KF5::BalooCore
    KF5::KDELibs4Support
    KF5::FileMetaData
)

set_target_properties(baloonaturalqueryparser PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_VERSION_MAJOR}
)

install(TARGETS baloonaturalqueryparser EXPORT BalooLibraryTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    completionproposal.h
    naturalqueryparser.h
    naturalqueryparser_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(autotests)
