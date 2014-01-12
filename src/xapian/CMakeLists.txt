add_definitions(-fexceptions)

set(XAPIAN_SRCS
    xapiansearchstore.cpp
)

add_library(balooxapian SHARED ${XAPIAN_SRCS})

target_link_libraries(balooxapian
    Qt5::Core
    ${QJSON_LIBRARIES}
    KF5::KDE4Support
    ${XAPIAN_LIBRARIES}
    baloocore
)

set_target_properties(balooxapian PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS balooxapian EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
