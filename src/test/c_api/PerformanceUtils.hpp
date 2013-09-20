// @@@LICENSE
//
//      Copyright (c) 2009-2013 LG Electronics, Inc.
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

#include <chrono>
#include <memory>

using namespace std;
using namespace std::chrono;

double BenchmarkMeasure(function<void(size_t)> code, size_t n)
{
	typedef high_resolution_clock ClockT;
	time_point<ClockT> start = ClockT::now();
	code(n);
	time_point<ClockT> finish = ClockT::now();
	duration<double> time_span = duration_cast<duration<double>>(finish - start);
	return time_span.count();
}

double BenchmarkPerformNs(function<void(size_t)> code)
{
	const double low_time_threshold = 3; // seconds, assume a good good resolution warmup
	const double measure_seconds = 5;
	size_t n = 50;
	double t;
	do
	{
		n*=2;
	} while ((t = BenchmarkMeasure(code, n)) < low_time_threshold);

	// actual measure
	size_t repeats = measure_seconds * n / t;
	double timing = BenchmarkMeasure(code, repeats);
	return timing * 1e9 / repeats;
}
