# Benchmarks, compiled, but not run automatically with ctest
add_executable(databasebenchmark databasebenchmark.cpp)
target_link_libraries(databasebenchmark Qt${QT_MAJOR_VERSION}::Test KF5::BalooEngine)

add_executable(positioncodecbenchmark positioncodecbenchmark.cpp)
target_link_libraries(positioncodecbenchmark Qt${QT_MAJOR_VERSION}::Test KF5::BalooCodecs)
