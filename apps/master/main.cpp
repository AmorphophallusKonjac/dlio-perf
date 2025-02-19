#include <iostream>

#include "ConfigManager.h"
#include "LogManager.h"

int main() {
    LogManager::getInstance("log", ".");
    std::cout << ConfigManager::getInstance().dataset.sample_size << std::endl;
    ConfigManager::getInstance().fromYaml("../config/test.yaml");
    std::cout << ConfigManager::getInstance().dataset.sample_size << std::endl;
    std::cout << ConfigManager::getInstance().workflow.gen_sample << std::endl;
    std::cout << ConfigManager::getInstance().train.epochs << std::endl;
    std::cout << ConfigManager::getInstance().generate.transfer_size_stdev <<std::endl;
    for (auto i : ConfigManager::getInstance().dataset.sample_sub_dir) {
        std::cout << i << " ";
    }
    return 0;
}