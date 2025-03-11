#ifndef SYNCCHECKPOINT_H
#define SYNCCHECKPOINT_H

#include <Checkpoint.h>
#include <IORequest.h>

class SyncCheckpoint final : public Checkpoint
{
public:
    explicit SyncCheckpoint(int slave_id, FileSystem* fs);
    void load() override;
    void save() override;
    void generate() override;

private:
    void loadThread(std::vector<IORequest> requests) const;
};


#endif //SYNCCHECKPOINT_H
