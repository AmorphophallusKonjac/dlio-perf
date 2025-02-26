#ifndef DENTRY_H
#define DENTRY_H

#include <string>

class Dentry {
public:
    enum DentryType {
        DIR,
        FILE
    };

    Dentry(DentryType ty, const std::string& path);

    DentryType ty_;
    std::string path_;
};



#endif //DENTRY_H
