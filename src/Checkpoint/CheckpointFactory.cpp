#include "CheckpointFactory.h"

#include <ConfigManager.h>
#include <SyncCheckpoint.h>

#include <stdexcept>

Checkpoint*
CheckpointFactory::getCheckpoint(const int slave_id, FileSystem* fs) {
    if (ck_)
        return ck_;
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    switch (ck_config.checkpoint_type) {
        case ConfigManager::CheckpointConfig::SYNC:
            ck_ = new SyncCheckpoint(slave_id, fs);
            break;
        default:
            throw std::runtime_error("Unknown checkpoint type");
    }
    return ck_;
}

CheckpointFactory::~CheckpointFactory() {
    if (ck_)
        delete ck_;
}