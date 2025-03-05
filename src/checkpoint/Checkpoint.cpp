#include "Checkpoint.h"

Checkpoint::Checkpoint(int slave_id, FileSystem* fs) : slave_id_(slave_id),
    counter_(0), fs_(fs) {
}