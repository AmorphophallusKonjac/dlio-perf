#include "PerfCounter.h"

#include <ConfigManager.h>
#include <mpi.h>
#include <spdlog/spdlog.h>

#include <iomanip>
#include <map>
#include <mutex>

void PerfCounter::addOperation(OperationTy ty,
                               std::chrono::steady_clock::time_point start_time,
                               std::chrono::steady_clock::time_point end_time,
                               long long size) {
    std::lock_guard lock_guard(ops_mutex_);
    ops_.push_back({start_time, end_time,
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        end_time - start_time),
                    ty, size});
}

YAML::Node PerfCounter::getPerfResult(std::vector<OperationRefInfo>& ops) {
    YAML::Node perf_summary;
    std::vector<OperationRefInfo> read_ops_, write_ops_, open_ops_;
    std::vector<long long> lat_read_ops_, lat_write_ops_, lat_open_ops_;
    for (auto op : ops) {
        switch (op.ty) {
            case OPEN:
                open_ops_.push_back(op);
                lat_open_ops_.push_back(op.latency);
                break;
            case WRITE:
                write_ops_.push_back(op);
                lat_write_ops_.push_back(op.latency);
                break;
            case READ:
                read_ops_.push_back(op);
                lat_read_ops_.push_back(op.latency);
                break;
            default:
                throw std::runtime_error("error op");
        }
    }
    QPS_perf read_qps_, write_qps_, open_qps_;
    BW_perf read_bw_, write_bw_, open_bw_;
    calcQpsAndBw(read_ops_, read_qps_, read_bw_);
    calcQpsAndBw(write_ops_, write_qps_, write_bw_);
    calcQpsAndBw(open_ops_, open_qps_, open_bw_);
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

    perf_summary["open"]["qps"] = open_qps_.toYaml();
    perf_summary["open"]["latency (microseconds)"] = PerfResult<long
        long>::calcPerfResult(lat_open_ops_).toYaml();
    perf_summary["open"]["latency(microseconds)"]["percentiles"] =
        percentilesToYaml<long long>(open_lat_per);
    return perf_summary;
}

void PerfCounter::setRefTimePoint(
    std::chrono::steady_clock::time_point start_time_point,
    std::chrono::steady_clock::time_point end_time_point) {
    start_time_point_ = start_time_point;
    end_time_point_ = end_time_point;
}

void PerfCounter::preprocess() {
    for (auto op : ops_) {
        long long start_second = std::chrono::duration_cast<
            std::chrono::seconds>(op.start_time - start_time_point_).count();
        long long end_second = std::chrono::duration_cast<std::chrono::seconds>(
            op.end_time - start_time_point_).count();
        long long latency = op.latency.count() / 1000;
        ref_ops_.push_back({start_second, end_second, latency, op.size, op.ty});
    }
    if (ConfigManager::getInstance().slave_id_ == 0) {
        for (auto ref_op : ref_ops_) {
            all_rank_ref_ops.push_back(ref_op);
        }
        for (int i = 1; i < ConfigManager::getInstance().slave_num_; ++i) {
            int len;
            MPI_Recv(&len, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            auto* recv_buffer = new long long[len * 5];
            MPI_Recv(recv_buffer, len * 5, MPI_LONG_LONG, i,
                     0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0, k = 0; j < len; ++j) {
                all_rank_ref_ops.push_back({recv_buffer[k], recv_buffer[k + 1],
                                            recv_buffer[k + 2],
                                            recv_buffer[k + 3],
                                            recv_buffer[k + 4]});
                k += 5;
            }
            delete[] recv_buffer;
        }
    } else {
        int len = ref_ops_.size();
        auto* send_buffer = new long long[len * 5];
        for (int i = 0, j = 0; i < len; ++i) {
            send_buffer[j++] = ref_ops_[i].ref_start_time;
            send_buffer[j++] = ref_ops_[i].ref_end_time;
            send_buffer[j++] = ref_ops_[i].latency;
            send_buffer[j++] = ref_ops_[i].size;
            send_buffer[j++] = ref_ops_[i].ty;
        }
        MPI_Send(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(send_buffer, len * 5, MPI_LONG_LONG, 0, 0,
                 MPI_COMM_WORLD);
        delete[] send_buffer;
    }
}

YAML::Node PerfCounter::perfThisRank() {
    return getPerfResult(ref_ops_);
}

YAML::Node PerfCounter::perfAllRank() {
    return getPerfResult(all_rank_ref_ops);
}

void PerfCounter::calcQpsAndBw(std::vector<OperationRefInfo>& ops,
                               QPS_perf& qps,
                               BW_perf& bw) {
    std::map<long long, std::pair<int, long long>> results;
    std::vector<int> qps_vec;
    std::vector<long long> bw_vec;
    for (auto& op : ops) {
        long long start_second = op.ref_start_time;
        long long end_second = op.ref_end_time;
        ++results[start_second].first;
        long long bandwidth_per_second =
            op.size / (end_second - start_second + 1);
        for (long long second = start_second; second <= end_second; ++second) {
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
        if (index < vec.size())
            results.push_back({percentile, vec[index]});
        else
            results.push_back({percentile, 0});
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
