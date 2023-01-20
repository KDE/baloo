add_executable(writeiotest writeiotest.cpp)
target_link_libraries(writeiotest
    KF6::BalooEngine
)
ecm_mark_as_test(writeiotest)

add_executable(memorytest memorytest.cpp)
target_link_libraries(memorytest
    KF6::BalooEngine
)
ecm_mark_as_test(memorytest)

add_executable(documenturldb documenturldbtest.cpp)
target_link_libraries(documenturldb
    Qt${QT_MAJOR_VERSION}::Core
    KF6::BalooEngine
)
ecm_mark_as_test(documenturldb)

add_executable(basicindexingtest basicindexingtest.cpp)
target_link_libraries(basicindexingtest
    Qt${QT_MAJOR_VERSION}::Core
    KF6::BalooEngine
    KF6::FileMetaData
    baloofilecommon
)
ecm_mark_as_test(basicindexingtest)
