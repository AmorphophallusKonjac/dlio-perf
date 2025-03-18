#include "RunnerSlave.h"

#include <BatchTask.h>
#include <ConfigManager.h>
#include <FileSystem.h>
#include <PerfCounter.h>
#include <mpi.h>
#include <spdlog/spdlog.h>

#include <numeric>
#include <filesystem>
#include <utility>
#include <fstream>

namespace fs = std::filesystem;

RunnerSlave::RunnerSlave(const int slave_id, const int slave_num) :
    slave_id_(slave_id), slave_num_(slave_num) {
}

void RunnerSlave::start() {
    try {
        initialize();
        MPI_Barrier(MPI_COMM_WORLD);
        run();
        MPI_Barrier(MPI_COMM_WORLD);
        finalize();
    } catch (const std::exception& e) {
        spdlog::error("rank {} fail. {}", slave_id_, e.what());
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
            if (slave_id_ == 0) {
                spdlog::info("Benchmark start");
            }
            const auto epochs = ConfigManager::getInstance().train.epochs;
            const auto workflow_config = ConfigManager::getInstance().workflow;
            const auto reader_config = ConfigManager::getInstance().reader;
            const auto dataset_config = ConfigManager::getInstance().dataset;
            const auto checkpoint_config = ConfigManager::getInstance().
                checkpoint;
            // load checkpoint
            loadCheckpoint();
            // start batch
            for (int i = 0; i < epochs; ++i) {
                spdlog::info("Rank {} start epoch {}", slave_id_, i);
                readSamples();
                if (workflow_config.checkpoint && i && i % checkpoint_config.
                    checkpoint_interval == 0) {
                    saveCheckpoint();
                }
            }
            return true;
        } catch (const std::exception& e) {
            spdlog::error("Rank {} fail. {}", slave_id_, e.what());
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
    spdlog::info("Start generate data...");
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
    spdlog::info("Finish generate data");
}

void RunnerSlave::getTrainFileList() {
    trainFileList_.clear();
    std::vector<int> dir_encode;
    const auto dataset_config = ConfigManager::getInstance().dataset;
    long long dir_num = 1;
    for (auto dir_dim_num : dataset_config.sample_sub_dir) {
        dir_encode.push_back(0);
        dir_num *= dir_dim_num;
    }
    const auto sample_num_per_subdir =
        dataset_config.sample_num / dir_num + (
            (dataset_config.sample_num % dir_num) != 0);
    std::vector<std::string> dir_list;
    for (long long i = 0; i < dir_num; ++i) {
        fs::path dir = dataset_config.data_folder;
        for (const auto idx : dir_encode) {
            dir /= "dir" + std::to_string(idx);
        }
        dir_list.push_back(dir.string());
        for (int j = dir_encode.size() - 1; j >= 0; --j) {
            ++dir_encode[j];
            if (dir_encode[j] == dataset_config.sample_sub_dir[j]) {
                dir_encode[j] = 0;
            } else {
                break;
            }
        }
    }
    for (int i = 0, j = -1; i < dataset_config.sample_num; ++i) {
        if (i % sample_num_per_subdir == 0)
            ++j;
        auto file_path = fs::path(dir_list[j]) / (
                             "sample" + std::to_string(i));
        trainFileList_.push_back(file_path.string());
    }
}

void RunnerSlave::initialize() {
    spdlog::info("Start initialize...");
    if (ConfigManager::getInstance().workflow.gen_data && slave_id_ == 0) {
        generate();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    getTrainFileList();
    rand_engine_.seed(getRandSeed());
    spdlog::info("Finish initialize");
}

uint32_t RunnerSlave::getRandSeed() const {
    const auto seed_str = ConfigManager::getInstance().reader.seed;
    uint32_t seed;
    if (slave_id_ == 0) {
        if (seed_str == "rand") {
            std::random_device rd;
            seed = rd();
        } else {
            seed = std::stoul(seed_str);
        }
    }
    MPI_Bcast(&seed, 1, MPI_INT32_T, 0, MPI_COMM_WORLD);
    return seed;
}

void RunnerSlave::finalize() {
    if (ConfigManager::getInstance().workflow.train == false)
        return;
    std::filesystem::path output_folder(
        ConfigManager::getInstance().output.folder);
    report["rank"] = slave_id_;
    report["perf"] = PerfCounter::getInstance().getPerfResult();
    create_directory(output_folder);
    std::ofstream result(
        output_folder / (std::to_string(slave_id_) + "_result.yaml"));
    result << report;
    result.close();
}