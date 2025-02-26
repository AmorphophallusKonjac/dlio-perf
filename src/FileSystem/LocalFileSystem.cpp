#include "LocalFileSystem.h"

#include<filesystem>
#include<LocalFile.h>
namespace fs = std::filesystem;

std::unique_ptr<File> LocalFileSystem::getFileDescriptor() {
    return std::make_unique<LocalFile>();
}

std::vector<Dentry> LocalFileSystem::readDir(std::string path) {
    std::vector<Dentry> dentry_vector;
    for (const auto& dentry : fs::directory_iterator(path)) {
        if (dentry.is_directory()) {
            dentry_vector.emplace_back(Dentry::DIR, dentry.path());
        } else if (dentry.is_regular_file()) {
            dentry_vector.emplace_back(Dentry::FILE, dentry.path());
        }
    }
    return std::move(dentry_vector);
}

bool LocalFileSystem::createDir(std::string path) {
    return fs::create_directory(path);
}

