add_subdirectory(master)

add_subdirectory(slave)

add_executable(test test.cpp)

target_link_libraries(test PUBLIC yaml-cpp::yaml-cpp)
