# Flexible Job-Shop Scheduling (FJSP)

Inter IIT Tech Meet 15.0 — Algo Prepathon

## What problem am I solving?

Imagine a factory. There are some **jobs** to finish and some **machines** to do them on.

- Every job is a list of steps (operations) that must be done **in order**. Step 2 cannot start before step 1 is finished.
- Every step can run on **more than one machine**, and different machines take different amounts of time for the same step.
- A machine can only do **one** step at a time, and once a step starts it cannot be paused.

My job is to decide two things for every step: **which machine** it runs on, and **when** it starts.

The goal is to finish everything as early as possible. The time when the very last job finishes is called the **makespan**, and I want that number to be as small as possible.

## What I built

The project is a full pipeline, not just a scheduler:

```
Generator  →  Instance  →  Algorithm  →  Schedule  →  Validator  →  Makespan  →  CSV results
```

### 1. Generator (`generator.cpp`)

Creates random factory problems. I wrote the random logic myself instead of dumping uniform random numbers.

- **How many steps per job** — a binomial distribution between a minimum and a maximum, so most jobs have a typical number of steps and very short or very long jobs are rare.
- **Which machines can do a step** — every step first gets one **guaranteed machine**, so it can always be done by somebody. Then every other machine is added with probability `machine_flexibility`. A low value makes a rigid factory (few choices), a high value makes a flexible one (many choices).
- **How long a step takes** — a normal (Gaussian) distribution around the middle of the time range. Turning up `processing_time_variance` makes some steps very short and others very long.
- **Bottlenecks** — with probability `bottleneck_probability` a step is forced onto one single congested machine, so everybody has to queue for it.
- **Seed** — every instance can be regenerated exactly from its seed.

Because each step always gets at least one machine and every processing time is at least 1, the generator **can never produce an invalid problem**.

### 2. Algorithm (`algorithm.cpp`)

A greedy dispatching rule. The scheduler keeps a clock, looks at the earliest time a machine is free, and for each free machine asks: *out of all the jobs waiting right now, which one should I take?*

Each candidate gets a score, and the **lowest score wins**:

```
score = processing_time
        − w_avg  × (average time of this step across all its machines)
        − w_wait × (how long this job has already been waiting)
```

In plain words, three things pull the decision:

1. **Processing time** — prefer a step that finishes quickly on this machine.
2. **Average-time term** — if a step is much faster here than on its other machines, this machine has an advantage, so grab it. This stops a fast machine from wasting its speed on an easy step.
3. **Waiting term** — if a job has been sitting idle for a long time, push it forward so it does not get starved.

The two weights `w_wait` and `w_avg` control how much the last two matter. They are the knobs I experiment with.

After choosing, the machine is marked busy until the step finishes, and the job is marked busy too, so its next step cannot start early. If no machine can start anything at the current time, the clock jumps to the next moment when something becomes free.

### 3. Validator (`validator.cpp`)

A completely separate checker. It **does not trust my scheduler**. Given a schedule, it checks all seven rules:

- every step is on a machine that is actually allowed to run it
- the duration matches the instance exactly
- steps of the same job never overlap and stay in order
- no machine runs two steps at the same time
- no negative start times
- no duplicate or missing steps, no unknown IDs
- the schedule is well-formed

If something is wrong it prints a clear message, for example:

```
Machine 5 contains overlapping operations:
    Job 1 Op 3: [10, 18]
    Job 6 Op 1: [15, 22]
```

It also rebuilds the per-machine schedules, the finish time of each job, and the makespan.

`tests.cpp` contains 13 tests that feed the validator deliberately broken schedules (wrong duration, wrong machine, overlap, missing step, and so on) and confirms it catches every one.

### 4. Experiments (`main.cpp`)

This is the part that produces the results.

- **11 random instance classes** — average, easy, hard, extreme, bottleneck-heavy, high flexibility, low flexibility, unbalanced, high variance, many jobs with few machines, few jobs with many machines. Each is run with **3 different seeds**.
- **9 hand-built edge cases** — a single machine, a single job, many jobs fighting over one bottleneck machine, an extreme time gap (1 vs 10000), a machine that is 4× faster than the rest, identical times everywhere, one very long job among short ones, lots of tiny steps, and near-zero flexibility.
- **16 weight settings** — every combination of `w_wait` and `w_avg` from {0, 0.3, 0.6, 1.0}.

That is 42 instances × 16 settings = **672 runs**. Every single run is checked by the validator, and all 672 come back VALID.

`metrics.cpp` computes the numbers that describe each instance (operations per job, average and variance of processing time, average and harmonic mean of eligible machines per step, how unevenly the load is spread) plus a **lower bound** on the best possible makespan, so I can see how far my answer is from what is theoretically possible.

## Results

| File | What is in it |
|---|---|
| `results/results.csv` | One row per run: instance properties, weights, makespan, lower bound, gap, machine utilisation, runtime |
| `results/best_weights.csv` | The best weight pair for each instance class |
| `instances/*.txt` | Every instance that was generated, with its class and seed |

## What I learned (failure analysis)

**1. The weights matter most exactly where the problem is hardest.**
Tuning the weights improves the makespan by 27% on high-variance instances, 19% on both high- and low-flexibility instances, and 17% on hard instances. But on a single machine, a single job, or a clear machine-advantage case it changes nothing at all, because there is no real decision to make. In every crowded case, a **high waiting penalty (0.6–1.0) wins** — starving a job is the most expensive mistake this rule can make.

**2. The scheduler commits to machines too early.**
My loop asks "which job is best for this machine?" and never "which machine is best for this job". So in the single-job test, machine 0 grabs the job at 5 time units before machine 1 (which would take only 3) is even considered. The result is a makespan of 40 against a lower bound of 24 — a 67% gap that **no weight setting can fix**, because the problem is the order the loop visits things in, not the score.

*Proposed improvement (idea only):* score every (job, machine) pair together in one list for the whole time step, instead of looping machine by machine.

**3. Extreme time gaps break it completely.**
In the test where one machine takes 1 unit and the other takes 10000, the slow machine looks "free" at time 0, so it happily grabs a job for 10000 units while the fast machine races through everything else. The gap explodes to over 100,000%.

*Proposed improvement (idea only):* skip a machine when its time for a step is far above the fastest option for that step, unless the job has already waited too long.

**4. Bottleneck instances are easier than they look.**
Bottleneck-heavy and extreme instances come within about 2–3% of the lower bound. When one machine is the obvious constraint there is barely any choice left, so a greedy rule does nearly as well as anything else. The hard cases are the ones with **lots of freedom**, not the ones with little.

## How to run

```bash
make          # builds the solver and the tests
make run      # runs all 672 experiments, writes results/ and instances/
make check    # runs the validator tests
make clean    # removes binaries and generated output
```

Needs g++ with C++17. No external libraries.

## Files

```
fjsp/
├── generator.cpp / .h     # makes random problems
├── schedule.h             # the problem (jobs, machines, times)
├── algorithm.cpp / .h     # my greedy scheduler
├── final_schedule.h       # the answer (job, op, machine, start, end)
├── validator.cpp / .h     # independent correctness checker
├── metrics.cpp / .h       # instance statistics, lower bound, utilisation
├── main.cpp               # runs all the experiments, writes the CSVs
├── tests.cpp              # proves the validator rejects bad schedules
├── results/               # results.csv, best_weights.csv
└── instances/             # every generated instance
```
