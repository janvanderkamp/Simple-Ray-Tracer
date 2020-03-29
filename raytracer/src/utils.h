#ifndef UTILS
#define UTILS

#include <chrono>

using namespace std;
using namespace chrono;

namespace Utils
{
	float rand_n()
	{
		return (float(rand()) / float(RAND_MAX));
	}

	Vector3 randomInUnitSphere()
	{
		Vector3 p;
		do
		{
			p = 2.f * Vector3(rand_n(), rand_n(), rand_n()) - Vector3(1.f);
		} while (p.magnitudeSquared() >= 1.0);

		return p;
	}

};


class TimeUtils
{
public:

	TimeUtils()
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

