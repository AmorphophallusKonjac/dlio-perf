#ifndef RUNNERSLAVE_H
#define RUNNERSLAVE_H

#include <memory>
#include <string>
#include <rest_rpc.hpp>

class RunnerSlave
{
public:
    RunnerSlave(std::string host, unsigned short port, std::string name);
    void start();

private:
    void initialize();
    void registerSelf();
    bool reportReady();
    void waitStart();
    std::unique_ptr<rest_rpc::rpc_client> rpc_client_;
    std::string name;
};


#endif //RUNNERSLAVE_H
