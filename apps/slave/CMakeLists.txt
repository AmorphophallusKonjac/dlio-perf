add_executable(slave main.cpp)

target_include_directories(slave PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(slave PUBLIC dlio-perf_lib)

target_include_directories(slave PUBLIC ${PROJECT_SOURCE_DIR}/third_party/CLI11)
