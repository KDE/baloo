add_definitions(-fexceptions)

set(XAPIAN_SRCS
    xapiansearchstore.cpp
    xapiandocument.cpp
    xapiandatabase.cpp
)

add_library(balooxapian SHARED ${XAPIAN_SRCS})

target_link_libraries(balooxapian
    Qt5::Core
    KF5::KDE4Support
    ${XAPIAN_LIBRARIES}
    baloocore
)

set_target_properties(balooxapian PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
)

install(TARGETS balooxapian EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
