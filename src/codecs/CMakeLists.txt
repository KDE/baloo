set(BALOO_CODECS_SRCS
    doctermscodec.cpp
    positioncodec.cpp
    postingcodec.cpp

    coding.cpp
)

ecm_qt_declare_logging_category(BALOO_CODECS_SRCS
    HEADER codecsdebug.h
    IDENTIFIER BALOO_CODECS
    CATEGORY_NAME kf.baloo.codecs
    DEFAULT_SEVERITY Warning
    DESCRIPTION "Baloo Codecs"
    EXPORT BALOO
)

add_library(KF6BalooCodecs STATIC ${BALOO_CODECS_SRCS})
add_library(KF6::BalooCodecs ALIAS KF6BalooCodecs)

set_property(TARGET KF6BalooCodecs PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(KF6BalooCodecs
    Qt6::Core
    KF6::CoreAddons
)
