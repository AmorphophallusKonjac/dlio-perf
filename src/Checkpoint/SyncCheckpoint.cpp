#include "SyncCheckpoint.h"

#include <ConfigManager.h>
#include <filesystem>
#include <thread>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

void SyncCheckpoint::load() {
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    auto ck_path = fs::path(ck_config.checkpoint_folder) / (
                       "checkpoint_" + std::to_string(slave_id_) + "_" +
                       std::to_string(counter_));
    if (counter_ == 0) {
        ck_path = fs::path(ck_config.checkpoint_folder) / "checkpoint_base";
    }
    const auto size = ck_config.checkpoint_size;
    const auto layers = ck_config.checkpoint_layers;
    long long pos = 0;
    std::vector<IORequest> requests;
    for (int i = 0; i < ck_config.read_threads; ++i) {
        requests.emplace_back(IORequest::READ, ck_path,
                              ck_config.read_transfer_size, fs_);
        auto layers_per_thread = layers / ck_config.read_transfer_size;
        if (i == ck_config.read_threads - 1) {
            layers_per_thread += layers % ck_config.read_transfer_size;
        }
        for (int j = 0; j < layers_per_thread; ++j) {
            auto layer_size = size / layers;
            if (i == ck_config.read_threads - 1 && j == layers_per_thread - 1) {
                layer_size += size % layers;
            }
            requests[i].addIOReq(pos, layer_size);
            pos += layer_size;
        }
    }
    std::vector<std::thread> read_threads;
    read_threads.reserve(ck_config.read_threads);
    for (int i = 0; i < ck_config.read_threads; ++i) {
        read_threads.emplace_back(&SyncCheckpoint::loadThread, this,
                                  requests[i]);
    }
    for (auto& thread : read_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

SyncCheckpoint::SyncCheckpoint(const int slave_id, FileSystem* fs) : Checkpoint(
    slave_id, fs) {
}


void SyncCheckpoint::save() {
    spdlog::info("Rank {} start save checkpoint {}", slave_id_, counter_ + 1);
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    const auto ck_path = fs::path(ck_config.checkpoint_folder) / (
                             "checkpoint_" + std::to_string(slave_id_) + "_" +
                             std::to_string(++counter_));
    const auto size = ck_config.checkpoint_size;
    const auto layers = ck_config.checkpoint_layers;
    IORequest request(IORequest::WRITE, ck_path.string(),
                      ck_config.write_transfer_size, fs_);
    long long pos = 0;
    for (int i = 0; i < layers; ++i) {
        auto layer_size = size / layers;
        if (i == layers - 1) {
            layer_size += size % layers;
        }
        request.addIOReq(pos, layer_size);
        pos += layer_size;
    }
    request.execute();
}


void SyncCheckpoint::generate() {
    const auto ck_config = ConfigManager::getInstance().checkpoint;
    fs_->createDir(ck_config.checkpoint_folder);
    const auto ck_path = fs::path(ck_config.checkpoint_folder) /
                         "checkpoint_base";
    const auto ck_file = fs_->getFileDescriptor();
    ck_file->open(ck_path, File::WRITE);
    ck_file->writeWholeFile(ck_config.checkpoint_size,
                            ck_config.write_transfer_size);
    ck_file->close();
}

void SyncCheckpoint::finalize() {
}

void SyncCheckpoint::loadThread(IORequest request) const {
    request.execute();
}