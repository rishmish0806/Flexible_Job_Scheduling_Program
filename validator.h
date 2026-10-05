#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <bits/stdc++.h>
using namespace std;
#include "final_schedule.h"
#include "schedule.h"

class validator {
public:
    vector<string> errors;
    vector<int> job_completion_times;
    vector<vector<vector<int>>> machine_schedules;
    int makespan = 0;

    bool validate(FinalSchedule* final_sch, Schedule* sch);
    void report(ostream &os) const;
};

#endif
