#ifndef FILESYSTEMFACTORY_H
#define FILESYSTEMFACTORY_H

#include <FileSystem.h>
#include <memory>

class FileSystemFactory {
public:
    FileSystem* getFileSystem();
    ~FileSystemFactory();
private:
    FileSystem* fs_ = nullptr;
};



#endif //FILESYSTEMFACTORY_H
