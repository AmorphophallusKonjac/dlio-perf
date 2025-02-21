#ifndef DATASETGENTHREAD_H
#define DATASETGENTHREAD_H

#include <FileSystem.h>
#include <vector>
#include <string>

class DatasetGenThread
{
public:
    DatasetGenThread(FileSystem* fs);

    void insertDir(std::string dir);

    void insertFile(std::string file);

    void run();

private:
    std::vector<std::string> dir_list_;
    std::vector<std::string> file_list_;
    FileSystem* fs_;
};


#endif //DATASETGENTHREAD_H
