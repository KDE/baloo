add_executable(writeiotest writeiotest.cpp)
target_link_libraries(writeiotest
    KF5::BalooEngine
)
ecm_mark_as_test(writeiotest)

add_executable(memorytest memorytest.cpp)
target_link_libraries(memorytest
    KF5::BalooEngine
)
ecm_mark_as_test(memorytest)

add_executable(documenturldb documenturldbtest.cpp)
target_link_libraries(documenturldb
    Qt5::Core
    KF5::BalooEngine
)
ecm_mark_as_test(documenturldb)

add_executable(basicindexingtest basicindexingtest.cpp)
target_link_libraries(basicindexingtest
    Qt5::Core
    KF5::BalooEngine
    KF5::FileMetaData
    baloofilecommon
)
ecm_mark_as_test(basicindexingtest)
