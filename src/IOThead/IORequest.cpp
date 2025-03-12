#include "IORequest.h"

#include <asio/registered_buffer.hpp>
#include <g3log/g3log.hpp>

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
        case READ:
            file->open(path, File::Flag::READ);
            break;
        case WRITE:
            file->open(path, File::Flag::WRITE);
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
                    if (i + transfer_size > size)
                        file->read(buffer, pos + i, size - i);
                    else
                        file->read(buffer, pos + i, transfer_size);
                }
                break;
            case WRITE:
                for (long long i = 0; i < size; i = i + transfer_size) {
                    if (i + transfer_size > size)
                        file->write(buffer, pos + i, size - i);
                    else
                        file->write(buffer, pos + i, transfer_size);
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