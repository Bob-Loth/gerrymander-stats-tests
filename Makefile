gerryStats: main.cpp stats.cpp stats.h gerryStats.h
	clang++ -std=c++11 *.cpp -o gerryStats

run: gerryStats
	make && ./gerryStats

clean:
	rm *.o gerryStats