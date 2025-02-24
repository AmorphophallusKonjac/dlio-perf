#include "RequestQueue.h"

RequestQueue::RequestQueue(size_t max_size) : max_size_(max_size) {
}

void RequestQueue::Push(const IORequest& request) {
    std::unique_lock lock(mutex_);
    not_full_.wait(lock, [this]() { return queue_.size() < max_size_; });

    queue_.push(request);

    not_empty_.notify_one();
}

IORequest RequestQueue::Pop() {
    std::unique_lock lock(mutex_);
    not_empty_.wait(lock, [this] { return !queue_.empty() || stop_flag_; });

    if (queue_.empty())
        return {};
    auto ret = queue_.front();
    queue_.pop();

    not_full_.notify_one();
    return ret;
}

void RequestQueue::Stop() {
    std::lock_guard lock(mutex_);
    stop_flag_ = true;
    not_empty_.notify_all();
}

