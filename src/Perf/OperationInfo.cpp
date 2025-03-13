#include "OperationInfo.h"

OperationInfo::OperationInfo(OperationTy ty,
                             std::chrono::steady_clock::time_point start_time,
                             std::chrono::steady_clock::time_point end_time) :
    ty(ty), start_time(start_time), end_time(end_time), size(0) {
    latency = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time);
}

OperationInfo::OperationInfo(OperationTy ty,
                             std::chrono::steady_clock::time_point start_time,
                             std::chrono::steady_clock::time_point end_time,
                             long long size) : ty(ty), start_time(start_time),
                                               end_time(end_time), size(size) {
}