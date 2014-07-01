kde_enable_exceptions()

set(XAPIAN_SRCS
    xapiansearchstore.cpp
    xapiandocument.cpp
    xapiandatabase.cpp
    termgenerator.cpp
)

add_library(KF5BalooXapian ${XAPIAN_SRCS})
add_library(KF5::BalooXapian ALIAS KF5BalooXapian)

target_link_libraries(KF5BalooXapian PUBLIC
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5::BalooCore
)

set_target_properties(KF5BalooXapian PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooXapian
)

generate_export_header(KF5BalooXapian BASE_NAME BALOO_XAPIAN EXPORT_FILE_NAME xapian_export.h)

install(TARGETS KF5BalooXapian EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(autotests)
