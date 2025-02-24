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
}

void RunnerSlave::start() {
    try {
        registerSelf();
        initialize();
        // waitStart();
        // benchmark start
    } catch (const std::exception& e) {
        LOGF(FATAL, "%s", e.what());
    }
}

void RunnerSlave::generate() {
    fs::path work_dir = fs::current_path();
    const auto dataset_config = ConfigManager::getInstance().dataset;
    const auto checkpoint_config = ConfigManager::getInstance().checkpoint;
    const auto fs = fs_factory_.getFileSystem();
    std::vector<IORequest> dir_requests;
    std::vector<IORequest> file_requests;
    fs::path data_folder(dataset_config.data_folder);
    if (data_folder.is_relative()) {
        data_folder = (work_dir / data_folder).lexically_normal();
    }
    // generate dataset dir
    fs->createDir(data_folder.string());
    long long dir_num = 1;
    std::vector<int> dir_encode;
    for (auto dir_dim_num : dataset_config.sample_sub_dir) {
        dir_requests.clear();
        dir_num *= dir_dim_num;
        dir_encode.push_back(0);
        for (int& i : dir_encode)
            i = 0;
        for (long long i = 0; i < dir_num; ++i) {
            fs::path dir = data_folder;
            for (const auto idx : dir_encode) {
                dir /= "dir" + std::to_string(idx);
            }
            dir_requests.emplace_back(IORequest::TaskTy::CREATE_DIR,
                                      dir.string(),
                                      0);
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
                                       dataset_config.sample_size, fs,
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
                                   dataset_config.sample_size);
    }
    BatchTask generate_dataset_file(file_requests.size(), 0,
                                    dataset_config.write_threads,
                                    dataset_config.sample_size, fs,
                                    "GenDatasetFile");
    generate_dataset_file.startIOCtrlThread(file_requests);
    generate_dataset_file.mainTask();
    generate_dataset_file.stopIOCtrlThread();
    // generate checkpoint file
    file_requests.clear();
    fs::path ck_folder(checkpoint_config.checkpoint_folder);
    if (ck_folder.is_relative()) {
        ck_folder = (work_dir / ck_folder).lexically_normal();
    }
    fs->createDir(ck_folder.string());
    fs::path ck_file_path =
        ck_folder / ("checkpoint_" + std::to_string(slave_id_) + "_0");
    auto ck_file = fs->getFileDescriptor();
    ck_file->open(ck_file_path.string(), File::WRITE);
    ck_file->writeWholeFile(checkpoint_config.checkpoint_size,
                            checkpoint_config.write_transfer_size);
    ck_file->close();
}

void RunnerSlave::initialize() {
    LOGF(INFO, "Start initialization...");
    if (ConfigManager::getInstance().workflow.gen_data) {
        LOGF(INFO, "Start generate dataset & checkpoint...");
        generate();
        LOGF(INFO, "Dataset & checkpoint generate done");
    }
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

void RunnerSlave::waitStart() {
    while (!reportReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}