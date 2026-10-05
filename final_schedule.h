#ifndef FINAL_SCHEDULE_H
#define FINAL_SCHEDULE_H

#include <bits/stdc++.h>
using namespace std;

class FinalSchedule {
public:

    vector<vector<int>> finalSchedule;

    int makespan() const {
        int mk = 0;
        for (const auto &row : finalSchedule) mk = max(mk, row[4]);
        return mk;
    }
};

#endif
