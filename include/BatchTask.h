#ifndef TASKBASE_H
#define TASKBASE_H

#include <IORequest.h>
#include <Semaphore.h>
#include <FileSystem.h>
#include <thread>

class BatchTask
{
public:
    BatchTask(long long batch_size, int prefetch_size, int thread_num, long long transfer_size, std::string task_name);
    void mainTask();
    void startIOCtrlThread(const std::vector<IORequest>& requests);
    void stopIOCtrlThread();
    virtual ~BatchTask() = default;

private:
    void ioCtrlThread(const std::vector<IORequest>& requests);
    void ioThread(int idx);
    virtual void process();
    long long batch_size_;
    int thread_num_;
    long long transfer_size_;
    Semaphore batch_gen_;
    std::thread io_ctrl_thread_;
    std::string task_name_;
    std::vector<std::thread> io_threads_;
    std::vector<int> thread_file_num_;
    std::vector<std::unique_ptr<Semaphore>> thread_sems_;
    std::vector<std::vector<IORequest>> thread_io_requests_;
};


#endif //TASKBASE_H
