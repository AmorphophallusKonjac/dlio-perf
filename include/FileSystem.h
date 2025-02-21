#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <File.h>
#include <string>
#include <vector>
#include <memory>

class FileSystem
{
public:
    virtual std::unique_ptr<File> getFileDescriptor() = 0;

    virtual std::vector<std::string> readDir(std::string path) = 0;

    virtual bool createDir(std::string path) = 0;

    virtual ~FileSystem();
};


#endif //FILESYSTEM_H
