#include <bits/stdc++.h>
using namespace std;
#include "metrics.h"

inputMetrics computeMetrics(Schedule* sch) {
    inputMetrics m;
    int jobs = sch->jobs_count, machines = sch->machines;
    auto &d = sch->job_data;

    int total_ops = 0;
    long long sum_p = 0; long long cnt_p = 0;
    int mn = INT_MAX, mx = INT_MIN;
    long long sum_elig = 0;
    double harmonic_denom = 0.0;
    vector<double> expected_load(machines, 0.0);

    for (int j = 0; j < jobs; j++) {
        total_ops += d[j].size();
        for (int k = 0; k < (int)d[j].size(); k++) {
            int e = d[j][k].size();
            sum_elig += e;
            harmonic_denom += 1.0 / (double)e;
            for (auto &pr : d[j][k]) {
                sum_p += pr.second; cnt_p++;
                mn = min(mn, pr.second); mx = max(mx, pr.second);
                expected_load[pr.first] += (double)pr.second / (double)e;
            }
        }
    }

    m.total_operations = total_ops;
    m.operations_per_job = jobs ? (float)total_ops / (float)jobs : 0.f;
    m.minm_operation_time = (mn == INT_MAX ? 0 : mn);
    m.maxm_operation_time = (mx == INT_MIN ? 0 : mx);
    m.average_procssing_time = cnt_p ? (float)((double)sum_p / (double)cnt_p) : 0.f;

    double var = 0.0;
    for (int j = 0; j < jobs; j++)
        for (int k = 0; k < (int)d[j].size(); k++)
            for (auto &pr : d[j][k]) {
                double diff = pr.second - m.average_procssing_time;
                var += diff * diff;
            }
    m.variance_in_processing_time = cnt_p ? (float)(var / (double)cnt_p) : 0.f;

    m.average_elegible_machines_per_operation = total_ops ? (float)((double)sum_elig / total_ops) : 0.f;
    m.harmonic_mean_of_the_eligible_machines_of_the_operation =
        (harmonic_denom > 0 ? (float)((double)total_ops / harmonic_denom) : 0.f);

    double mean_load = 0.0;
    for (double l : expected_load) mean_load += l;
    mean_load /= max(1, machines);
    double lv = 0.0;
    for (double l : expected_load) lv += (l - mean_load) * (l - mean_load);
    m.load_distribution_variance = (float)(lv / max(1, machines));

    return m;
}

int lowerBound(Schedule* sch) {
    int jobs = sch->jobs_count, machines = sch->machines;
    auto &d = sch->job_data;

    int job_bound = 0;
    for (int j = 0; j < jobs; j++) {
        int s = 0;
        for (int k = 0; k < (int)d[j].size(); k++) {
            int best = INT_MAX;
            for (auto &pr : d[j][k]) best = min(best, pr.second);
            s += best;
        }
        job_bound = max(job_bound, s);
    }

    vector<long long> forced(machines, 0);
    for (int j = 0; j < jobs; j++)
        for (int k = 0; k < (int)d[j].size(); k++)
            if (d[j][k].size() == 1) forced[d[j][k][0].first] += d[j][k][0].second;
    long long machine_bound = 0;
    for (int i = 0; i < machines; i++) machine_bound = max(machine_bound, forced[i]);

    long long total_min_work = 0;
    for (int j = 0; j < jobs; j++)
        for (int k = 0; k < (int)d[j].size(); k++) {
            int best = INT_MAX;
            for (auto &pr : d[j][k]) best = min(best, pr.second);
            total_min_work += best;
        }
    long long work_bound = (machines > 0 ? (total_min_work + machines - 1) / machines : 0);

    return (int)max(max<long long>(job_bound, machine_bound), work_bound);
}

double machineUtilization(const vector<vector<int>>& rows, int machines, int makespan) {
    if (machines <= 0 || makespan <= 0) return 0.0;
    long long busy = 0;
    for (auto &r : rows) busy += (r[4] - r[3]);
    return (double)busy / ((double)machines * (double)makespan);
}

void writeInstance(Schedule* sch, const string& path) {
    ofstream f(path);
    if (!f) return;
    f << "# instance_class=" << sch->instance_class << " seed=" << sch->seed << "\n";
    f << sch->jobs_count << " " << sch->machines << "\n";
    for (int j = 0; j < sch->jobs_count; j++) {
        f << sch->job_data[j].size();
        for (int k = 0; k < (int)sch->job_data[j].size(); k++) {
            f << " " << sch->job_data[j][k].size();
            for (auto &pr : sch->job_data[j][k]) f << " " << pr.first << " " << pr.second;
        }
        f << "\n";
    }
}
