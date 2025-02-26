#include "RunnerMaster.h"

#include <ConfigManager.h>
#include <SlaveStatus.h>
#include <g3log/g3log.hpp>
#include <random>

RunnerMaster::RunnerMaster(unsigned short port, size_t size, int slave_num) :
    server_(std::make_unique<rest_rpc::rpc_service::rpc_server>(port, size)),
    slave_status_(std::make_unique<SlaveStatus>(slave_num)) {
}

void RunnerMaster::start() {
    startRpcServer();
    LOGF(INFO, "Wait all slaves to register...");
    if (!waitAllSlaveRegister()) {
        throw std::runtime_error("Slave start failed");
    }
    LOGF(INFO, "All slaves registered...");
    LOGF(INFO, "Wait all slaves to finished");
    waitAllSlaveFinish();
    LOGF(INFO, "All slaves finished");
}

void RunnerMaster::startRpcServer() {
    server_->register_handler("register", &SlaveStatus::slaveRegister,
                              slave_status_.get());
    server_->register_handler("reportReady", &SlaveStatus::slaveReady,
                              slave_status_.get());
    server_->register_handler("getRandSeed", &ConfigManager::getRandSeed,
                              &ConfigManager::getInstance());
    server_->register_handler("reportFinish", &SlaveStatus::slaveFinish,
                              slave_status_.get());
    server_->async_run();
}

bool RunnerMaster::waitAllSlaveRegister() const {
    const auto limits = std::chrono::system_clock::now() +
                        std::chrono::seconds(100);
    while (std::chrono::system_clock::now() < limits) {
        if (!slave_status_->allSlaveRegistered()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            return true;
        }
    }
    LOGF(FATAL, "Some slave register timeout");
    return false;
}

void RunnerMaster::waitAllSlaveFinish() const {
    while (!slave_status_->allSlaveFinish()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}