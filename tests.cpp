#include <bits/stdc++.h>
using namespace std;
#include "schedule.h"
#include "final_schedule.h"
#include "algorithm.h"
#include "validator.h"

static Schedule* tinyInstance() {
    Schedule* s = new Schedule();
    s->machines = 2; s->jobs_count = 2; s->instance_class = "unit_test"; s->seed = 0;
    s->job_data = {
        { {{0,3},{1,5}}, {{1,4}} },
        { {{0,2}},       {{0,6},{1,7}} }
    };
    return s;
}

static void check(const string& name, FinalSchedule fs, Schedule* sch, bool expect_valid) {
    validator v;
    bool ok = v.validate(&fs, sch);
    cout << left << setw(34) << name << (ok ? "VALID  " : "INVALID")
         << (ok == expect_valid ? "   [test passed]" : "   [TEST FAILED]") << "\n";
    if (!ok && !v.errors.empty()) cout << "        -> " << v.errors[0] << "\n";
}

int main() {
    Schedule* sch = tinyInstance();

    Algorithm alg;
    FinalSchedule* good = alg.fnl(sch);
    check("solver output", *good, sch, true);

    FinalSchedule f;
    f.finalSchedule = {{0,0,0,0,3},{0,1,1,3,7},{1,0,0,3,5},{1,1,0,5,11}};
    check("hand-written valid schedule", f, sch, true);

    f.finalSchedule = {{0,0,1,0,5},{0,1,1,5,9},{1,0,0,0,2},{1,1,0,2,8}};
    f.finalSchedule[0][2] = 1;
    check("alternate valid assignment", f, sch, true);

    f.finalSchedule = {{0,0,1,0,4},{0,1,1,4,8},{1,0,0,0,2},{1,1,0,2,8}};
    check("wrong duration", f, sch, false);

    f.finalSchedule = {{0,0,0,0,3},{0,1,0,3,7},{1,0,0,3,5},{1,1,0,5,11}};
    check("ineligible machine (J0 Op1 on M0)", f, sch, false);

    f.finalSchedule = {{0,0,0,0,3},{0,1,1,2,6},{1,0,0,3,5},{1,1,0,5,11}};
    check("precedence violation", f, sch, false);

    f.finalSchedule = {{0,0,0,0,3},{0,1,1,3,7},{1,0,0,1,3},{1,1,0,5,11}};
    check("machine overlap on M0", f, sch, false);

    f.finalSchedule = {{0,0,0,0,3},{0,1,1,3,7},{1,0,0,3,5}};
    check("missing operation", f, sch, false);

    f.finalSchedule = {{0,0,0,0,3},{0,0,1,3,8},{0,1,1,8,12},{1,0,0,3,5},{1,1,0,5,11}};
    check("duplicate operation", f, sch, false);

    f.finalSchedule = {{0,0,0,-2,1},{0,1,1,3,7},{1,0,0,3,5},{1,1,0,5,11}};
    check("negative start time", f, sch, false);

    f.finalSchedule = {{0,0,5,0,3},{0,1,1,3,7},{1,0,0,3,5},{1,1,0,5,11}};
    check("invalid machine id", f, sch, false);

    f.finalSchedule = {{0,9,0,0,3},{0,1,1,3,7},{1,0,0,3,5},{1,1,0,5,11}};
    check("invalid operation id", f, sch, false);

    f.finalSchedule = {{0,0,0,0}};
    check("malformed row", f, sch, false);
    return 0;
}
