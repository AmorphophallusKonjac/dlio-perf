#include <iostream>

#include "ConfigManager.h"
#include "LogManager.h"
#include "RunnerMaster.h"

int main() {
    LogManager::getInstance("master", ".");
    RunnerMaster master(9000, 6, 1);
    master.start();
    return 0;
}