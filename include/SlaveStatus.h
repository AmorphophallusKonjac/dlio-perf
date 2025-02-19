#ifndef SLAVESTATUS_H
#define SLAVESTATUS_H

#include <mutex>
#include <string>
#include <unordered_set>
#include <rest_rpc.hpp>

class SlaveStatus {
    class StatusSet {
    public:
        void insert(std::string slave);
        std::size_t size();
    private:
        std::unordered_set<std::string> set_;
        std::mutex mutex_;
    };
public:
    explicit SlaveStatus(int slave_num);

    void registerSlave(rest_rpc::rpc_service::rpc_conn conn, std::string slave);

    bool allSlaveRegistered();
private:
    int slave_num_;
    StatusSet ready_slaves_;
    StatusSet success_slaves_;
};



#endif //SLAVESTATUS_H
