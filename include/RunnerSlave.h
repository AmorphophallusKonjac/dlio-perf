#ifndef RUNNERSLAVE_H
#define RUNNERSLAVE_H

#include <BatchTask.h>
#include <string>
#include <FileSystemFactory.h>
#include <random>
#include <CheckpointFactory.h>
#include <chrono>

class RunnerSlave
{
public:
    RunnerSlave(int slave_id, int slave_num);
    void start();

private:
    std::vector<std::string> getShuffleFileList();
    bool run();
    void readSamples();
    void loadCheckpoint();
    void saveCheckpoint();
    void waitCheckpoint();
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
    FileSystemFactory fs_factory_;
    CheckpointFactory ck_factory_;
    std::mt19937 rand_engine_;
    std::vector<std::string> trainFileList_;
    std::chrono::steady_clock::time_point start_time_point_;
    std::chrono::steady_clock::time_point end_time_point_;
    BatchTask* read_batch_task_ = nullptr;
};


#endif //RUNNERSLAVE_H
