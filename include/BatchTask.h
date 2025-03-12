#ifndef TASKBASE_H
#define TASKBASE_H

#include <RequestQueue.h>
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
    void ioThread();
    virtual void process();
    long long batch_size_;
    int thread_num_;
    long long transfer_size_;
    RequestQueue request_queue_;
    Semaphore thread_finish_;
    Semaphore batch_gen_;
    Semaphore batch_use_;
    std::thread io_ctrl_thread_;
    std::string task_name_;
};


#endif //TASKBASE_H
