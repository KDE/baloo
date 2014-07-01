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

add_library(KF5BalooPim SHARED ${BALOO_PIM_LIB_SRCS})


add_library(KF5::BalooPim ALIAS KF5BalooPim)

generate_export_header(KF5BalooPim BASE_NAME BALOO_PIM EXPORT_FILE_NAME pim_export.h)

target_link_libraries(KF5BalooPim
    PUBLIC
    Qt5::Core

    PRIVATE
    KF5::KDELibs4Support
    KF5::AkonadiCore
    KF5::ItemModels
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5BalooPim PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooPim
)

install(TARGETS KF5BalooPim EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})


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
