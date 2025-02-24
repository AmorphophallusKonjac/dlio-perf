#ifndef RUNNERSLAVE_H
#define RUNNERSLAVE_H

#include <memory>
#include <string>
#include <rest_rpc.hpp>
#include <FileSystemFactory.h>

class RunnerSlave
{
public:
    RunnerSlave(const std::string& host, unsigned short port, std::string name, int slave_id, int slave_num);
    void start();

private:
    void generate();
    void initialize();
    void registerSelf();
    bool reportReady();
    void waitStart();

    int slave_id_;
    int slave_num_;
    std::unique_ptr<rest_rpc::rpc_client> rpc_client_;
    std::string name_;
    FileSystemFactory fs_factory_;
};


#endif //RUNNERSLAVE_H
