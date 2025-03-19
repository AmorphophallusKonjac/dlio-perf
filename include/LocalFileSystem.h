#ifndef LOCALFILESYSTEM_H
#define LOCALFILESYSTEM_H

#include <FileSystem.h>

class LocalFileSystem final : public FileSystem
{
public:
    std::unique_ptr<File> getFileDescriptor() override;

    std::vector<Dentry> readDir(std::string path) override;

    bool createDir(std::string path) override;
    void remove(std::string path) override;

    ~LocalFileSystem() override = default;
};


#endif //LOCALFILESYSTEM_H
