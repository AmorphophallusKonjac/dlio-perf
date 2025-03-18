#include <ConfigManager.h>
#include <RunnerSlave.h>
#include <CLI11/CLI11.hpp>
#include <mpi/mpi.h>

class CLIParser {
public:
    CLIParser() {
        app.add_option("--config", config_file_)->required();
        app.add_option("--log_dir", log_dir_);
    }

    void parse(const int argc, char** argv) {
        app.parse(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &slave_id_);
        MPI_Comm_size(MPI_COMM_WORLD, &slave_num_);
    }

    int exit(const CLI::ParseError& e) const {
        return app.exit(e);
    }

    std::string config_file_ = "./config.yaml";
    int slave_id_ = 0;
    int slave_num_ = 1;
    std::string log_dir_;

private:
    CLI::App app{"a deep learning I/O benchmark"};
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    CLIParser cli_parser;
    try {
        cli_parser.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return cli_parser.exit(e);
    }

    ConfigManager::getInstance().fromYaml(cli_parser.config_file_);
    RunnerSlave slave(cli_parser.slave_id_, cli_parser.slave_num_);
    slave.start();

    MPI_Finalize();
    return 0;
}