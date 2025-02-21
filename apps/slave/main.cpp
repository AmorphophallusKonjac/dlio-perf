#include <RunnerSlave.h>
#include <LogManager.h>

int main() {
    LogManager::getInstance("slave", ".");
    RunnerSlave slave("127.0.0.1", 9000, "slave1");
    slave.start();
    return 0;
}