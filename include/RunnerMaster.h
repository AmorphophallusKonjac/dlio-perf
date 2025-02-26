#ifndef RUNNERMASTER_H
#define RUNNERMASTER_H

#include <memory>
#include <SlaveStatus.h>

class RunnerMaster
{
public:
    explicit RunnerMaster(unsigned short port, size_t size, int slave_num);
    void start();

private:
    void startRpcServer();
    bool waitAllSlaveRegister() const;
    void waitAllSlaveFinish() const;
    std::unique_ptr<rest_rpc::rpc_service::rpc_server> server_;
    std::unique_ptr<SlaveStatus> slave_status_;
};


#endif //RUNNERMASTER_H
