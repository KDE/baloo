kde_enable_exceptions()

set(LUCENE_SRCS
    lucenedocument.cpp
    luceneindex.cpp
)

add_library(KF5BalooLucene ${LUCENE_SRCS})
add_library(KF5::BalooLucene ALIAS KF5BalooLucene)

target_link_libraries(KF5BalooLucene PUBLIC
    Qt5::Core
    ${LucenePlusPlus_LIBRARIES}
    ${Boost_LIBRARIES}
)

set_target_properties(KF5BalooLucene PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooLucene
)

generate_export_header(KF5BalooLucene BASE_NAME BALOO_LUCENE EXPORT_FILE_NAME lucene_export.h)

install(TARGETS KF5BalooLucene EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
