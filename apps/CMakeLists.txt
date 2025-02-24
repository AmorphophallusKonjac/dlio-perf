add_executable(test_yaml_config test_yaml_config.cpp)

target_link_libraries(test_yaml_config PUBLIC yaml-cpp::yaml-cpp g3log)

include_directories(
        "${rest_rpc_SOURCE_DIR}/thirdparty/msgpack-c/include"
        "${rest_rpc_SOURCE_DIR}/thirdparty/asio"
)

add_executable(server server.cpp)

target_include_directories(server PUBLIC ${rest_rpc_SOURCE_DIR}/include)

add_executable(client client.cpp)

target_include_directories(client PUBLIC ${rest_rpc_SOURCE_DIR}/include)

add_subdirectory(master)

add_subdirectory(slave)

add_executable(test_mpi mpi.cpp)
target_link_libraries(test_mpi PUBLIC ${MPI_CXX_LIBRARIES})
target_include_directories(test_mpi PUBLIC ${MPI_CXX_INCLUDE_DIRS})

add_subdirectory(dlio-perf)


