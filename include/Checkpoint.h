#ifndef CHECKPOINT_H
#define CHECKPOINT_H
#include <FileSystem.h>

class Checkpoint
{
public:
    Checkpoint(int slave_id, FileSystem* fs);
    virtual void generate() = 0;
    virtual void load() = 0;
    virtual void save() = 0;
    virtual void finalize() = 0;
    virtual ~Checkpoint() = default;

protected:
    int slave_id_;
    int counter_;
    FileSystem* fs_;
};


#endif //CHECKPOINT_H
