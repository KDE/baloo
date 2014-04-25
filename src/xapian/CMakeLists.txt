kde_enable_exceptions()

set(XAPIAN_SRCS
    xapiansearchstore.cpp
    xapiandocument.cpp
    xapiandatabase.cpp
)

add_library(balooxapian SHARED ${XAPIAN_SRCS})

target_link_libraries(balooxapian
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    baloocore
)

set_target_properties(balooxapian PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
)

generate_export_header(balooxapian BASE_NAME BALOO_XAPIAN EXPORT_FILE_NAME xapian_export.h)

install(TARGETS balooxapian EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
