#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <bits/stdc++.h>
using namespace std;

class Schedule {
public:
    int machines;
    int jobs_count;

    vector<vector<vector<pair<int, int>>>> job_data;

    string instance_class = "unknown";
    int seed = 0;
};

#endif
