add_definitions(-DTRANSLATION_DOMAIN=\"baloo_naturalqueryparser\")

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

add_library(KF5BalooNaturalQueryParser ${QUERYPARSER_SRCS})
add_library(KF5::BalooNaturalQueryParser ALIAS KF5BalooNaturalQueryParser)

target_link_libraries(KF5BalooNaturalQueryParser
    KF5::BalooCore
    KF5::KDELibs4Support
    KF5::FileMetaData
)

set_target_properties(KF5BalooNaturalQueryParser PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooNaturalQueryParser
)

generate_export_header(KF5BalooNaturalQueryParser BASE_NAME BALOO_NATURALQUERYPARSER EXPORT_FILE_NAME naturalqueryparser_export.h)
ecm_generate_headers(KF5BalooNaturalQueryParser_CamelCase_HEADERS
    HEADER_NAMES
    CompletionProposal
    NaturalQueryParser
    NaturalFileQueryParser

    PREFIX baloo
    REQUIRED_HEADERS KF5BalooNaturalQueryParser_HEADERS
)

install(TARGETS KF5BalooNaturalQueryParser EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/naturalqueryparser_export.h
    ${KF5BalooNaturalQueryParser_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/baloo
    COMPONENT Devel
)

install(FILES
    ${KF5BalooNaturalQueryParser_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/Baloo
    COMPONENT Devel
)

add_subdirectory(autotests)
