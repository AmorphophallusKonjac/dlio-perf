#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <vector>

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
        int sample_size = 65536;
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
        bool shuffle = false;
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
            ASYN
        };

        std::string checkpoint_folder = "./checkpoints";
        int checkpoint_interval = 1;
        long long checkpoint_size = 1024;
        int read_threads = 1;
        int write_threads = 1;
        CheckpointTy checkpoint_write_type = SYNC;
        int read_transfer_size = 262144;
        double read_transfer_size_stdev = 0.0;
        int write_transfer_size = 262144;
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

private:
    ConfigManager() = default;
};


#endif //CONFIGMANAGER_H
