add_executable(dlio_perf main.cpp)

target_include_directories(dlio_perf PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(dlio_perf PUBLIC dlio-perf_lib)