#ifndef PERFRESULT_H
#define PERFRESULT_H
#include <yaml-cpp/node/convert.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

template <typename T>
class PerfResult
{
public:
    T min;
    T max;
    T avg;
    T stddev;
    long long samples;
    static PerfResult<T> calcPerfResult(const std::vector<T>& vec);
    YAML::Node toYaml();
};

template <typename T>
PerfResult<T> PerfResult<T>::calcPerfResult(const std::vector<T>& vec)
{
    if (vec.size() == 0)
    {
        PerfResult<T> res;
        res.avg = 0;
        res.max = 0;
        res.min = 0;
        res.stddev = 0;
        res.samples = 0;
        return res;
    }
    auto max_it = std::max_element(vec.begin(), vec.end());
    auto min_it = std::min_element(vec.begin(), vec.end());
    T max_val = *max_it;
    T min_val = *min_it;

    T avg = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();

    T sum = 0;
    for (T value : vec)
    {
        sum += std::pow(value - avg, 2);
    }
    T stddev = std::sqrt(sum / vec.size());

    // return results
    PerfResult<T> res;
    res.avg = avg;
    res.max = max_val;
    res.min = min_val;
    res.stddev = stddev;
    res.samples = vec.size();
    return res;
}

template <typename T>
YAML::Node PerfResult<T>::toYaml()
{
    YAML::Node perf;
    perf["avg"] = avg;
    perf["max"] = max;
    perf["min"] = min;
    perf["stddev"] = stddev;
    perf["samples"] = samples;
    return std::move(perf);
}


using BW_perf = PerfResult<long long>;
using QPS_perf = PerfResult<int>;


#endif //PERFRESULT_H
