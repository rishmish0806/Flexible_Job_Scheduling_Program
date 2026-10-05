#ifndef GENERATOR_H
#define GENERATOR_H

#include <bits/stdc++.h>
using namespace std;
#include "schedule.h"

class generator {
public:
    Schedule* generate(int num_jobs, int num_machines, int target_operations_per_job,
                       int max_ops, int min_ops, float machine_flexibility,
                       int process_time_start, int process_time_end,
                       float process_time_variance, float bottleneck_probability,
                       int seed);
};

#endif
