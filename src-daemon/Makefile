optimized_cmp_flags=-O2
debug_cmp_flags=-g
lib_flags=-lm
working_dir=.

clean:
	rm -rf ./load_balancer
production: clean prod_build
debug: clean debug_build
debug_build: $(working_dir)/main.cpp $(working_dir)/LoadBalancerConfiguration.cpp $(working_dir)/LoadBalancerState.cpp $(working_dir)/LoadBalancer.cpp $(working_dir)/LoadBalancerAlgorithm.cpp $(working_dir)/../global-hdrs/base64.c
	g++ -DDEBUG -static $(debug_cmp_flags) $(working_dir)/main.cpp $(working_dir)/LoadBalancerConfiguration.cpp $(working_dir)/LoadBalancerState.cpp $(working_dir)/LoadBalancer.cpp $(working_dir)/LoadBalancerAlgorithm.cpp $(working_dir)/../global-hdrs/base64.c -o load_balancer $(lib_flags)
prod_build: $(working_dir)/main.cpp $(working_dir)/LoadBalancerConfiguration.cpp $(working_dir)/LoadBalancerState.cpp $(working_dir)/LoadBalancer.cpp $(working_dir)/LoadBalancerAlgorithm.cpp $(working_dir)/../global-hdrs/base64.c
	g++ -DPRODUCTION -static $(optimized_cmp_flags) $(working_dir)/main.cpp $(working_dir)/LoadBalancerConfiguration.cpp $(working_dir)/LoadBalancerState.cpp $(working_dir)/LoadBalancer.cpp $(working_dir)/LoadBalancerAlgorithm.cpp $(working_dir)/../global-hdrs/base64.c -o load_balancer $(lib_flags)
