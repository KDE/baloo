set(BALOO_ENGINE_SRCS
    postingiterator.cpp
    andpostingiterator.cpp
    vectorpostingiterator.cpp
)

add_library(KF5BalooEngine ${BALOO_ENGINE_SRCS})
add_library(KF5::BalooEngine ALIAS KF5BalooEngine)

target_link_libraries(KF5BalooEngine
    PUBLIC
    Qt5::Core
    KF5::CoreAddons
)

set_target_properties(KF5BalooEngine PROPERTIES
    VERSION ${BALOO_VERSION_STRING}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooEngine
)
##target_include_directories(KF5BalooEngine INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Baloo>")

generate_export_header(KF5BalooEngine BASE_NAME BALOO_ENGINE EXPORT_FILE_NAME engine_export.h)

install(TARGETS KF5BalooEngine EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(autotests)
