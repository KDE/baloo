kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

set(BALOO_PIM_LIB_SRCS
    query.cpp
    resultiterator.cpp
    contactquery.cpp
    emailquery.cpp
    notequery.cpp
    contactcompleter.cpp

    ../search/email/agepostingsource.cpp
)

find_package(KF5KDELibs4Support REQUIRED)
find_package(KF5ItemModels REQUIRED)

add_library(baloopim SHARED ${BALOO_PIM_LIB_SRCS})

generate_export_header(baloopim BASE_NAME BALOO_PIM EXPORT_FILE_NAME pim_export.h)

target_link_libraries(baloopim
    PUBLIC
    Qt5::Core

    PRIVATE
    KF5::KDELibs4Support
    KF5::AkonadiCore
    KF5::ItemModels
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
    notequery.h

    resultiterator.h
    ${CMAKE_CURRENT_BINARY_DIR}/pim_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo/pim COMPONENT Devel
)

#REACTIVATE it
#add_subdirectory(tests)
