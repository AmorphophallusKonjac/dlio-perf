#include "PerfCounter.h"

#include <map>
#include <mutex>

void PerfCounter::addOperation(const OperationInfo& op) {
    std::lock_guard lock_guard(ops_mutex_);
    switch (op.ty) {
        case OperationInfo::READ:
            read_ops_.push_back(op);
            break;
        case OperationInfo::WRITE:
            write_ops_.push_back(op);
            break;
        case OperationInfo::OPEN:
            open_ops_.push_back(op);
            break;
    }
}

void PerfCounter::addOperation(OperationInfo::OperationTy ty,
                               std::chrono::steady_clock::time_point start_time,
                               std::chrono::steady_clock::time_point end_time) {
    std::lock_guard lock_guard(ops_mutex_);
    switch (ty) {
        case OperationInfo::READ:
            throw std::runtime_error("No size");
            break;
        case OperationInfo::WRITE:
            throw std::runtime_error("No size");
            break;
        case OperationInfo::OPEN:
            open_ops_.emplace_back(ty, start_time, end_time);
            break;
    }
}

void PerfCounter::addOperation(OperationInfo::OperationTy ty,
                               std::chrono::steady_clock::time_point start_time,
                               std::chrono::steady_clock::time_point end_time,
                               long long size) {
    std::lock_guard lock_guard(ops_mutex_);
    switch (ty) {
        case OperationInfo::READ:
            read_ops_.emplace_back(ty, start_time, end_time, size);
            break;
        case OperationInfo::WRITE:
            write_ops_.emplace_back(ty, start_time, end_time, size);
            break;
        case OperationInfo::OPEN:
            throw std::runtime_error("open do not have size");
            break;
    }
}

YAML::Node PerfCounter::getPerfResult() {
    YAML::Node perf_summary;
    BW_perf open_bw;
    calcPerfResult(read_ops_, read_qps_, read_bw_);
    calcPerfResult(write_ops_, write_qps_, write_bw_);
    calcPerfResult(open_ops_, open_qps_, open_bw);
    perf_summary["read"]["qps"] = read_qps_.toYaml();
    perf_summary["read"]["BW(KiB/s)"] = read_bw_.toYaml();
    perf_summary["write"]["qps"] = write_qps_.toYaml();
    perf_summary["write"]["BW(KiB/s)"] = write_bw_.toYaml();
    perf_summary["open"]["qps"] = write_qps_.toYaml();
    return std::move(perf_summary);
}

void PerfCounter::calcPerfResult(std::vector<OperationInfo>& ops, QPS_perf& qps,
                                 BW_perf& bw) {
    std::map<int, std::pair<int, long long>> results;
    std::vector<int> qps_vec;
    std::vector<long long> bw_vec;
    for (auto& op : ops) {
        int start_second = std::chrono::duration_cast<std::chrono::seconds>(
            op.start_time.time_since_epoch()).count();
        int end_second = std::chrono::duration_cast<std::chrono::seconds>(
            op.end_time.time_since_epoch()).count();
        ++results[start_second].first;
        long long bandwidth_per_second =
            op.size / (end_second - start_second + 1);
        for (int second = start_second; second <= end_second; ++second) {
            results[second].second += bandwidth_per_second / 1024;
        }
    }
    for (auto& result : results) {
        qps_vec.push_back(result.second.first);
        bw_vec.push_back(result.second.second);
    }
    qps = PerfResult<int>::calcPerfResult(qps_vec);
    bw = PerfResult<long long>::calcPerfResult(bw_vec);
}