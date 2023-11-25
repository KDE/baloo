# Benchmarks, compiled, but not run automatically with ctest

add_executable(positioncodecbenchmark positioncodecbenchmark.cpp)
target_link_libraries(positioncodecbenchmark Qt6::Test KF6::BalooCodecs)

add_executable(termgeneratorbenchmark termgeneratorbenchmark.cpp)
target_link_libraries(termgeneratorbenchmark Qt6::Test KF6::BalooCodecs KF6::BalooEngine)
