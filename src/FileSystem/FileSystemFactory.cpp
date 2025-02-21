#include "FileSystemFactory.h"
#include "g3log/g3log.hpp"
#include "ConfigManager.h"
#include "LocalFileSystem.h"

FileSystem* FileSystemFactory::getFileSystem() {
    if (fs_)
        return fs_;
    if (const std::string fs = ConfigManager::getInstance().env.filesystem; fs == "LocalFS") {
        fs_ = new LocalFileSystem();
    } else {
        LOGF(WARNING, "Unknown type of filesystem, set LocalFS instead");
        fs_ = new LocalFileSystem();
    }
    return fs_;
}

FileSystemFactory::~FileSystemFactory() {
    delete fs_;
}
