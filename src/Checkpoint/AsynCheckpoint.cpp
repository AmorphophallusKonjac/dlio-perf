#include "AsynCheckpoint.h"

#include <filesystem>
#include <ConfigManager.h>

namespace fs = std::filesystem;

AsynCheckpoint::AsynCheckpoint(int slave_id, FileSystem* fs) : Checkpoint(
        slave_id, fs), stop_flag_(false) {
    // save_thread_
    save_thread_ = std::thread(&AsynCheckpoint::saveThread, this);
}

void AsynCheckpoint::generate() {
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    fs_->createDir(ck_config.checkpoint_folder);
    const auto ck_path = fs::path(ck_config.checkpoint_folder) / (
                             "checkpoint_" + std::to_string(slave_id_) + "_0");
    const auto ck_file = fs_->getFileDescriptor();
    ck_file->open(ck_path, File::WRITE);
    ck_file->writeWholeFile(ck_config.checkpoint_size,
                            ck_config.write_transfer_size);
    ck_file->close();
}

void AsynCheckpoint::load() {
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    const auto ck_path = fs::path(ck_config.checkpoint_folder) / (
                             "checkpoint_" + std::to_string(slave_id_) + "_" +
                             std::to_string(counter_));
    std::vector<std::thread> read_threads;
    std::vector<IORequest> requests[ck_config.read_threads];
    for (long long pos = 0, idx = 0; pos < ck_config.checkpoint_size;
         pos += ck_config.read_transfer_size) {
        long long size = ck_config.read_transfer_size;
        if (pos + size >= ck_config.checkpoint_size)
            size = ck_config.checkpoint_size - pos;
        requests[idx++].emplace_back(IORequest::READ, ck_path.string(), size,
                                     pos);

        idx %= ck_config.read_threads;
    }
    read_threads.reserve(ck_config.read_threads);
    for (int i = 0; i < ck_config.read_threads; ++i) {
        read_threads.emplace_back(&AsynCheckpoint::loadThread, this,
                                  requests[i]);
    }
    for (auto& thread : read_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void AsynCheckpoint::save() {
    sem_.signal(1);
}

AsynCheckpoint::~AsynCheckpoint() {
    stop_flag_ = true;
    if (save_thread_.joinable())
        save_thread_.join();
}

void AsynCheckpoint::loadThread(std::vector<IORequest> requests) const {
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    const auto buffer = new char[ck_config.read_transfer_size];
    for (const auto& req : requests) {
        req.execute(fs_, ck_config.read_transfer_size, buffer);
    }
    delete[] buffer;
}

void AsynCheckpoint::saveThread() {
    while (true) {
        sem_.wait(1);
        if (stop_flag_)
            break;
        const auto ck_config = ConfigManager::getInstance().checkpoint;
        const auto ck_path = fs::path(ck_config.checkpoint_folder) / (
                                 "checkpoint_" + std::to_string(slave_id_) + "_"
                                 +
                                 std::to_string(++counter_));
        const auto ck_file = fs_->getFileDescriptor();
        ck_file->open(ck_path, File::WRITE);
        auto size = ck_config.checkpoint_size;
        auto layers = ck_config.checkpoint_layers;
        long long pos = 0;
        for (int i = 0; i < layers; ++i) {
            auto layer_size = size / layers;
            if (i == layers - 1) {
                layer_size += size % layers;
            }
            for (long long j = 0; j < layer_size;
                 j += ck_config.write_transfer_size) {
                auto buffer_size = ck_config.write_transfer_size;
                if (j + buffer_size >= layer_size) {
                    buffer_size = layer_size - j;
                }
                const auto buffer = new char[buffer_size];
                ck_file->write(buffer, pos, buffer_size);
                pos += buffer_size;
                delete[] buffer;
            }
        }
        ck_file->close();
    }
}