file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(dlio-perf_lib STATIC
        ${SOURCE_FILES}
)

target_include_directories(dlio-perf_lib PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/third_party/include"
)

target_link_libraries(dlio-perf_lib PUBLIC yaml-cpp::yaml-cpp)

target_link_libraries(dlio-perf_lib PRIVATE Threads::Threads)

target_include_directories(dlio-perf_lib PUBLIC ${MPI_CXX_INCLUDE_DIRS})

target_link_libraries(dlio-perf_lib PUBLIC ${MPI_CXX_LIBRARIES})
