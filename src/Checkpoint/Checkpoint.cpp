#include "Checkpoint.h"

#include <ConfigManager.h>
#include <filesystem>

namespace fs = std::filesystem;

Checkpoint::Checkpoint(int slave_id, FileSystem* fs) : slave_id_(slave_id),
    counter_(0), fs_(fs) {
}

void Checkpoint::clear() const {
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    for (int i = 1; i <= counter_; ++i) {
        auto ck_path = fs::path(ck_config.checkpoint_folder) / (
                           "checkpoint_" + std::to_string(slave_id_) + "_" +
                           std::to_string(i));
        fs_->remove(ck_path.string());
    }
}