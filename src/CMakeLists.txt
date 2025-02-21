file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(dlio-perf_lib STATIC
        ${SOURCE_FILES}
)

target_include_directories(dlio-perf_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_include_directories(dlio-perf_lib PUBLIC
        "${rest_rpc_SOURCE_DIR}/thirdparty/msgpack-c/include"
        "${rest_rpc_SOURCE_DIR}/thirdparty/asio"
        "${rest_rpc_SOURCE_DIR}/include"
)

target_link_libraries(dlio-perf_lib PUBLIC yaml-cpp::yaml-cpp)
target_link_libraries(dlio-perf_lib PUBLIC g3log)
