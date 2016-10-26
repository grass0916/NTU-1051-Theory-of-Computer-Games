#pragma once

#ifndef ELAPSEDTIME_H
#define ELAPSEDTIME_H

#include <chrono>

#define TimePoint std::chrono::time_point<std::chrono::high_resolution_clock> 

class ElapsedTime {
	public:
		void setStart();
		void setEnd();
		long long getElapsedTime();

	private:
		TimePoint start, end;
};

#endif
