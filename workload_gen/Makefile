all: workload

workload: workload_gen.cpp
	g++ -O3 -o workload workload_gen.cpp -Wall -Wextra --std=c++11
workload_2: workload_gen_w_selectivity.cpp
	g++ -O3 -o workload workload_gen_w_selectivity.cpp -Wall -Wextra --std=c++11
workload_3: workload_gen_by_range.cpp
	g++ -O3 -o workload workload_gen_by_range.cpp -Wall -Wextra --std=c++11

clean:
	$(RM) -rf workload my_data
