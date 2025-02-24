#include <mpi.h>
#include <LogManager.h>
#include <ConfigManager.h>
#include <CLI11.hpp>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    CLI::App app{"a deep learning I/O benchmark"};
    std::string config_file = "./config.yaml";
    std::string master_host = "127.0.0.1";
    short port = 9000;
    std::string slave_name = "slave";
    MPI_Finalize();
    return 0;
}