#include "LogManager.h"

#include <filesystem>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

LogManager::LogManager(std::string prefix, std::string folder) {
    worker = g3::LogWorker::createLogWorker();
    if (!std::filesystem::exists(folder)) {
        if (!std::filesystem::create_directories(folder)) {
            folder = ".";
        }
    }
    auto handle = worker->addDefaultLogger(prefix, folder);
    g3::initializeLogging(worker.get());
}

LogManager::~LogManager() {
    g3::internal::shutDownLogging();
}
