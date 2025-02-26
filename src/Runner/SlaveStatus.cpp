#include "SlaveStatus.h"


void SlaveStatus::StatusSet::insert(std::string slave) {
    std::lock_guard lock(mutex_);
    set_.insert(slave);
}

std::size_t SlaveStatus::StatusSet::size() {
    std::lock_guard lock(mutex_);
    return set_.size();
}

bool SlaveStatus::StatusSet::contain(std::string slave) {
    std::lock_guard lock(mutex_);
    return set_.find(slave) != set_.end();
}

SlaveStatus::SlaveStatus(const int slave_num) :
    slave_num_(slave_num) {
}

void SlaveStatus::slaveRegister(rest_rpc::rpc_service::rpc_conn conn,
                                std::string slave) {
    unready_slaves_.insert(slave);
}

bool SlaveStatus::slaveReady(rest_rpc::rpc_service::rpc_conn conn,
                             std::string slave) {
    if (!ready_slaves_.contain(slave)) {
        ready_slaves_.insert(slave);
    }
    return ready_slaves_.size() == slave_num_;
}

void SlaveStatus::slaveFinish(rest_rpc::rpc_service::rpc_conn conn,
                              std::string slave, bool success) {
    if (success) {
        success_slaves_.insert(slave);
    } else {
        fail_slaves_.insert(slave);
    }
}

bool SlaveStatus::allSlaveRegistered() {
    return slave_num_ == unready_slaves_.size();
}

bool SlaveStatus::allSlaveFinish() {
    return slave_num_ == success_slaves_.size() + fail_slaves_.size();
}