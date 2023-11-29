#include "lib/Profiler.h"

void Profiler::Metric::updateState(const Profiler::TaskState &newState)
{
  absl::Time currentTime = absl::Now();
  absl::Duration d = currentTime - stateStarted;
  switch (currentState)
  {
  case TaskState::kBlocked:
    blockedTime += d;
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
  TaskState state = getStateFromString(newState);

  if (metrics.count(gtid) == 0)
    metrics.insert({gtid, Metric(absl::Now())});
  metrics[gtid].updateTime(newState);

  if (state == TaskState::kDied)
  {
    results.emplace_back(Profiler::Result{gtid, metrics[gtid]});
    metrics.erase(gtid);
  }
}

void Profiler::PrintResults()
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
  fprintf(stdout, "=================================\n");

  for (auto &res : results)
  {
    fprintf(stdout, "Ghost Thread Id: %d\n", res.gtid);
    //    time_t unixTime = absl::ToUnixSeconds(currentTime);
    fprintf(stdout, "BlockTime: %" PRId64 "\nRunnableTime: %" PRId64 "\nQueuedTime: %" PRId64 "\nonCpuTime: %" PRId64 "\nyieldingTime: %" PRId64 "\n",
            res.metric.blockTime, res.metric.runnableTime, res.metric.queuedTime, res.metric.onCpuTime, res.metric.yieldingTime);
    fprintf(stdout, "CreatedAt: %lld, DiedAt: %lld\n", absl::ToUnixSeconds(res.metric.createdAt), absl::ToUnixSeconds(res.metric.diedAt));
    fprintf(stdout, "---------------------------------\n");
  }

  fprintf(stdout, "=================================\n");
}

Profiler::TaskState Profiler::getStateFromString(std::string_view state)
{
  if (state == "Blocked")
    return Profiler::TaskState::kBlocked;
  else if (state == "Runnable")
    return Profiler::TaskState::kRunnable;
  else if (state == "Queued")
    return Profiler::TaskState::kQueued;
  else if (state == "onCpu")
    return Profiler::TaskState::kOnCpu;
  else if (state == "yielding")
    return Profiler::TaskState::kYielding;
  else
  {
    fprintf(stderr, "Task state is unknown(%s)\n", state);
    return Profiler::TaskState::unknown;
  }
}