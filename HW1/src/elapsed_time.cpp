#include "elapsed_time.h"

void ElapsedTime::setStart() {
	this->start = std::chrono::high_resolution_clock::now();
}

void ElapsedTime::setEnd() {
	this->end = std::chrono::high_resolution_clock::now();
}

long long ElapsedTime::getElapsedTime() {
	return std::chrono::duration_cast<std::chrono::microseconds>(this->end - this->start).count();
}
