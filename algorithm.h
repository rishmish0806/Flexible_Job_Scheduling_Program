#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <bits/stdc++.h>
using namespace std;
#include "schedule.h"
#include "final_schedule.h"

class inputMetrics {
public:
    float operations_per_job = 0;
    int   minm_operation_time = 0;
    int   maxm_operation_time = 0;
    float average_procssing_time = 0;
    float variance_in_processing_time = 0;
    float average_elegible_machines_per_operation = 0;
    float load_distribution_variance = 0;
    float harmonic_mean_of_the_eligible_machines_of_the_operation = 0;
    int   total_operations = 0;
};

class Algorithm {
public:

    float penalty_on_waittime = 0.3f;
    float penalty_on_average_time = 0.3f;

    FinalSchedule* fnl(Schedule* sch);
};

#endif
