#include "LocalFile.h"

#include <g3log/g3log.hpp>

bool LocalFile::read(char* buffer, const int pos, const int size) {
    try {
        if (pos != pos_)
            file_.seekp(pos);
        file_.read(buffer, size);
        pos_ = pos + size;
        return true;
    } catch (const std::exception& e) {
        LOGF(FATAL, "read %s fail, %s", path_.c_str(), e.what());
        return false;
    }
}

bool LocalFile::write(const char* buffer, const int pos, const int size) {
    try {
        if (pos != pos_)
            file_.seekp(pos);
        file_.write(buffer, size);
        pos_ = pos + size;
        return true;
    } catch (const std::exception& e) {
        LOGF(FATAL, "write %s fail, %s", path_.c_str(), e.what());
        return false;
    }
}

bool LocalFile::open(std::string path, Flag flag) {
    try {
        path_ = path;
        file_.open(path);
        return true;
    } catch (const std::exception& e) {
        LOGF(FATAL, "open %s fail, %s", path_.c_str(), e.what());
        return false;
    }
}

void LocalFile::close() {
    file_.close();
}



