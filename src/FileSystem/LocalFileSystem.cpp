#include "LocalFileSystem.h"

#include<filesystem>
#include<LocalFile.h>
namespace fs = std::filesystem;

std::unique_ptr<File> LocalFileSystem::getFileDescriptor() {
    return std::make_unique<LocalFile>();
}

std::vector<std::string> LocalFileSystem::readDir(std::string path) {
    std::vector<std::string> dentry_vector;
    for (const auto& dentry : fs::directory_iterator(path)) {
        dentry_vector.push_back(dentry.path());
    }
    return std::move(dentry_vector);
}

bool LocalFileSystem::createDir(std::string path) {
    return fs::create_directory(path);
}

