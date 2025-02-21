#ifndef LOCALFILESYSTEM_H
#define LOCALFILESYSTEM_H

#include <FileSystem.h>

class LocalFileSystem final : public FileSystem
{
public:
    std::unique_ptr<File> getFileDescriptor() override;

    std::vector<std::string> readDir(std::string path) override;

    bool createDir(std::string path) override;
};


#endif //LOCALFILESYSTEM_H
