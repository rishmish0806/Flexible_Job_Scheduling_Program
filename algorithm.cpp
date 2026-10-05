#include <bits/stdc++.h>
using namespace std;
#include "algorithm.h"
#include "final_schedule.h"
#include "schedule.h"
#define pb push_back

FinalSchedule* Algorithm::fnl(Schedule* sch) {
    int jobs     = sch->jobs_count;
    int machines = sch->machines;
    auto schd    = sch->job_data;

    vector<int> machine_occupany(machines, 0);
    vector<vector<int>> final_schd;
    vector<queue<int>> job_queue(jobs);
    vector<vector<float>> mean_process_time(jobs);

    int total_ops = 0;
    for (int i = 0; i < jobs; i++) {
        for (int j = 0; j < (int)schd[i].size(); j++) {
            job_queue[i].push(j);
            total_ops++;
        }
    }

    for (int i = 0; i < jobs; i++) {
        int sz = schd[i].size();
        for (int j = 0; j < sz; j++) {
            int tot_time = 0;
            for (auto &p : schd[i][j]) tot_time += p.second;
            float mean_time = (float)tot_time / (float)schd[i][j].size();
            mean_process_time[i].pb(mean_time);
        }
    }

    vector<int> first_time(jobs, 0);

    int scheduled_ops = 0;
    while (scheduled_ops < total_ops) {
        int min_time = INT_MAX;
        for (int i = 0; i < machines; i++) min_time = min(min_time, machine_occupany[i]);

        bool progress = false;

        for (int i = 0; i < machines; i++) {
            if (machine_occupany[i] > min_time) continue;

            vector<pair<float, pair<int, int>>> vec;
            for (int j = 0; j < jobs; j++) {
                if (job_queue[j].empty()) continue;
                if (first_time[j] > min_time) continue;

                int operation = job_queue[j].front();
                int process_time_of_machine = -1;
                for (auto &p : schd[j][operation]) {
                    if (p.first == i) { process_time_of_machine = p.second; break; }
                }
                if (process_time_of_machine == -1) continue;

                float score = (float)process_time_of_machine
                              - penalty_on_average_time * mean_process_time[j][operation]
                              - penalty_on_waittime * (float)(min_time - first_time[j]);

                vec.pb({score, {process_time_of_machine, j}});
            }

            sort(vec.begin(), vec.end());
            if (vec.empty()) continue;

            int job_to_execute            = vec[0].second.second;
            int time_to_execute_operation = vec[0].second.first;
            int operation_to_execute      = job_queue[job_to_execute].front();

            job_queue[job_to_execute].pop();
            first_time[job_to_execute] = min_time + time_to_execute_operation;
            machine_occupany[i]        = min_time + time_to_execute_operation;

            final_schd.pb({job_to_execute, operation_to_execute, i,
                           min_time, min_time + time_to_execute_operation});
            scheduled_ops++;
            progress = true;
        }

        if (!progress) {

            int next = INT_MAX;
            for (int i = 0; i < machines; i++)
                if (machine_occupany[i] > min_time) next = min(next, machine_occupany[i]);
            for (int j = 0; j < jobs; j++)
                if (!job_queue[j].empty() && first_time[j] > min_time) next = min(next, first_time[j]);
            if (next == INT_MAX) break;
            for (int i = 0; i < machines; i++)
                if (machine_occupany[i] <= min_time) machine_occupany[i] = next;
        }
    }

    FinalSchedule* Schedule_final = new FinalSchedule();
    Schedule_final->finalSchedule = final_schd;
    return Schedule_final;
}
