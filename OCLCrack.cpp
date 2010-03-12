/*
 *  OCLCrack.cpp
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

#include <sstream>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string.h>

#include "OCLCrack.h"
#include "HashStore.h"
#include "Timer.h"
#include "MD5_cl.h"

using std::ostringstream;
using std::ifstream;
using std::cerr;
using std::cout;

OCLCrack::OCLCrack(HashStore * hs, int minlen, int maxlen,
                   string c, bool v) throw (int) :
    minPlainTextLength(minlen),
    maxPlainTextLength(maxlen),
    verbose(v),
    charset(c),
    charsetLength(charset.length())
{
	hashStore = hs;
	base = new unsigned char[16];
	
    verbose = v;
	hashStore->SetCharset(charset);
	
	if (initializeOpenCL("MD5.cl") == EXIT_FAILURE) throw 1;
	
	result = new unsigned int[maxPlainTextsPerRound * 4];
	
	smallLength = 0;
	unsigned int temp = maxPlainTextsPerRound;
	while (temp /= charsetLength) ++smallLength;
}

OCLCrack::~OCLCrack()
{
    clReleaseMemObject(charsetBuffer);
    clReleaseMemObject(baseBuffer);
	clReleaseMemObject(generatedHashes);
	clReleaseProgram(program);
	clReleaseKernel(compute);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);
	
	delete [] result;
	delete [] base;
}

int OCLCrack::initializeOpenCL(const char * kernel)
{
	int err;
	
    cl_platform_id platform;
    err =clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS)
    {
        cerr << "Could not retrieve OpenCL platform ID.\n";
        return EXIT_FAILURE;
    }

    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
	if (err != CL_SUCCESS)
	{
		cerr << "Could not retrieve OpenCL device IDs.\n";
		return EXIT_FAILURE;
    }

    cl_device_type deviceType;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type),
                   &deviceType, NULL);
    if (deviceType != CL_DEVICE_TYPE_GPU)
    {
        cerr << "Warning! Not using GPU as OpenCL device!!!\n"
                "oclcrack is currently optimized for CUDA architecture, \n"
                "therefore performance (and possibly other) issues may \n"
                "occur if it is being run on other hardware!\n";
    }

    // TODO: calculating the number of plaintexts per round should
    // be further adjusted according to the device type.. reason:
    // using the current calculation, the cracking freezes my computer if
    // running on CPU..
    groupSize = 192;
	cl_ulong maximumAllocationSize;
	err = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong),
                          &maximumAllocationSize, NULL);
    if (err != CL_SUCCESS)
	{
        maxPlainTextsPerRound = 10080000;
	}
	else
	{
		maxPlainTextsPerRound = 
			maximumAllocationSize / sizeof(unsigned int) / 
			4 / groupSize * groupSize;
	}

	context = clCreateContext(0, 1, &device, NULL, NULL, &err);
	if (err != CL_SUCCESS)
	{
		cerr << "Could not create OpenCL context.\n";
		return EXIT_FAILURE;
	}
	
	commands = clCreateCommandQueue(context, device, 0, &err);
	if (err != CL_SUCCESS)
	{
		cerr << "Could not create OpenCL command queue.\n";
		return EXIT_FAILURE;
	}
	
	// Create program object
	program = clCreateProgramWithSource(context, 1, &kernelSource, 
										NULL, &err);
	if (err != CL_SUCCESS)
	{
		cerr << "Could not create OpenCL program.\n";
		return EXIT_FAILURE;
	}
	
	clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		cerr << "Could not build program.\n";
		return EXIT_FAILURE;
	}
	
	// Create kernel object
	compute = clCreateKernel(program, "ComputeHashes", &err);
	if (err != CL_SUCCESS)
	{
		cerr << "Could not create kernel.\n";
		return EXIT_FAILURE;
	}

    // Create buffer for base
    baseBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                sizeof(unsigned char) * 16, NULL, &err);
    if (err != CL_SUCCESS)
    {
        cerr << "Could not create buffer.\n";
        return EXIT_FAILURE;
    }

    // Create buffer for charset...
    charsetBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                   sizeof(unsigned char) * charsetLength,
                                   NULL, &err);
    if (err != CL_SUCCESS)
    {
        cerr << "Could not create buffer.\n" << err;
        return EXIT_FAILURE;
    }
	
    // ... and upload the charset into it
    clEnqueueWriteBuffer(commands, charsetBuffer, CL_TRUE, 0,
                         sizeof(unsigned char) * charsetLength,
                         charset.c_str(), 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
        cerr << "Could not write to buffer.\n";
		return EXIT_FAILURE;
	}
	
	// Create buffer for found hashes
	generatedHashes = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
								 sizeof(unsigned int) * maxPlainTextsPerRound * 4, 
                                 NULL, &err);
	if (err != CL_SUCCESS)
	{
		cerr << "Could not create buffer.\n";
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

void inline OCLCrack::runKernel(unsigned int length, 
								unsigned int plaintexts) const
{
    clSetKernelArg(compute, 0, sizeof(cl_mem), &generatedHashes);
    clSetKernelArg(compute, 1, sizeof(cl_mem), &charsetBuffer);
    clSetKernelArg(compute, 2, sizeof(cl_mem), &baseBuffer);
	clSetKernelArg(compute, 3, sizeof(unsigned int), &charsetLength);
	clSetKernelArg(compute, 4, sizeof(unsigned int), &length);
	
	size_t workitems = static_cast<size_t> (plaintexts);
	clEnqueueNDRangeKernel(commands, compute, 1, NULL, &workitems, 
						   &groupSize, 0, NULL, NULL);
	clFinish(commands); 	
}

void inline OCLCrack::advanceBase(unsigned int length, unsigned int plaintexts)
{
	int counter = plaintexts - 1;
	for (int j = 0, a = 0, carry = 0; j < length; ++j, counter /= charsetLength)
	{
		a = base[j] + carry + counter % charsetLength;
		carry = a / charsetLength;
		a -= carry * charsetLength;
		base[j] = a;
	}
}

void OCLCrack::Crack()
{
	Timer timer;
	
    unsigned int plainTextsPerRound;
	
	// Loop through all plaintext lengths
	for (int length = minPlainTextLength; length <= maxPlainTextLength; ++length)
	{
		if (verbose) cout << "Current plaintext length: " << length << endl;
		
		uint64_t rounds;
		unsigned int mod = 0;
		
		// Determine how many plaintexts are necessery in this round
		if (length <= smallLength)
		{
			rounds = 1;
			
			mod = 1;
			for (int i = 0; i < length; ++i)
				mod *= charsetLength;
			
			if (mod % groupSize != 0)
			{
				mod = ((mod / groupSize) + 1) * groupSize;
			}
		}
		else 
		{
			plainTextsPerRound = maxPlainTextsPerRound;
			uint64_t allPlainTexts = 1;
			for (int i = 0; i < length; ++i) allPlainTexts *= charsetLength;
			rounds = allPlainTexts / maxPlainTextsPerRound;
			mod = allPlainTexts % maxPlainTextsPerRound;
			if (mod != 0) ++rounds;
			if (mod % groupSize != 0)
			{
				mod = ((mod / groupSize) + 1) * groupSize;
			}
		}

		// Loop through all possible plaintexts of length 'length'
		memset(base, 0, 16);
		for (uint64_t i = 0; i < rounds; ++i)
		{
			timer.Start();
			
			if (i == rounds - 1) plainTextsPerRound = mod;
			
			// Write current base to VGA memory
            clEnqueueWriteBuffer(commands, baseBuffer, CL_TRUE, 0,
                                 sizeof(unsigned char) * 16,
                                 base, 0, NULL, NULL);
			
			// Run compute kernel
            runKernel(length, plainTextsPerRound);
            clFinish(commands);
			
			// Read generated hashes
            clEnqueueReadBuffer(commands, generatedHashes, CL_TRUE, 0,
                                sizeof(unsigned int) * plainTextsPerRound * 4,
                                result, 0, NULL, NULL);
			
            hashStore->CompareHashes(result, plainTextsPerRound, length, base);
            if (hashStore->NumberOfHashes() == 0)
			{
                cout << "\n//***** All hashes has been cracked!!! *******//\n";
				hashStore->DisplayCracked();
				return;
			}
			
			if (verbose)
			{
                cout << "\tthis round (#" << i << "): " << plainTextsPerRound
					<< " generated hashes (elapsed time: " 
					<< timer.Stop() << "s)\n";
			}
			
			advanceBase(length, plainTextsPerRound);
		}
	}
}
