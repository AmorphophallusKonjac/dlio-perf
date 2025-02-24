#include <ConfigManager.h>
#include <RunnerSlave.h>
#include <LogManager.h>
#include <CLI11.hpp>

class CLIParser {
public:
    CLIParser() {
        app.add_option("--config", config_file_)->required();
        app.add_option("--master_host", master_host_)->required();
        app.add_option("--master_port", master_port_)->check(
            CLI::Range(1, 65535))->required();
        app.add_option("--slave_id", slave_id_)->required();
        app.add_option("--slave_name", slave_name_)->required();
        app.add_option("--slave_num", slave_num_)->required();
        app.add_option("--log_dir", log_dir_);
    }

    void parse(const int argc, char** argv) {
        app.parse(argc, argv);
    }

    int exit(const CLI::ParseError& e) const {
        return app.exit(e);
    }

    std::string config_file_ = "./config.yaml";
    std::string master_host_ = "127.0.0.1";
    unsigned short master_port_ = 9000;
    int slave_id_ = 0;
    std::string slave_name_ = "slave";
    int slave_num_ = 1;
    std::string log_dir_ = "./logs";

private:
    CLI::App app{"a deep learning I/O benchmark"};
};

int main(int argc, char** argv) {
    CLIParser cli_parser;
    try {
        cli_parser.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return cli_parser.exit(e);
    }

    LogManager::getInstance(cli_parser.slave_name_, cli_parser.log_dir_);
    ConfigManager::getInstance().fromYaml(cli_parser.config_file_);
    RunnerSlave slave(cli_parser.master_host_, cli_parser.master_port_,
                      cli_parser.slave_name_, cli_parser.slave_id_,
                      cli_parser.slave_num_);
    slave.start();
    return 0;
}