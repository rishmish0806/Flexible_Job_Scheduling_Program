#include <bits/stdc++.h>
using namespace std;
#include "schedule.h"
#include "final_schedule.h"
#include "generator.h"
#include "algorithm.h"
#include "validator.h"
#include "metrics.h"

struct GenParams {
    string name;
    int num_jobs, num_machines, target_ops, max_ops, min_ops;
    float flexibility;
    int pt_start, pt_end;
    float pt_variance;
    float bottleneck_prob;
};

static vector<GenParams> instanceClasses() {
    return {

        {"average",               10,  5,  5,  8,  3, 0.40f,   5, 20,  4.0f, 0.00f},
        {"easy",                   5,  8,  3,  5,  2, 0.80f,   5, 15,  2.0f, 0.00f},
        {"hard",                  25,  5,  8, 12,  5, 0.25f,   5, 40, 10.0f, 0.15f},
        {"extreme",               40,  4, 12, 16,  8, 0.15f,   5, 60, 20.0f, 0.30f},
        {"bottleneck_heavy",      15,  6,  6,  9,  4, 0.50f,   5, 25,  5.0f, 0.55f},
        {"high_flexibility",      15,  6,  6,  9,  4, 0.95f,   5, 25,  5.0f, 0.00f},
        {"low_flexibility",       15,  6,  6,  9,  4, 0.05f,   5, 25,  5.0f, 0.00f},
        {"unbalanced",            20,  6,  6, 14,  2, 0.30f,   5, 30, 12.0f, 0.35f},
        {"high_variance",         15,  5,  6,  9,  4, 0.40f,   5, 50, 25.0f, 0.00f},
        {"many_jobs_few_mach",    30,  3,  5,  8,  3, 0.50f,   5, 20,  4.0f, 0.00f},
        {"few_jobs_many_mach",     4, 12,  6,  9,  4, 0.50f,   5, 20,  4.0f, 0.00f},
    };
}

static Schedule* makeInstance(const string& name, int machines,
                              const vector<vector<vector<pair<int,int>>>>& data) {
    Schedule* s = new Schedule();
    s->machines = machines;
    s->jobs_count = data.size();
    s->job_data = data;
    s->instance_class = name;
    s->seed = -1;
    return s;
}

static vector<Schedule*> edgeCases() {
    vector<Schedule*> v;

    {
        vector<vector<vector<pair<int,int>>>> d;
        for (int j = 0; j < 5; j++) { vector<vector<pair<int,int>>> job;
            for (int k = 0; k < 4; k++) job.push_back({{0, 3 + k}});
            d.push_back(job); }
        v.push_back(makeInstance("edge_single_machine", 1, d));
    }

    {
        vector<vector<pair<int,int>>> job;
        for (int k = 0; k < 8; k++) job.push_back({{0, 5}, {1, 3}, {2, 9}});
        v.push_back(makeInstance("edge_one_job", 3, {job}));
    }

    {
        vector<vector<vector<pair<int,int>>>> d;
        for (int j = 0; j < 12; j++) {
            vector<vector<pair<int,int>>> job;
            job.push_back({{0, 4}});
            job.push_back({{1, 6}, {2, 6}, {3, 6}});
            d.push_back(job);
        }
        v.push_back(makeInstance("edge_one_bottleneck", 4, d));
    }

    {
        vector<vector<vector<pair<int,int>>>> d;
        for (int j = 0; j < 6; j++) {
            vector<vector<pair<int,int>>> job;
            for (int k = 0; k < 3; k++) job.push_back({{0, 1}, {1, 10000}});
            d.push_back(job);
        }
        v.push_back(makeInstance("edge_extreme_gap", 2, d));
    }

    {
        vector<vector<vector<pair<int,int>>>> d;
        for (int j = 0; j < 8; j++) {
            vector<vector<pair<int,int>>> job;
            for (int k = 0; k < 4; k++) job.push_back({{0, 5}, {1, 20}, {2, 20}});
            d.push_back(job);
        }
        v.push_back(makeInstance("edge_machine_advantage", 3, d));
    }

    {
        vector<vector<vector<pair<int,int>>>> d;
        for (int j = 0; j < 10; j++) {
            vector<vector<pair<int,int>>> job;
            for (int k = 0; k < 5; k++) job.push_back({{0, 7}, {1, 7}, {2, 7}, {3, 7}});
            d.push_back(job);
        }
        v.push_back(makeInstance("edge_identical_times", 4, d));
    }

    {
        vector<vector<vector<pair<int,int>>>> d;
        vector<vector<pair<int,int>>> longjob;
        for (int k = 0; k < 40; k++) longjob.push_back({{0, 10}, {1, 12}});
        d.push_back(longjob);
        for (int j = 0; j < 6; j++) {
            vector<vector<pair<int,int>>> job;
            for (int k = 0; k < 2; k++) job.push_back({{0, 2}, {1, 2}});
            d.push_back(job);
        }
        v.push_back(makeInstance("edge_long_chain", 2, d));
    }

    {
        vector<vector<vector<pair<int,int>>>> d;
        for (int j = 0; j < 20; j++) {
            vector<vector<pair<int,int>>> job;
            for (int k = 0; k < 15; k++) job.push_back({{0, 1}, {1, 1}, {2, 2}});
            d.push_back(job);
        }
        v.push_back(makeInstance("edge_many_short_ops", 3, d));
    }

    {
        vector<vector<vector<pair<int,int>>>> d;
        for (int j = 0; j < 10; j++) {
            vector<vector<pair<int,int>>> job;
            for (int k = 0; k < 5; k++) job.push_back({{(j + k) % 4, 5 + ((j * k) % 7)}});
            d.push_back(job);
        }
        v.push_back(makeInstance("edge_zero_flexibility", 4, d));
    }
    return v;
}

int main() {
    const vector<int> seeds = {1, 2, 3};
    const vector<float> weights = {0.0f, 0.3f, 0.6f, 1.0f};

    system("mkdir -p results instances");

    ofstream csv("results/results.csv");
    csv << "instance_class,seed,num_jobs,num_machines,total_operations,operations_per_job,"
        << "min_operation_time,max_operation_time,average_processing_time,variance_in_processing_time,"
        << "average_eligible_machines_per_operation,harmonic_mean_eligible_machines,"
        << "load_distribution_variance,penalty_on_waittime,penalty_on_average_time,"
        << "valid,final_makespan,lower_bound,lb_gap_percent,machine_utilization,runtime_ms\n";
    csv << fixed << setprecision(4);

    generator gen;
    validator val;

    vector<Schedule*> all;
    vector<string> labels;

    for (const auto &c : instanceClasses())
        for (int s : seeds) {
            Schedule* sch = gen.generate(c.num_jobs, c.num_machines, c.target_ops, c.max_ops,
                                         c.min_ops, c.flexibility, c.pt_start, c.pt_end,
                                         c.pt_variance, c.bottleneck_prob, s);
            sch->instance_class = c.name;
            all.push_back(sch); labels.push_back(c.name);
        }
    for (auto* e : edgeCases()) { all.push_back(e); labels.push_back(e->instance_class); }

    map<string, map<pair<float,float>, vector<long long>>> agg;
    int runs = 0, invalid = 0;

    for (size_t idx = 0; idx < all.size(); idx++) {
        Schedule* sch = all[idx];
        writeInstance(sch, "instances/" + sch->instance_class + "_seed" + to_string(sch->seed) + ".txt");

        inputMetrics m = computeMetrics(sch);
        int lb = lowerBound(sch);

        for (float w_wait : weights) {
            for (float w_avg : weights) {
                Algorithm alg;
                alg.penalty_on_waittime = w_wait;
                alg.penalty_on_average_time = w_avg;

                auto t0 = chrono::high_resolution_clock::now();
                FinalSchedule* fs = alg.fnl(sch);
                auto t1 = chrono::high_resolution_clock::now();
                double ms = chrono::duration<double, milli>(t1 - t0).count();

                bool ok = val.validate(fs, sch);
                int makespan = fs->makespan();
                double util = machineUtilization(fs->finalSchedule, sch->machines, makespan);
                double gap = (lb > 0 ? 100.0 * (makespan - lb) / (double)lb : 0.0);

                if (!ok) { invalid++; cerr << "[INVALID] " << sch->instance_class
                                           << " seed " << sch->seed << " w=(" << w_wait << ","
                                           << w_avg << ")\n"; val.report(cerr); }

                csv << sch->instance_class << "," << sch->seed << "," << sch->jobs_count << ","
                    << sch->machines << "," << m.total_operations << "," << m.operations_per_job << ","
                    << m.minm_operation_time << "," << m.maxm_operation_time << ","
                    << m.average_procssing_time << "," << m.variance_in_processing_time << ","
                    << m.average_elegible_machines_per_operation << ","
                    << m.harmonic_mean_of_the_eligible_machines_of_the_operation << ","
                    << m.load_distribution_variance << "," << w_wait << "," << w_avg << ","
                    << (ok ? "VALID" : "INVALID") << "," << makespan << "," << lb << ","
                    << gap << "," << util << "," << ms << "\n";

                agg[sch->instance_class][{w_wait, w_avg}].push_back(makespan);
                runs++;
                delete fs;
            }
        }
    }
    csv.close();

    ofstream best("results/best_weights.csv");
    best << "instance_class,best_penalty_on_waittime,best_penalty_on_average_time,"
         << "mean_makespan,worst_mean_makespan,improvement_percent\n";
    best << fixed << setprecision(3);

    cout << "\n=== Best penalty weights per instance class (mean makespan over seeds) ===\n";
    cout << left << setw(24) << "class" << setw(10) << "w_wait" << setw(10) << "w_avg"
         << setw(14) << "mean mkspan" << setw(14) << "worst mean" << "gain%\n";
    for (auto &kv : agg) {
        double bestMean = 1e18, worstMean = -1; pair<float,float> bw{0,0};
        for (auto &wk : kv.second) {
            double s = 0; for (long long v : wk.second) s += v;
            double mean = s / wk.second.size();
            if (mean < bestMean) { bestMean = mean; bw = wk.first; }
            worstMean = max(worstMean, mean);
        }
        double gain = (worstMean > 0 ? 100.0 * (worstMean - bestMean) / worstMean : 0.0);
        best << kv.first << "," << bw.first << "," << bw.second << "," << bestMean << ","
             << worstMean << "," << gain << "\n";
        cout << left << setw(24) << kv.first << setw(10) << bw.first << setw(10) << bw.second
             << setw(14) << bestMean << setw(14) << worstMean << fixed << setprecision(2) << gain << "\n";
    }
    best.close();

    cout << "\n" << runs << " runs over " << all.size() << " instances, "
         << invalid << " invalid schedules.\n"
         << "Wrote results/results.csv, results/best_weights.csv and instances/*.txt\n";
    return 0;
}
