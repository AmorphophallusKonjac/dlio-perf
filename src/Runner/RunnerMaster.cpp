#include "RunnerMaster.h"

#include <SlaveStatus.h>

RunnerMaster::RunnerMaster(unsigned short port, size_t size, int slave_num) :
    server_(std::make_unique<rest_rpc::rpc_service::rpc_server>(port, size)),
    slave_status_(std::make_unique<SlaveStatus>(slave_num)) {

}

void RunnerMaster::start() {
    startRpcServer();
    waitAllSlaveRegister();
}

void RunnerMaster::startRpcServer() {
    server_->register_handler("registerSlave", &SlaveStatus::registerSlave, &slave_status_);
    server_->async_run();
}

bool RunnerMaster::waitAllSlaveRegister() {
    while (!slave_status_->allSlaveRegistered()) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}






