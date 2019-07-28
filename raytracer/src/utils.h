#ifndef UTILS
#define UTILS

#include <chrono>

using namespace std;
using namespace chrono;

class Utils
{
public:

	Utils()
	{
		_startTime = std::chrono::high_resolution_clock::now();
	}

	float secondsSinceRun() const 
	{
		return static_cast<double>(millisecondsSinceRun()) / 1000.0;
	}

	uint64_t millisecondsSinceRun() const
	{
		return duration_cast<milliseconds>(high_resolution_clock::now() - _startTime).count();
	}

private:

	time_point<steady_clock> _startTime;
};

#endif // !UTILS

