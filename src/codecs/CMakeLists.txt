set(BALOO_CODECS_SRCS
    doctermscodec.cpp
    positioncodec.cpp
    postingcodec.cpp

    coding.cpp
)

add_library(KF5BalooCodecs STATIC ${BALOO_CODECS_SRCS})
add_library(KF5::BalooCodecs ALIAS KF5BalooCodecs)

target_link_libraries(KF5BalooCodecs
    Qt${QT_MAJOR_VERSION}::Core
    KF5::CoreAddons
)
