#include "IORequest.h"

#include <PerfCounter.h>

IORequest::IORequest(const TaskTy ty, std::string path, const long long size,
                     const long long pos, const long long transfer_size,
                     FileSystem* fs) :
    ty(ty), path(std::move(path)),
    transfer_size(transfer_size), fs(fs) {
    this->size_.emplace_back(size);
    this->pos_.emplace_back(pos);
}

IORequest::IORequest(TaskTy ty, std::string path, long long transfer_size,
                     FileSystem* fs) : ty(ty), path(std::move(path)),
                                       transfer_size(transfer_size), fs(fs) {
}

IORequest::IORequest() : ty(READ), transfer_size(-1), fs(nullptr) {
}

bool IORequest::empty() const {
    return path.empty();
}

void IORequest::execute() const {
    const auto file = fs->getFileDescriptor();
    switch (ty) {
        case READ: {
            auto start = std::chrono::steady_clock::now();
            file->open(path, File::Flag::READ);
            auto end = std::chrono::steady_clock::now();
            PerfCounter::getInstance().addOperation(
                OperationInfo::OPEN, start, end);
        }
        break;
        case WRITE: {
            auto start = std::chrono::steady_clock::now();
            file->open(path, File::Flag::WRITE);
            auto end = std::chrono::steady_clock::now();
            PerfCounter::getInstance().addOperation(
                OperationInfo::OPEN, start, end);
        }
        break;
        default:
            break;
    }
    for (size_t i = 0; i < pos_.size(); ++i) {
        auto pos = pos_[i];
        auto size = size_[i];
        const auto buffer = new char[size];
        switch (ty) {
            case READ:
                for (long long i = 0; i < size; i = i + transfer_size) {
                    auto trans_size = transfer_size;
                    if (i + transfer_size > size)
                        trans_size = size - i;
                    auto start = std::chrono::steady_clock::now();
                    file->read(buffer, pos + i, trans_size);
                    auto end = std::chrono::steady_clock::now();
                    PerfCounter::getInstance().addOperation(
                        OperationInfo::READ, start, end, trans_size);
                }
                break;
            case WRITE:
                for (long long i = 0; i < size; i = i + transfer_size) {
                    auto trans_size = transfer_size;
                    if (i + transfer_size > size)
                        trans_size = size - i;
                    auto start = std::chrono::steady_clock::now();
                    file->write(buffer, pos + i, trans_size);
                    auto end = std::chrono::steady_clock::now();
                    PerfCounter::getInstance().addOperation(
                        OperationInfo::WRITE, start, end, trans_size);
                }
                break;
            case CREATE_DIR:
                fs->createDir(path);
                break;
        }
        delete[] buffer;
    }
    switch (ty) {
        case READ:
        case WRITE:
            file->close();
            break;
        default:
            break;
    }
}

void IORequest::addIOReq(long long pos, long size) {
    this->pos_.emplace_back(pos);
    this->size_.emplace_back(size);
}