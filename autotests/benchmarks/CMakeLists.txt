# Benchmarks, compiled, but not run automatically with ctest
add_executable(databasebenchmark databasebenchmark.cpp)
target_link_libraries(databasebenchmark Qt5::Test KF5::BalooEngine)

add_executable(positioncodecbenchmark positioncodecbenchmark.cpp)
target_link_libraries(positioncodecbenchmark Qt5::Test KF5::BalooCodecs)
