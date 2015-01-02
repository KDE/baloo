set(LUCENE_SRCS
    lucenedocument.cpp
    luceneindexwriter.cpp)

add_library(KF5BalooLucene ${LUCENE_SRCS})

target_link_libraries(KF5BalooLucene PUBLIC
    Qt5::Core
    #add libraries for lucene
)

#todo read cmake docs and comlete this