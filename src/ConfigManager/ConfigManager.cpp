#include "ConfigManager.h"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;

void ConfigManager::fromYaml(const std::string& file) {
    try {
        spdlog::info("Loading yaml file {}", file);
        YAML::Node config = YAML::LoadFile(file);
        node_ = config;

        if (config["env"]) {
            if (const auto node = config["env"]["filesystem"]) {
                env.filesystem = node.as<std::string>();
            }
        }

        if (config["workflow"]) {
            if (const auto node = config["workflow"]["gen_data"]) {
                workflow.gen_data = node.as<bool>();
            }
            if (const auto node = config["workflow"]["train"]) {
                workflow.train = node.as<bool>();
            }
            if (const auto node = config["workflow"]["checkpoint"]) {
                workflow.checkpoint = node.as<bool>();
            }
            if (const auto node = config["workflow"]["gen_sample"]) {
                workflow.gen_sample = node.as<bool>();
            }
            if (!workflow.train) {
                workflow.checkpoint = false;
                workflow.gen_sample = false;
            }
        }

        if (config["dataset"]) {
            if (const auto node = config["dataset"]["sample_size"]) {
                dataset.sample_size = node.as<int>();
            }
            if (const auto node = config["dataset"]["sample_num"]) {
                dataset.sample_num = node.as<long long>();
            }
            if (const auto node = config["dataset"]["data_folder"]) {
                dataset.data_folder = node.as<std::string>();
            }
            if (const auto node = config["dataset"]["sample_sub_dir"]) {
                dataset.sample_sub_dir.clear();
                for (std::size_t i = 0; i < node.size(); ++i) {
                    dataset.sample_sub_dir.push_back(node[i].as<int>());
                }
            }
            if (const auto node = config["dataset"]["write_threads"]) {
                dataset.write_threads = node.as<int>();
            }
        }

        if (config["generate"]) {
            if (const auto node = config["generate"]["generate_folder"]) {
                generate.generate_folder = node.as<std::string>();
            }
            if (const auto node = config["generate"]["gen_intervals"]) {
                generate.gen_intervals = node.as<int>();
            }
            if (const auto node = config["generate"]["gen_sample_num"]) {
                generate.gen_sample_num = node.as<int>();
            }
            if (const auto node = config["generate"]["gen_sub_dir"]) {
                generate.gen_sub_dir.clear();
                for (std::size_t i = 0; i < node.size(); ++i) {
                    generate.gen_sub_dir.push_back(node[i].as<int>());
                }
            }
            if (const auto node = config["generate"]["write_threads"]) {
                generate.write_threads = node.as<int>();
            }
            if (const auto node = config["generate"]["transfer_size"]) {
                generate.transfer_size = node.as<int>();
            }
            if (const auto node = config["generate"]["transfer_size_stdev"]) {
                generate.transfer_size_stdev = node.as<double>();
            }
        }

        if (config["reader"]) {
            if (const auto node = config["reader"]["batch_size"]) {
                reader.batch_size = node.as<int>();
            }
            if (const auto node = config["reader"]["read_threads"]) {
                reader.read_threads = node.as<int>();
            }
            if (const auto node = config["reader"]["prefetch_size"]) {
                reader.prefetch_size = node.as<int>();
            }
            if (const auto node = config["reader"]["shuffle"]) {
                reader.shuffle = node.as<std::string>();
            }
            if (const auto node = config["reader"]["seed"]) {
                reader.seed = node.as<std::string>();
            }
            if (const auto node = config["reader"]["transfer_size"]) {
                reader.transfer_size = node.as<int>();
            }
            if (const auto node = config["reader"]["transfer_size_stdev"]) {
                reader.transfer_size_stdev = node.as<double>();
            }
        }

        if (config["train"]) {
            if (const auto node = config["train"]["steps"]) {
                train.steps = node.as<int>();
            }
            if (const auto node = config["train"]["interval"]) {
                train.interval = node.as<int>();
            }
        }

        if (config["checkpoint"]) {
            if (const auto node = config["checkpoint"]["checkpoint_folder"]) {
                checkpoint.checkpoint_folder = node.as<std::string>();
            }
            if (const auto node = config["checkpoint"]["checkpoint_interval"]) {
                checkpoint.checkpoint_interval = node.as<int>();
            }
            if (const auto node = config["checkpoint"]["checkpoint_size"]) {
                checkpoint.checkpoint_size = node.as<long long>();
            }
            if (const auto node = config["checkpoint"]["checkpoint_layers"]) {
                checkpoint.checkpoint_layers = node.as<int>();
            }
            if (const auto node = config["checkpoint"]["read_threads"]) {
                checkpoint.read_threads = node.as<int>();
            }
            if (const auto node = config["checkpoint"]["write_threads"]) {
                checkpoint.write_threads = node.as<int>();
            }
            if (const auto node = config["checkpoint"][
                "checkpoint_write_type"]) {
                if (node.as<std::string>() == "sync") {
                    checkpoint.checkpoint_type = CheckpointConfig::SYNC;
                } else if (node.as<std::string>() == "asyn") {
                    checkpoint.checkpoint_type = CheckpointConfig::ASYN;
                } else if (node.as<std::string>() == "snapshot") {
                    checkpoint.checkpoint_type = CheckpointConfig::SNAPSHOP;
                } else {
                    spdlog::warn(
                        "Unknown checkpoint write type. Use sync type");
                    checkpoint.checkpoint_type = CheckpointConfig::SYNC;
                }
            }
            if (const auto node = config["checkpoint"]["read_transfer_size"]) {
                checkpoint.read_transfer_size = node.as<long long>();
            }
            if (const auto node = config["checkpoint"][
                "read_transfer_size_stdev"]) {
                checkpoint.read_transfer_size_stdev = node.as<double>();
            }
            if (const auto node = config["checkpoint"]["write_transfer_size"]) {
                checkpoint.write_transfer_size = node.as<long long>();
            }
            if (const auto node = config["checkpoint"][
                "write_transfer_size_stdev"]) {
                checkpoint.write_transfer_size_stdev = node.as<double>();
            }
        }

        if (config["output"]) {
            if (const auto node = config["output"]["folder"]) {
                output.folder = node.as<std::string>();
            }
        }
    } catch (const std::exception& e) {
        spdlog::error(e.what());
    }
    checkConfig();
}

void ConfigManager::checkConfig() {
    reader.transfer_size = std::min(dataset.sample_size, reader.transfer_size);
    checkpoint.read_transfer_size = std::min(checkpoint.read_transfer_size,
                                             checkpoint.checkpoint_size);
    checkpoint.write_transfer_size = std::min(checkpoint.write_transfer_size,
                                              checkpoint.checkpoint_size);
    const fs::path work_dir = fs::current_path();
    if (const auto data_folder = fs::path(dataset.data_folder);
        data_folder.is_relative())
        dataset.data_folder = (work_dir / dataset.data_folder).
                              lexically_normal().string();
    if (const auto ck_folder = fs::path(checkpoint.checkpoint_folder);
        ck_folder.is_relative())
        checkpoint.checkpoint_folder =
            (work_dir / checkpoint.checkpoint_folder).
            lexically_normal().string();
    if (const auto res_folder = fs::path(output.folder); res_folder.
        is_relative()) {
        output.folder = (work_dir / output.folder).lexically_normal().string();
    }
}
