#include "RunnerSlave.h"

#include <rest_rpc.hpp>
#include <g3log/g3log.hpp>
#include <ConfigManager.h>
#include <FileSystemFactory.h>

RunnerSlave::RunnerSlave(std::string host, unsigned short port,
                         std::string name) :
    rpc_client_(std::make_unique<rest_rpc::rpc_client>(host, port)),
    name(name) {
}

void RunnerSlave::start() {
    try {
        registerSelf();
        FileSystemFactory fsFactory;
        initialize();
        waitStart();
        // benchmark start
    } catch (const std::exception& e) {
        LOGF(FATAL, "%s", e.what());
    }
}

void RunnerSlave::initialize() {
    if (ConfigManager::getInstance().workflow.gen_data) {
    }
}

void RunnerSlave::registerSelf() {
    if (!rpc_client_->connect(10)) {
        LOGF(FATAL, "Slave connect timeout");
        throw std::runtime_error("Fail to connect server");
    }
    rpc_client_->call<void>("register", name);
    rpc_client_->close();
}

bool RunnerSlave::reportReady() {
    if (!rpc_client_->connect(10)) {
        LOGF(FATAL, "Slave connect timeout");
        throw std::runtime_error("Fail to connect server");
    }
    const auto ret = rpc_client_->call<bool>("reportReady", name);
    rpc_client_->close();
    return ret;
}

void RunnerSlave::waitStart() {
    while (!reportReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}