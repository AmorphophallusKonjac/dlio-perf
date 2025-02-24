#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#include <atomic>
#include <queue>
#include <IORequest.h>
#include <mutex>
#include <condition_variable>

class RequestQueue
{
public:
    explicit RequestQueue(size_t max_size);
    void Push(const IORequest& request);
    IORequest Pop();
    void Stop();

private:
    std::queue<IORequest> queue_;
    const size_t max_size_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    std::atomic<bool> stop_flag_{false};
};


#endif //REQUESTQUEUE_H
