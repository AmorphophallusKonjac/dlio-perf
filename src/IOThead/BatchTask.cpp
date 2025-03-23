#include "BatchTask.h"

#include <spdlog/spdlog.h>

#include <thread>
#include <utility>

BatchTask::BatchTask(const long long batch_size, const int prefetch_size,
                     const int thread_num, const long long transfer_size,
                     std::string task_name) :
    batch_size_(batch_size), thread_num_(thread_num),
    transfer_size_(transfer_size), batch_gen_(1 - thread_num_),
    task_name_(std::move(task_name)) {
    for (int i = 0; i < thread_num_; ++i) {
        thread_file_num_.push_back(0);
        thread_sems_.emplace_back(std::make_unique<Semaphore>());
        thread_io_requests_.emplace_back();
    }
}

void BatchTask::mainTask() {
    for (int i = 0; i < thread_num_; ++i)
        thread_sems_[i]->signal(1);
    batch_gen_.wait(thread_num_);
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
    for (int i = 0; i < batch_size_; ++i)
        ++thread_file_num_[i % thread_num_];
    for (int i = 0; i < requests.size(); ++i) {
        thread_io_requests_[i % thread_num_].push_back(requests[i]);
    }
    std::vector<std::thread> io_threads;
    io_threads.reserve(thread_num_);
    for (int i = 0; i < thread_num_; ++i) {
        io_threads.emplace_back(&BatchTask::ioThread, this, i);
    }
    for (auto& thread : io_threads) {
        if (thread.joinable())
            thread.join();
    }
}


void BatchTask::ioThread(int idx) {
    for (int i = 0; i < thread_io_requests_[idx].size();
         i = i + thread_file_num_[idx]) {
        thread_sems_[idx]->wait(1);
        for (int j = 0; j < thread_file_num_[idx]; ++j) {
            thread_io_requests_[idx][i + j].execute();
        }
        batch_gen_.signal(1);
    }
}

void BatchTask::process() {
}