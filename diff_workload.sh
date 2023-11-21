arr=("0.005" "0.01" "0.02" "0.04" "0.1")

# 0.005 0.025 0.05 0.07 0.08 0.09 0.1

for workload in ${arr[@]}; do
	echo "RUN $workload"
	if ! test -f ./diff_policy/ghost_$workload.out.done; then
		sudo bazel-bin/experiments/scripts/shinjuku.par ghost $workload 30us 10000 151000 10000 agent_shinjuku 15s | tee ./diff_policy/ghost_$workload.out
		mv ./diff_policy/ghost_$workload.out ./diff_policy/ghost_$workload.out.done
		echo "Done ghost_$workload.out"
	fi

	if ! test -f ./diff_policy/cfs_$workload.out.done; then
		sudo bazel-bin/experiments/scripts/shinjuku.par cfs $workload 30us 10000 151000 10000 cfs 15s | tee ./diff_policy/cfs_$workload.out
		mv ./diff_policy/cfs_$workload.out ./diff_policy/cfs_$workload.out.done
		echo "Done cfs_$workload.out"
	fi

	if ! test -f ./diff_policy/fifo_per_cpu_agent_$workload.out.done; then
		sudo bazel-bin/experiments/scripts/shinjuku.par ghost $workload 30us 10000 151000 10000 fifo_per_cpu_agent 15s | tee ./diff_policy/fifo_per_cpu_agent_$workload.out
		mv ./diff_policy/fifo_per_cpu_agent_$workload.out ./diff_policy/fifo_per_cpu_agent_$workload.out.done
		echo "Done fifo_per_cpu_agent_$workload.out"
	fi

	if ! test -f ./diff_policy/agent_sol_$workload.out.done; then
		sudo bazel-bin/experiments/scripts/shinjuku.par ghost $workload 30us 10000 151000 10000 agent_sol 15s | tee ./diff_policy/agent_sol_$workload.out
		mv ./diff_policy/agent_sol_$workload.out ./diff_policy/agent_sol_$workload.out.done
		echo "Done agent_sol_$workload.out"
	fi

	
	
done 

