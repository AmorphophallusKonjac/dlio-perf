add_executable(test test.cpp)

target_link_libraries(test PUBLIC yaml-cpp::yaml-cpp glog::glog gflags::gflags)