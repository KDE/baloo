include_directories(
    ${KDEPIMLIBS_INCLUDE_DIRS}
    ${XAPIAN_INCLUDE_DIR}
)

set(BALOO_PIM_LIB_SRCS
    query.cpp
    resultiterator.cpp
    contactquery.cpp
    emailquery.cpp
    contactcompleter.cpp

    ../search/email/agepostingsource.cpp
)

add_definitions(-fexceptions)

add_library(baloopim SHARED ${BALOO_PIM_LIB_SRCS})

target_link_libraries(baloopim
    Qt5::Core
    KF5::KDE4Support
    ${QJSON_LIBRARIES}
    ${XAPIAN_LIBRARIES}
)

set_target_properties(baloopim PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
)

install(TARGETS baloopim EXPORT BalooLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    query.h
    contactquery.h
    emailquery.h
    contactcompleter.h

    resultiterator.h
    pim_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo/pim COMPONENT Devel
)

add_subdirectory(tests)
