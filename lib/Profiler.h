#ifndef GHOST_PROFILER_H
#define GHOST_PROFILER_H

#include "absl/container/flat_hash_map.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "lib/base.h"

#include <vector>

namespace ghost
{
    class Profiler
    {
    public:
        // 1. kBlocked->kRunnable->kQueued
        // 2. kQueued->kRunnable->kOnCpu
        enum class TaskState
        {
            kCreated,
            kBlocked,
            kRunnable,
            kQueued,
            kOnCpu,
            kYielding,
            kDied,
            unknown
        };
        struct Metric // Record how long it stayed in that state
        {
            absl::Time createdAt;        // created time
            absl::Duration blockTime;    // Blocked state
            absl::Duration runnableTime; // runnable state
            absl::Duration queuedTime;   // Queued state
            absl::Duration onCpuTime;    // OnCPU state
            absl::Duration yieldingTime;
            absl::Time diedAt;
            int64_t preemptCount; // if it's preempted

            TaskState currentState;
            absl::Time stateStarted;

            Metric() : createdAt(absl::Now()), currentState(TaskState::kCreated), stateStarted(_createdAt) {}
            void updateState(const Profiler::TaskState newState);
        };
        void update(Gtid gtid, std::string_view newState);
        void PrintResults();

    private:
        static TaskState getStateFromString(std::string_view state);
        absl::flat_hash_map<int64_t, Metric> metrics;

        struct Result
        {
            Gtid gtid;
            Metric metric;
        };

        std::vector<Profiler::Result> results;
    };
}
#endif