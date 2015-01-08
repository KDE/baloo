kde_enable_exceptions()

set(LUCENE_SRCS
    lucenedocument.cpp
    luceneindexwriter.cpp
)

add_library(KF5BalooLucene ${LUCENE_SRCS})

target_link_libraries(KF5BalooLucene PUBLIC
    Qt5::Core
    /usr/lib/liblucene++.so
)

install(TARGETS KF5BalooLucene EXPORT KF5BalooTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
