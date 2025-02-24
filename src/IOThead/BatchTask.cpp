#include "BatchTask.h"

#include <thread>

BatchTask::BatchTask(const long long batch_size, const int prefetch_size,
                     const int thread_num, const long long transfer_size,
                     FileSystem* fs) :
    batch_size_(batch_size), thread_num_(thread_num),
    transfer_size_(transfer_size), fs_(fs), request_queue_(thread_num * 2),
    thread_finish_(1 - batch_size), batch_gen_(0), batch_use_(prefetch_size) {
}

void BatchTask::mainTask() {
    batch_use_.signal(1);
    batch_gen_.wait(1);
    process();
}

void BatchTask::startIOCtrlThread(const std::vector<IORequest>& requests) {
    io_ctrl_thread_ = std::thread(&BatchTask::ioCtrlThread, this, requests);
}

void BatchTask::stopIOCtrlThread() {
    if (io_ctrl_thread_.joinable())
        io_ctrl_thread_.join();
}

void BatchTask::ioCtrlThread(const std::vector<IORequest>& requests) {
    std::vector<std::thread> io_threads;
    io_threads.reserve(thread_num_);
    for (int i = 0; i < thread_num_; ++i) {
        io_threads.emplace_back(&BatchTask::ioThread, this);
    }
    for (int i = 0; i < requests.size(); i = i + batch_size_) {
        batch_use_.wait(1);
        for (int j = 0; j < batch_size_; ++j) {
            request_queue_.Push(requests[i + j]);
        }
        thread_finish_.wait(batch_size_);
        batch_gen_.signal(1);
    }
    request_queue_.Stop();
    for (auto& thread : io_threads) {
        if (thread.joinable())
            thread.join();
    }
}


void BatchTask::ioThread() {
    auto buffer = new char[transfer_size_];
    for (auto request = request_queue_.Pop(); !request.empty();
         request = request_queue_.Pop()) {
        request.execute(fs_, transfer_size_, buffer);
        thread_finish_.signal(1);
    }
    delete[] buffer;
}

void BatchTask::process() {
}