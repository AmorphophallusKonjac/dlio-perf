#include <ConfigManager.h>
#include <LogManager.h>
#include <CLI11.hpp>
#include <RunnerMaster.h>

class CLIParser {
public:
    CLIParser() {
        app.add_option("--config", config_file_)->required();
        app.add_option("--master_port", master_port_)->check(
            CLI::Range(1, 65535))->required();
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
    unsigned short master_port_ = 9000;
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

    LogManager::getInstance("master", cli_parser.log_dir_);
    ConfigManager::getInstance().fromYaml(cli_parser.config_file_);
    RunnerMaster master(cli_parser.master_port_, 4, cli_parser.slave_num_);
    master.start();

    return 0;
}