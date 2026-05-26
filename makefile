sb: # Server Build
	@g++ src/server.cpp -o src/output/server.exe

cb: # Client Build
	@g++ src/client.cpp -o src/output/client.exe

bb: # Benchmark Build
	@g++ benchmark/Benchmark.cpp -o benchmark/output/benchmark.exe

sr: # Server Run
	@./src/output/server.exe

cr: # Client Run
	@./src/output/client.exe

br: # Benchmark Run
	@./benchmark/output/benchmark.exe