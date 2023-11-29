optimized_cmp_flags=-O2
debug_cmp_flags=-g
lib_flags=-lm
all_flags=-std=c++17
working_dir=.
binary_name=health_reporter

clean:
	rm -rf ./health_reporter
production: clean prod_build
debug: clean debug_build
debug_build: $(working_dir)/main.cpp $(working_dir)/HealthReporter.cpp $(working_dir)/../global-hdrs/base64.c
	g++ -DDEBUG -static $(all_flags) $(debug_cmp_flags) $(working_dir)/main.cpp $(working_dir)/HealthReporter.cpp $(working_dir)/../global-hdrs/base64.c -o $(binary_name) $(lib_flags)
prod_build: $(working_dir)/main.cpp $(working_dir)/HealthReporter.cpp $(working_dir)/../global-hdrs/base64.c
	g++ -DPRODUCTION -static $(all_flags) $(optimized_cmp_flags) $(working_dir)/main.cpp $(working_dir)/HealthReporter.cpp $(working_dir)/../global-hdrs/base64.c -o $(binary_name) $(lib_flags)
