#ifndef SLAVESTATUS_H
#define SLAVESTATUS_H

#include <mutex>
#include <string>
#include <unordered_set>
#include <rest_rpc.hpp>

class SlaveStatus
{
    class StatusSet
    {
    public:
        void insert(std::string slave);
        std::size_t size();
        bool contain(std::string slave);

    private:
        std::unordered_set<std::string> set_;
        std::mutex mutex_;
    };

public:
    explicit SlaveStatus(int slave_num);

    void slaveRegister(rest_rpc::rpc_service::rpc_conn conn, std::string slave);

    bool slaveReady(rest_rpc::rpc_service::rpc_conn conn, std::string slave);

    bool allSlaveRegistered();

    bool allSlaveReady();

private:
    int slave_num_;
    StatusSet unready_slaves_;
    StatusSet ready_slaves_;
    StatusSet success_slaves_;
};


#endif //SLAVESTATUS_H
