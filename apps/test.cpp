#include <iostream>
#include <string>

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>

#include <glog/logging.h>

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    std::cout << "hello world" << std::endl;
    YAML::Node config = YAML::LoadFile("../config/test.yaml");
    // const std::string name = config["name"].as<std::string>();
    const int age = config["age"].as<int>();
    // std::cout << name << std::endl;
    std::cout << age << std::endl;
    return 0;
}
