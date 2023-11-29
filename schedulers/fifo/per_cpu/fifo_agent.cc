// Copyright 2021 Google LLC
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include <cstdint>
#include <string>
#include <vector>

#include "absl/debugging/symbolize.h"
#include "absl/flags/parse.h"
#include "lib/agent.h"
#include "lib/enclave.h"
#include "schedulers/fifo/per_cpu/fifo_scheduler.h"
#include "Profiler.h"

ABSL_FLAG(std::string, ghost_cpus, "1-5", "cpulist");
ABSL_FLAG(std::string, enclave, "", "Connect to preexisting enclave directory");

namespace ghost {

static void ParseAgentConfig(AgentConfig* config, Profiler *profiler = nullptr) {
  CpuList ghost_cpus =
      MachineTopology()->ParseCpuStr(absl::GetFlag(FLAGS_ghost_cpus));
  CHECK(!ghost_cpus.Empty());

  if(profiler != nullptr){
    config->profiler = profiler;
  }
  Topology* topology = MachineTopology();
  config->topology_ = topology;
  config->cpus_ = ghost_cpus;
  std::string enclave = absl::GetFlag(FLAGS_enclave);
  if (!enclave.empty()) {
    int fd = open(enclave.c_str(), O_PATH);
    CHECK_GE(fd, 0);
    config->enclave_fd_ = fd;
  }
}

}  // namespace ghost

int main(int argc, char* argv[]) {
  absl::InitializeSymbolizer(argv[0]);
  absl::ParseCommandLine(argc, argv);

  ghost::AgentConfig config;
  Profiler profiler;
  ghost::ParseAgentConfig(&config, &profiler);

  printf("Initializing...\n");

  // Using new so we can destruct the object before printing Done
  auto uap = new ghost::AgentProcess<ghost::FullFifoAgent<ghost::LocalEnclave>,
                                     ghost::AgentConfig>(config);
  
  ghost::GhostHelper()->InitCore();
  printf("Initialization complete, ghOSt active.\n");
  // When `stdout` is directed to a terminal, it is newline-buffered. When
  // `stdout` is directed to a non-interactive device (e.g, a Python subprocess
  // pipe), it is fully buffered. Thus, in order for the Python script to read
  // the initialization message as soon as it is passed to `printf`, we need to
  // manually flush `stdout`.
  fflush(stdout);

  ghost::Notification exit;
  ghost::GhostSignals::AddHandler(SIGINT, [&exit](int) {
    static bool first = true;  // We only modify the first SIGINT.

    if (first) {
      exit.Notify();
      first = false;
      return false;  // We'll exit on subsequent SIGTERMs.
    }
    return true;
  });

  // TODO: this is racy - uap could be deleted already
  ghost::GhostSignals::AddHandler(SIGUSR1, [uap](int) {
    uap->Rpc(ghost::FifoScheduler::kDebugRunqueue);
    return false;
  });

  exit.WaitForNotification();

  profiler.PrintResults();

  delete uap;

  printf("\nDone!\n");

  return 0;
}
