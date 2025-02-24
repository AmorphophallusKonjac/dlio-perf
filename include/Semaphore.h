#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore
{
public:
    explicit Semaphore(long long count = 0) : count_(count)
    {
    }

    void signal(const long long delta)
    {
        std::lock_guard lock(mutex_);

        count_ += delta;
        cv_.notify_one();
    }

    void wait(const long long delta)
    {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return count_ > 0; });

        count_ -= delta;
    }

private:
    long long count_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif //SEMAPHORE_H
