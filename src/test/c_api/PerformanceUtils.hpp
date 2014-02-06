// @@@LICENSE
//
//      Copyright (c) 2009-2014 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#include <ctime>
#include <memory>
#include <vector>
#include <algorithm>

#define BYTES_IN_MB 1000000

using namespace std;

inline double ConvertToMBps(size_t bytes, double seconds)
{
	return bytes / (seconds * BYTES_IN_MB);
}

static clockid_t ChooseBenchmarkClock()
{
	clockid_t clock;
	if (clock_getcpuclockid(0, &clock) != 0)
	{
		// no CPU time, fallback to monotonic
		return CLOCK_MONOTONIC;
	}
	return clock;
}

static double BenchmarkMeasure(function<void(size_t)> code, size_t n)
{
	static const clockid_t clock = ChooseBenchmarkClock();
	timespec start, finish;
	int zero0 = clock_gettime(clock, &start);
	code(n);
	int zero1 = clock_gettime(clock, &finish);
	assert( zero0 == 0 );
	assert( zero1 == 0 );
	if (zero0 != 0 || zero1 != 0) abort();
	return (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec)*1e-9;
}

double Median(std::vector<double> &samples)
{
	assert( !samples.empty() );
	size_t m = samples.size()/2;
	double median;
	std::nth_element(samples.begin(), samples.begin() + m, samples.end());
	median = samples[m];
	if ((samples.size() % 2) == 0)
	{
		std::nth_element(samples.begin(), samples.begin() + m - 1, samples.end());
		median = (median + samples[m-1])/2;
	}
	return median;
}

// benchmark scalable code function (linearity is required)
double BenchmarkPerform(function<void(size_t)> code)
{
	const double low_time_threshold = 1; // seconds, assume a good good resolution warmup
	const double measure_seconds = 15; // quarter of minute
	const double proximity = 0.05; // +5% to ensure boundary cross
	const size_t samples_count = 5; // 5 samples by ~3 seconds each

	const double sample_time_mark = measure_seconds / samples_count;
	const double sample_time = (1+proximity) * sample_time_mark;

	vector<double> samples;
	samples.reserve(samples_count);

	// for first element we do warmup
	size_t n = 50;
	double t;
	while ((t = BenchmarkMeasure(code, n)) < sample_time_mark)
	{
		if (t < low_time_threshold) n *= 2;
		else n = sample_time * n / t;
	}
	samples.push_back(t / n);

	// for rest we do only slight adjustment
	size_t total_n = n;
	double total_t = t;
	for (size_t i = 1; i < samples_count; ++i)
	{
		do
		{
			// adjust n according to current average
			n = sample_time * total_n / total_t;
			t = BenchmarkMeasure(code, n);

			// linearity
			total_n += n, total_t += t;
		} while (t < sample_time_mark); // take only values with a good resolution
		samples.push_back(t / n);
	}
	return Median(samples);
}
