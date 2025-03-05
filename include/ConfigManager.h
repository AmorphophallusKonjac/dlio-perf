#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <vector>
#include <rest_rpc/rpc_client.hpp>
#include <rest_rpc/rpc_server.h>

class ConfigManager
{
public:
    struct EnvConfig
    {
        std::string filesystem = "LocalFS";
    };

    EnvConfig env;

    struct WorkflowConfig
    {
        bool gen_data = false;
        bool train = true;
        bool checkpoint = true;
        bool gen_sample = false;
    };

    WorkflowConfig workflow;

    struct DatasetConfig
    {
        int sample_size = 262144;
        long long sample_num = 1;
        std::string data_folder = "./data";
        std::vector<int> sample_sub_dir = std::vector<int>();
        int write_threads = 1;
    };

    DatasetConfig dataset;

    struct GenerateConfig
    {
        std::string generate_folder = "./generate";
        int gen_intervals = 1;
        int gen_sample_num = 1;
        std::vector<int> gen_sub_dir;
        int write_threads = 1;
        long long transfer_size = 262144;
        double transfer_size_stdev = 0.0;
    };

    GenerateConfig generate;

    struct ReaderConfig
    {
        int batch_size = 1;
        int read_threads = 1;
        int prefetch_size = 0;
        std::string shuffle = "none";
        std::string seed = "rand";
        int transfer_size = 262144;
        double transfer_size_stdev = 0.0;
    };

    ReaderConfig reader;

    struct TrainConfig
    {
        int epochs = 1;
    };

    TrainConfig train;

    struct CheckpointConfig
    {
        enum CheckpointTy
        {
            SYNC,
            ASYN,
            SNAPSHOP
        };

        std::string checkpoint_folder = "./checkpoints";
        int checkpoint_interval = 1;
        long long checkpoint_size = 262144;
        int read_threads = 1;
        int write_threads = 1;
        CheckpointTy checkpoint_type = SYNC;
        int checkpoint_layers = 1;
        long long read_transfer_size = 262144;
        double read_transfer_size_stdev = 0.0;
        long long write_transfer_size = 262144;
        double write_transfer_size_stdev = 0.0;
    };

    CheckpointConfig checkpoint;

    struct OutputConfig
    {
        std::string folder = "./result";
    };

    OutputConfig output;


    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    static ConfigManager& getInstance()
    {
        static ConfigManager instance;
        return instance;
    }

    void fromYaml(const std::string& file);

    uint32_t getRandSeed(rest_rpc::rpc_service::rpc_conn conn);

private:
    ConfigManager() = default;
    void checkConfig();
};


#endif //CONFIGMANAGER_H
