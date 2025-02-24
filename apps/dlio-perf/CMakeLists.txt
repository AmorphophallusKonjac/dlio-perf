add_executable(dlio-perf main.cpp)

target_link_libraries(dlio-perf PUBLIC dlio-perf_lib)
target_include_directories(dlio-perf PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_include_directories(dlio-perf PUBLIC ${PROJECT_SOURCE_DIR}/third_party/CLI11)