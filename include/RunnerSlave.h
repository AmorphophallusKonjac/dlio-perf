#ifndef RUNNERSLAVE_H
#define RUNNERSLAVE_H

#include <memory>
#include <string>
#include <rest_rpc.hpp>
#include <FileSystemFactory.h>
#include <random>
#include <CheckpointFactory.h>
#include <yaml-cpp/yaml.h>

class RunnerSlave
{
public:
    RunnerSlave(const std::string& host, unsigned short port, std::string name, int slave_id, int slave_num);
    void start();

private:
    std::vector<std::string> getShuffleFileList();
    bool run();
    void readSamples();
    void loadCheckpoint();
    void saveCheckpoint();
    void generate();
    void getTrainFileList();
    void initialize();
    void registerSelf();
    bool reportReady();
    void reportFinish(bool success);
    void waitStart();
    uint32_t getRandSeed() const;
    void finalize();

    int slave_id_;
    int slave_num_;
    std::unique_ptr<rest_rpc::rpc_client> rpc_client_;
    std::string name_;
    FileSystemFactory fs_factory_;
    CheckpointFactory ck_factory_;
    std::mt19937 rand_engine_;
    std::vector<std::string> trainFileList_;
    YAML::Node report;
};


#endif //RUNNERSLAVE_H
