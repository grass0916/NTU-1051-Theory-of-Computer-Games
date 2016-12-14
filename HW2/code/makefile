all: ./src/search.cpp
	g++ ./src/search.cpp -std=c++11 -O2 -Wall -o M10415096
judge: ./src/judge.cpp
	g++ ./src/judge.cpp -std=c++11 -O2 -Wall -o judge
test: M10415096 judge
	rm -rf log_ju.txt log_p1.txt log_p2.txt
	./judge 7122 > log_ju.txt &
	./M10415096 127.0.0.1 7122 > log_p1.txt &
	./M10415096 127.0.0.1 7122 > log_p2.txt &
