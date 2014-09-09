add_executable(writeiotest writeiotest.cpp)
target_link_libraries(writeiotest
    Qt5::Core
    KF5::BalooXapian
)

add_executable(memorytest memorytest.cpp)
target_link_libraries(memorytest
    Qt5::Core
    KF5::BalooXapian
)
