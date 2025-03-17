#include "PerfCounter.h"

#include <iomanip>
#include <map>
#include <mutex>
#include <boost/mpl/push_back.hpp>

void PerfCounter::addOperation(const OperationInfo& op) {
    std::lock_guard lock_guard(ops_mutex_);
    switch (op.ty) {
        case OperationInfo::READ:
            read_ops_.push_back(op);
            lat_read_ops_.push_back(op.latency.count() / 1000);
            gra_read_ops_.push_back(op.size / 1024);
            break;
        case OperationInfo::WRITE:
            write_ops_.push_back(op);
            lat_write_ops_.push_back(op.latency.count() / 1000);
            gra_write_ops_.push_back(op.size / 1024);
            break;
        case OperationInfo::OPEN:
            open_ops_.push_back(op);
            lat_open_ops_.push_back(op.latency.count() / 1000);
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
            lat_open_ops_.push_back(open_ops_.back().latency.count() / 1000);
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
            lat_read_ops_.push_back(read_ops_.back().latency.count() / 1000);
            break;
        case OperationInfo::WRITE:
            write_ops_.emplace_back(ty, start_time, end_time, size);
            lat_write_ops_.push_back(write_ops_.back().latency.count() / 1000);
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
    auto read_lat_per = getPercentiles(lat_read_ops_, latPercentiles);
    auto write_lat_per = getPercentiles(lat_write_ops_, latPercentiles);
    auto open_lat_per = getPercentiles(lat_open_ops_, latPercentiles);

    perf_summary["read"]["qps"] = read_qps_.toYaml();
    perf_summary["read"]["BW(KiB/s)"] = read_bw_.toYaml();
    perf_summary["read"]["latency(microseconds)"] = PerfResult<long
        long>::calcPerfResult(lat_read_ops_).toYaml();
    perf_summary["read"]["latency(microseconds)"]["percentiles"] =
        percentilesToYaml<long long>(read_lat_per);

    perf_summary["write"]["qps"] = write_qps_.toYaml();
    perf_summary["write"]["BW(KiB/s)"] = write_bw_.toYaml();
    perf_summary["write"]["latency(microseconds)"] = PerfResult<long
        long>::calcPerfResult(lat_write_ops_).toYaml();
    perf_summary["write"]["latency(microseconds)"]["percentiles"] =
        percentilesToYaml<long long>(write_lat_per);

    perf_summary["open"]["qps"] = write_qps_.toYaml();
    perf_summary["open"]["latency (microseconds)"] = PerfResult<long
        long>::calcPerfResult(lat_open_ops_).toYaml();
    perf_summary["open"]["latency(microseconds)"]["percentiles"] =
        percentilesToYaml<long long>(open_lat_per);
    return perf_summary;
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

template <typename T>
std::vector<std::pair<double, T>> PerfCounter::getPercentiles(
    std::vector<T>& vec, std::vector<double>& per) {
    std::sort(vec.begin(), vec.end());
    std::vector<std::pair<double, T>> results;
    for (double percentile : per) {
        int index = static_cast<int>(std::ceil((percentile / 100) * vec.size()))
                    - 1;
        index = (index > 0) ? index : 0;
        results.push_back({percentile, vec[index]});
    }
    return results;
}

template <typename T>
YAML::Node PerfCounter::percentilesToYaml(
    const std::vector<std::pair<double, T>>& pairs) {
    YAML::Node per;
    for (auto pair : pairs) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << pair.first;
        per[oss.str() + "%"] = pair.second;
    }
    return per;
}
