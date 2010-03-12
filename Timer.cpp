/*
 *  Timer.cpp
 *  oclcrack
 *
 *  Copyright (c) 2009 Tamas Szakaly - sghctoma@gmail.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#include <cstdio>
#include "Timer.h"

Timer::Timer()
{
#if (defined(WIN32) || defined(__WIN32__) || defined(__WIN32))
	QueryPerformanceFrequency(&frequency);
#endif
}

void Timer::Start()
{
#if (defined(WIN32) || defined(__WIN32__) || defined(__WIN32))
	QueryPerformanceCounter(&start);
#else
	gettimeofday(&start, NULL);
#endif
}

double Timer::Stop()
{
#if (defined(WIN32) || defined(__WIN32__) || defined(__WIN32))
	QueryPerformanceCounter(&end);
	return (double)(end - start) / frequency * 1000.0;
#else
	gettimeofday(&end, NULL);
	return (double)end.tv_sec + (double)end.tv_usec / 1000000 - 
			(double)start.tv_sec - (double)start.tv_usec / 1000000;
#endif
}
