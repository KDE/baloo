add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(XAPIAN_SRCS
    xapiansearchstore.cpp
)

kde4_add_library(balooxapian SHARED ${XAPIAN_SRCS})

target_link_libraries(balooxapian
    ${QT_QTCORE_LIBRARY}
    ${QJSON_LIBRARIES}
    ${KDE4_KDECORE_LIBRARY}
    ${XAPIAN_LIBRARIES}
    baloocore
)

set_target_properties(balooxapian PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS balooxapian EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
