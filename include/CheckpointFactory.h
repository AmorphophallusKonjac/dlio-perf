#ifndef CHECKPOINTFACTORY_H
#define CHECKPOINTFACTORY_H

#include <Checkpoint.h>

class CheckpointFactory
{
public:
    Checkpoint* getCheckpoint(int slave_id, FileSystem* fs);
    ~CheckpointFactory();

private:
    Checkpoint* ck_ = nullptr;
};


#endif //CHECKPOINTFACTORY_H
