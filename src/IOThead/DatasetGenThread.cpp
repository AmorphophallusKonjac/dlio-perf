#include "DatasetGenThread.h"

#include <fstream>
#include <filesystem>

DatasetGenThread::DatasetGenThread(FileSystem* fs) :
    fs_(fs) {
}

void DatasetGenThread::insertDir(std::string dir) {
    dir_list_.push_back(dir);
}

void DatasetGenThread::insertFile(std::string file) {
    file_list_.push_back(file);
}

void DatasetGenThread::run() {
    for (const auto& dir : dir_list_) {
        fs_->createDir(dir);
    }

    for (const auto& file_path : file_list_) {
        std::ofstream file;
        file.open(file_path, std::ios::out);
    }
}


