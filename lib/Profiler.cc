#include "lib/Profiler.h"
#include <inttypes.h>
#include <stdio.h>

namespace ghost
{
  void Profiler::Metric::updateState(const Profiler::TaskState newState)
  {
    absl::Time currentTime = absl::Now();
    absl::Duration d = currentTime - stateStarted;
    switch (currentState)
    {
    case TaskState::kBlocked:
      blockTime += d;
      break;
    case TaskState::kRunnable:
      runnableTime += d;
      break;
    case TaskState::kQueued:
      queuedTime += d;
      break;
    case TaskState::kOnCpu:
      onCpuTime += d;
      break;
    case TaskState::kYielding:
      yieldingTime += d;
      break;
    default:
      break;
    }
    currentState = newState;
  }

  void Profiler::update(Gtid gtid, std::string_view newState)
  {
    m.lock();
    TaskState state = getStateFromString(newState);
    int64_t rawTid = gtid.id();

    metrics[rawTid].updateState(getStateFromString(newState));

    if (state == TaskState::kDied)
    {
      printf("Thread Died %ld, %ld\n", rawTid, metrics[rawTid].onCpuTime);
      results.push_back(Profiler::Result{rawTid, metrics[rawTid]});
      metrics.erase(rawTid);
    }
    m.unlock();
  }

  void Profiler::PrintResults(FILE *to)
  {
    //   absl::Time createdAt;        // created time
    // absl::Duration blockTime;    // Blocked state
    // absl::Duration runnableTime; // runnable state
    // absl::Duration queuedTime;   // Queued state
    // absl::Duration onCpuTime;    // OnCPU state
    // absl::Duration yieldingTime;
    // absl::Time diedAt;
    // int64_t preemptCount; // if it's preempted

    // TaskState currentState;
    // absl::Time stateStarted;
    fprintf(to, "=================================\n");
    fprintf(to, "=============== Metrics %d ==================\n", metrics.size());
    for (auto &[gtid, m] : metrics)
    {
      fprintf(to, "Ghost Thread Id: %ld\n", gtid);
      //    time_t unixTime = absl::ToUnixSeconds(currentTime);
      fprintf(to, "BlockTime: %" PRId64 "\nRunnableTime: %" PRId64 "\nQueuedTime: %" PRId64 "\nonCpuTime: %" PRId64 "\nyieldingTime: %" PRId64 "\n",
              absl::ToInt64Nanoseconds(m.blockTime), absl::ToInt64Nanoseconds(m.runnableTime), absl::ToInt64Nanoseconds(m.queuedTime),
              absl::ToInt64Nanoseconds(m.onCpuTime), absl::ToInt64Nanoseconds(m.yieldingTime));
      fprintf(to, "CreatedAt: %" PRId64 ", DiedAt: %" PRId64 "\n", absl::ToUnixSeconds(m.createdAt), absl::ToUnixSeconds(m.diedAt));
      fprintf(to, "---------------------------------\n");
    }
    fprintf(to, "=============== Results %d ==================\n", results.size());
    for (auto &res : results)
    {
      fprintf(to, "Ghost Thread Id: %ld\n", res.tid);
      //    time_t unixTime = absl::ToUnixSeconds(currentTime);
      fprintf(to, "BlockTime: %" PRId64 "\nRunnableTime: %" PRId64 "\nQueuedTime: %" PRId64 "\nonCpuTime: %" PRId64 "\nyieldingTime: %" PRId64 "\n",
              absl::ToInt64Nanoseconds(res.metric.blockTime), absl::ToInt64Nanoseconds(res.metric.runnableTime), absl::ToInt64Nanoseconds(res.metric.queuedTime),
              absl::ToInt64Nanoseconds(res.metric.onCpuTime), absl::ToInt64Nanoseconds(res.metric.yieldingTime));
      fprintf(to, "CreatedAt: %" PRId64 ", DiedAt: %" PRId64 "\n", absl::ToUnixSeconds(res.metric.createdAt), absl::ToUnixSeconds(res.metric.diedAt));
      fprintf(to, "---------------------------------\n");
    }

    fprintf(to, "=================================\n");
  }

  Profiler::TaskState Profiler::getStateFromString(std::string_view state)
  {
    if (state == "Blocked")
      return Profiler::TaskState::kBlocked;
    else if (state == "Runnable")
      return Profiler::TaskState::kRunnable;
    else if (state == "Queued")
      return Profiler::TaskState::kQueued;
    else if (state == "OnCpu")
      return Profiler::TaskState::kOnCpu;
    else if (state == "yielding")
      return Profiler::TaskState::kYielding;
    else if (state == "Died")
      return Profiler::TaskState::kDied;
    else
    {
      fprintf(stderr, "Task state is unknown(%s)\n", state.data());
      return Profiler::TaskState::unknown;
    }
  }
}