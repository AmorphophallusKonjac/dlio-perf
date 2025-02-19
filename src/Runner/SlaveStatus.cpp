#include "SlaveStatus.h"


void SlaveStatus::StatusSet::insert(std::string slave) {
    std::lock_guard lock(mutex_);
    set_.insert(slave);
}

std::size_t SlaveStatus::StatusSet::size() {
    std::lock_guard lock(mutex_);
    return set_.size();
}

SlaveStatus::SlaveStatus(const int slave_num) :
    slave_num_(slave_num) {

}

void SlaveStatus::registerSlave(rest_rpc::rpc_service::rpc_conn conn, std::string slave) {
    ready_slaves_.insert(slave);
}

bool SlaveStatus::allSlaveRegistered() {
    return slave_num_ == ready_slaves_.size();
}


