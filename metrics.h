#ifndef METRICS_H
#define METRICS_H

#include <bits/stdc++.h>
using namespace std;
#include "schedule.h"
#include "algorithm.h"

inputMetrics computeMetrics(Schedule* sch);
int lowerBound(Schedule* sch);
double machineUtilization(const vector<vector<int>>& rows, int machines, int makespan);
void writeInstance(Schedule* sch, const string& path);

#endif
