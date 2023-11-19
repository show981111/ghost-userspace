wkld=("0.005" "0.010" "0.020" "0.040" "0.080" "0.16")
slice=("30us" "60us" "120us" "240us" "480us" )

# 0.005 0.025 0.05 0.07 0.08 0.09 0.1

for wk in ${wkld[@]}; do
	for s in ${slice[@]}; do
		echo "RUN $s"
		if ! test -f ./diff_wkld_slice/ghost_$wk_$s.out.done; then
			sudo bazel-bin/experiments/scripts/shinjuku.par ghost $wk  $s 10000 151000 10000 agent_shinjuku 15s | tee ./diff_wkld_slice/ghost_$wk_$s.out
			mv ./diff_wkld_slice/ghost_$wk_$s.out ./diff_wkld_slice/ghost_$wk_$s.out.done
			echo "Done ghost_$wk_$s.out"
		fi

		if ! test -f ./diff_wkld_slice/cfs_$wk.out.done; then
			sudo bazel-bin/experiments/scripts/shinjuku.par cfs $wk $s 10000 151000 10000 cfs 15s | tee ./diff_wkld_slice/cfs_$wk.out
			mv ./diff_wkld_slice/cfs_$wk.out ./diff_wkld_slice/cfs_$wk.out.done
			echo "Done cfs_$wk.out"
		fi
	done

	# if ! test -f ./diff_wkld_slice/fifo_per_cpu_agent_$s.out.done; then
	# 	sudo bazel-bin/experiments/scripts/shinjuku.par ghost $s 30us 10000 151000 10000 fifo_per_cpu_agent 30s | tee ./diff_wkld_slice/fifo_per_cpu_agent_$s.out
	# 	mv ./diff_wkld_slice/fifo_per_cpu_agent_$s.out ./diff_wkld_slice/fifo_per_cpu_agent_$s.out.done
	# 	echo "Done fifo_per_cpu_agent_$s.out"
	# fi

	# if ! test -f ./diff_wkld_slice/agent_sol_$s.out.done; then
	# 	sudo bazel-bin/experiments/scripts/shinjuku.par ghost $s 30us 10000 151000 10000 agent_sol 30s | tee ./diff_wkld_slice/agent_sol_$s.out
	# 	mv ./diff_wkld_slice/agent_sol_$s.out ./diff_wkld_slice/agent_sol_$s.out.done
	# 	echo "Done agent_sol_$s.out"
	# fi

	
	
done 

