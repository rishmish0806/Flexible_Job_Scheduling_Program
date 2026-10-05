#include <bits/stdc++.h>
using namespace std;
#include "generator.h"
#include "schedule.h"

Schedule* generator::generate(int num_jobs, int num_machines, int target_operations_per_job,
                              int max_ops, int min_ops, float machine_flexibility,
                              int process_time_start, int process_time_end,
                              float process_time_variance, float bottleneck_probability,
                              int seed) {

    Schedule* sch = new Schedule();
    sch->machines = num_machines;
    sch->jobs_count = num_jobs;
    sch->seed = seed;
    sch->job_data.assign(num_jobs, vector<vector<pair<int, int>>>());

    mt19937 rng(seed);

    int t = max(0, max_ops - min_ops);
    float p = 0.0f;
    if (t > 0) p = (float)(target_operations_per_job - min_ops) / (float)t;
    p = min(1.0f, max(0.0f, p));
    binomial_distribution<int> ops_gen(t, p);
    uniform_real_distribution<float> flexibility_gen(0.0, 1.0);
    uniform_int_distribution<int> random_machine_gen(0, num_machines - 1);

    float avg_process_time = ((float)(process_time_start) + (float)(process_time_end)) / 2.0;
    normal_distribution<float> gauss_process_time(avg_process_time, process_time_variance);

    int bottleneck_machine = random_machine_gen(rng);

    for (int i = 0; i < num_jobs; i++) {
        int ops = min_ops + ops_gen(rng);
        ops = max(1, ops);
        sch->job_data[i].assign(ops, vector<pair<int, int>>());
        for (int j = 0; j < ops; j++) {

            bool force_bottleneck = (flexibility_gen(rng) < bottleneck_probability);
            int guaranteed_machine = force_bottleneck ? bottleneck_machine : random_machine_gen(rng);

            for (int k = 0; k < num_machines; k++) {

                float prb = flexibility_gen(rng);
                bool eligible = (k == guaranteed_machine) ||
                                (!force_bottleneck && prb < machine_flexibility);
                if (eligible) {
                    float raw_process_time = gauss_process_time(rng);

                    int process_time = max(1, (int)round(raw_process_time));

                    sch->job_data[i][j].push_back({k, process_time});
                }
            }

        }
    }

    return sch;
}
