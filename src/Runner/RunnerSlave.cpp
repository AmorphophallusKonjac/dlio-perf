#include "RunnerSlave.h"

#include <BatchTask.h>
#include <rest_rpc.hpp>
#include <g3log/g3log.hpp>
#include <ConfigManager.h>
#include <FileSystem.h>
#include <numeric>
#include <filesystem>
#include <utility>

namespace fs = std::filesystem;

RunnerSlave::RunnerSlave(const std::string& host, unsigned short port,
                         std::string name, const int slave_id,
                         const int slave_num) :
    slave_id_(slave_id),
    slave_num_(slave_num),
    rpc_client_(std::make_unique<rest_rpc::rpc_client>(host, port)),
    name_(std::move(name)) {
    rpc_client_->enable_auto_reconnect();
}

void RunnerSlave::start() {
    try {
        registerSelf();
        initialize();
        waitStart();
        // benchmark start
        reportFinish(run());
    } catch (const std::exception& e) {
        LOGF(FATAL, "%s", e.what());
    }
}

std::vector<std::string> RunnerSlave::getShuffleFileList() {
    auto shuffle_mode = ConfigManager::getInstance().reader.shuffle;
    auto slice_len = trainFileList_.size() / slave_num_;
    slice_len -= slice_len % ConfigManager::getInstance().reader.batch_size;
    std::vector<std::string> shuffle_file_list;
    if (shuffle_mode == "none") {
        for (int i = 0, j = slave_id_; i < slice_len; ++i) {
            shuffle_file_list.emplace_back(trainFileList_[j]);
            j += slave_num_;
        }
    } else if (shuffle_mode == "local") {
        for (int i = 0, j = slave_id_; i < slice_len; ++i) {
            shuffle_file_list.emplace_back(trainFileList_[j]);
            j += slave_num_;
        }
        std::shuffle(shuffle_file_list.begin(), shuffle_file_list.end(),
                     rand_engine_);
    } else if (shuffle_mode == "global") {
        std::shuffle(trainFileList_.begin(), trainFileList_.end(),
                     rand_engine_);
        for (int i = 0, j = slave_id_; i < slice_len; ++i) {
            shuffle_file_list.emplace_back(trainFileList_[j]);
            j += slave_num_;
        }
    } else {
        throw std::runtime_error("Unknown shuffle mode");
    }
    return shuffle_file_list;
}

bool RunnerSlave::run() {
    if (ConfigManager::getInstance().workflow.train) {
        try {
            LOGF(INFO, "Start benchmark...");
            const auto epochs = ConfigManager::getInstance().train.epochs;
            const auto workflow_config = ConfigManager::getInstance().workflow;
            const auto reader_config = ConfigManager::getInstance().reader;
            const auto dataset_config = ConfigManager::getInstance().dataset;
            const auto checkpoint_config = ConfigManager::getInstance().
                checkpoint;
            const auto fs = fs_factory_.getFileSystem();
            // load checkpoint
            loadCheckpoint();
            // start batch
            for (int i = 0; i < epochs; ++i) {
                readSamples();
                if (workflow_config.checkpoint && i && i % checkpoint_config.
                    checkpoint_interval == 0) {
                    saveCheckpoint();
                }
            }
            LOGF(INFO, "Benchmark finish");
            return true;
        } catch (const std::exception& e) {
            LOGF(FATAL, "slave %s exception: %s", name_.c_str(), e.what());
            return false;
        }
    }
    return true;
}

void RunnerSlave::readSamples() {
    const auto reader_config = ConfigManager::getInstance().reader;
    const auto dataset_config = ConfigManager::getInstance().dataset;
    const auto fs = fs_factory_.getFileSystem();
    auto train_file_list = getShuffleFileList();
    const auto batch_steps = (dataset_config.sample_num / slave_num_) /
                             reader_config.batch_size;
    std::vector<IORequest> reader_requests;
    for (auto file : train_file_list) {
        reader_requests.emplace_back(IORequest::READ, file,
                                     dataset_config.sample_size, 0,
                                     reader_config.transfer_size,
                                     fs);
    }
    BatchTask read_sample_task(reader_config.batch_size,
                               reader_config.prefetch_size,
                               reader_config.read_threads,
                               reader_config.transfer_size,
                               "reader");
    read_sample_task.startIOCtrlThread(reader_requests);
    for (int j = 0; j < batch_steps; ++j) {
        read_sample_task.mainTask();
    }
    read_sample_task.stopIOCtrlThread();
}

void RunnerSlave::loadCheckpoint() {
    const auto fs = fs_factory_.getFileSystem();
    ck_factory_.getCheckpoint(slave_id_, fs)->load();
}

void RunnerSlave::saveCheckpoint() {
    const auto fs = fs_factory_.getFileSystem();
    ck_factory_.getCheckpoint(slave_id_, fs)->save();
}

void RunnerSlave::generate() {
    const auto dataset_config = ConfigManager::getInstance().dataset;
    const auto checkpoint_config = ConfigManager::getInstance().checkpoint;
    const auto fs = fs_factory_.getFileSystem();
    std::vector<IORequest> dir_requests;
    std::vector<IORequest> file_requests;
    // generate dataset dir
    fs->createDir(dataset_config.data_folder);
    long long dir_num = 1;
    std::vector<int> dir_encode;
    for (auto dir_dim_num : dataset_config.sample_sub_dir) {
        dir_requests.clear();
        dir_num *= dir_dim_num;
        dir_encode.push_back(0);
        for (int& i : dir_encode)
            i = 0;
        for (long long i = 0; i < dir_num; ++i) {
            fs::path dir = dataset_config.data_folder;
            for (const auto idx : dir_encode) {
                dir /= "dir" + std::to_string(idx);
            }
            dir_requests.emplace_back(IORequest::TaskTy::CREATE_DIR,
                                      dir.string(), 0, 0, 0, fs);
            for (int j = dir_encode.size() - 1; j >= 0; --j) {
                ++dir_encode[j];
                if (dir_encode[j] == dataset_config.sample_sub_dir[j]) {
                    dir_encode[j] = 0;
                } else {
                    break;
                }
            }
        }
        BatchTask generate_dataset_dir(dir_requests.size(), 0,
                                       dataset_config.write_threads,
                                       dataset_config.sample_size,
                                       "GenDatasetDir");
        generate_dataset_dir.startIOCtrlThread(dir_requests);
        generate_dataset_dir.mainTask();
        generate_dataset_dir.stopIOCtrlThread();
    }
    // generate dataset file
    const auto sample_num_per_subdir =
        dataset_config.sample_num / dir_num + (
            (dataset_config.sample_num % dir_num) != 0);
    for (int i = 0, j = -1; i < dataset_config.sample_num; ++i) {
        if (i % sample_num_per_subdir == 0)
            ++j;
        auto file_path = fs::path(dir_requests[j].path) / (
                             "sample" + std::to_string(i));
        file_requests.emplace_back(IORequest::TaskTy::WRITE, file_path.string(),
                                   dataset_config.sample_size, 0,
                                   dataset_config.sample_size, fs);
    }
    BatchTask generate_dataset_file(file_requests.size(), 0,
                                    dataset_config.write_threads,
                                    dataset_config.sample_size,
                                    "GenDatasetFile");
    generate_dataset_file.startIOCtrlThread(file_requests);
    generate_dataset_file.mainTask();
    generate_dataset_file.stopIOCtrlThread();
    // generate checkpoint file
    ck_factory_.getCheckpoint(slave_id_, fs)->generate();
}

void RunnerSlave::getTrainFileList() {
    const auto fs = fs_factory_.getFileSystem();
    trainFileList_.clear();
    std::queue<std::string> queue;
    for (queue.push(ConfigManager::getInstance().dataset.data_folder); !queue.
         empty(); queue.pop()) {
        auto dentry_vector = fs->readDir(queue.front());
        for (const auto& dentry : dentry_vector) {
            switch (dentry.ty_) {
                case Dentry::DIR:
                    queue.push(dentry.path_);
                    break;
                case Dentry::FILE:
                    trainFileList_.emplace_back(dentry.path_);
                    break;
            }
        }
    }
    if (trainFileList_.size() != ConfigManager::getInstance().dataset.
        sample_num) {
        throw std::runtime_error("Dataset error");
    }
}

void RunnerSlave::initialize() {
    LOGF(INFO, "Start initialization...");
    if (ConfigManager::getInstance().workflow.gen_data) {
        LOGF(INFO, "Start generate dataset & checkpoint...");
        generate();
        LOGF(INFO, "Dataset & checkpoint generate done");
    }
    LOGF(INFO, "Scan dataset to get train file list...");
    getTrainFileList();
    LOGF(INFO, "Dataset scan done");
    rand_engine_.seed(getRandSeed());
}

void RunnerSlave::registerSelf() {
    if (!rpc_client_->connect(10)) {
        LOGF(FATAL, "Slave connect timeout");
        throw std::runtime_error("Fail to connect server");
    }
    rpc_client_->call<void>("register", name_);
    rpc_client_->close();
}

bool RunnerSlave::reportReady() {
    if (!rpc_client_->connect(10)) {
        LOGF(FATAL, "Slave connect timeout");
        throw std::runtime_error("Fail to connect server");
    }
    const auto ret = rpc_client_->call<bool>("reportReady", name_);
    rpc_client_->close();
    return ret;
}

void RunnerSlave::reportFinish(bool success) {
    if (!rpc_client_->connect(10)) {
        LOGF(FATAL, "Slave connect timeout");
        throw std::runtime_error("Fail to connect server");
    }
    rpc_client_->call<void>("reportFinish", name_, success);
}

void RunnerSlave::waitStart() {
    while (!reportReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

uint32_t RunnerSlave::getRandSeed() const {
    if (!rpc_client_->connect(10)) {
        LOGF(FATAL, "Slave connect timeout");
        throw std::runtime_error("Fail to connect server");
    }
    const auto ret = rpc_client_->call<uint32_t>("getRandSeed");
    return ret;
}