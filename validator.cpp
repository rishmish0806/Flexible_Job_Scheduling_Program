#include <bits/stdc++.h>
using namespace std;
#include "validator.h"

bool validator::validate(FinalSchedule* final_sch, Schedule* sch) {
    errors.clear();
    makespan = 0;

    int machines = sch->machines;
    int jobs     = sch->jobs_count;
    auto original_schedule = sch->job_data;

    vector<unordered_map<int, int>> mp(jobs);
    vector<int> last_op(jobs, -1);
    vector<int> job_ready(jobs, 0);
    job_completion_times.assign(jobs, 0);
    machine_schedules.assign(machines, {});

    auto vec = final_sch->finalSchedule;

    sort(vec.begin(), vec.end(), [](const vector<int>& a, const vector<int>& b) {
        if (a.size() < 5 || b.size() < 5) return a.size() < b.size();
        return a[3] < b[3];
    });

    for (auto v : vec) {
        if (v.size() != 5) { errors.push_back("Malformed schedule row (expected 5 fields)"); return false; }

        int curr_job     = v[0];
        int curr_op      = v[1];
        int curr_machine = v[2];
        int start_time   = v[3];
        int end_time     = v[4];

        if (curr_job < 0 || curr_job >= jobs) {
            errors.push_back("Invalid job id " + to_string(curr_job)); return false;
        }
        if (curr_op < 0 || curr_op >= (int)original_schedule[curr_job].size()) {
            errors.push_back("Invalid operation id " + to_string(curr_op) +
                             " for Job " + to_string(curr_job)); return false;
        }
        if (curr_machine < 0 || curr_machine >= machines) {
            errors.push_back("Invalid machine id " + to_string(curr_machine)); return false;
        }
        if (start_time < 0) {
            errors.push_back("Negative start time for Job " + to_string(curr_job) +
                             " Op " + to_string(curr_op)); return false;
        }
        if (mp[curr_job][curr_op] != 0) {
            errors.push_back("Duplicate operation: Job " + to_string(curr_job) +
                             " Op " + to_string(curr_op)); return false;
        }
        if (last_op[curr_job] != curr_op - 1) {
            errors.push_back("Precedence/order violation: Job " + to_string(curr_job) +
                             " Op " + to_string(curr_op) + " scheduled after Op " +
                             to_string(last_op[curr_job])); return false;
        }
        if (start_time < job_ready[curr_job]) {
            errors.push_back("Precedence violation: Job " + to_string(curr_job) + " Op " +
                             to_string(curr_op) + " starts at " + to_string(start_time) +
                             " but previous operation ends at " + to_string(job_ready[curr_job]));
            return false;
        }

        int p = -1;
        for (auto &q : original_schedule[curr_job][curr_op]) if (q.first == curr_machine) p = q.second;
        if (p == -1) {
            errors.push_back("Ineligible machine " + to_string(curr_machine) + " for Job " +
                             to_string(curr_job) + " Op " + to_string(curr_op)); return false;
        }
        if (end_time != start_time + p) {
            errors.push_back("Wrong duration: Job " + to_string(curr_job) + " Op " +
                             to_string(curr_op) + " on Machine " + to_string(curr_machine) +
                             " has [" + to_string(start_time) + ", " + to_string(end_time) +
                             "] but p = " + to_string(p)); return false;
        }

        if (!machine_schedules[curr_machine].empty()) {
            auto &prev = machine_schedules[curr_machine].back();
            if (start_time < prev[4]) {
                errors.push_back("Machine " + to_string(curr_machine) + " contains overlapping operations:");
                errors.push_back("    Job " + to_string(prev[0]) + " Op " + to_string(prev[1]) +
                                 ": [" + to_string(prev[3]) + ", " + to_string(prev[4]) + "]");
                errors.push_back("    Job " + to_string(curr_job) + " Op " + to_string(curr_op) +
                                 ": [" + to_string(start_time) + ", " + to_string(end_time) + "]");
                return false;
            }
        }

        mp[curr_job][curr_op]++;
        last_op[curr_job]++;
        job_ready[curr_job] = end_time;
        job_completion_times[curr_job] = end_time;
        machine_schedules[curr_machine].push_back(v);
        makespan = max(makespan, end_time);
    }

    for (int i = 0; i < jobs; i++) {
        for (int j = 0; j < (int)original_schedule[i].size(); j++) {
            if (mp[i][j] == 0) {
                errors.push_back("Missing operation: Job " + to_string(i) + " Op " + to_string(j));
                return false;
            }
        }
    }

    return true;
}

void validator::report(ostream &os) const {
    if (errors.empty()) {
        os << "VALID   makespan = " << makespan << "\n";
    } else {
        os << "INVALID\n";
        for (const auto &e : errors) os << "  " << e << "\n";
    }
}
