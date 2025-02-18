add_executable(master main.cpp)

target_include_directories(master PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(master PUBLIC dlio-perf_lib)
