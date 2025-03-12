#ifndef ASYNCHECKPOINT_H
#define ASYNCHECKPOINT_H

#include <Checkpoint.h>
#include <IORequest.h>
#include <Semaphore.h>
#include <thread>

class AsynCheckpoint : public Checkpoint
{
public:
    explicit AsynCheckpoint(int slave_id, FileSystem* fs);
    void generate() override;
    void load() override;
    void save() override;
    ~AsynCheckpoint();

private:
    void loadThread(IORequest request) const;
    void saveThread();

    std::thread save_thread_;
    Semaphore sem_;
    bool stop_flag_;
};


#endif //ASYNCHECKPOINT_H
