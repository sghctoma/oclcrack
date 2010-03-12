/*
 *  OCLCrack.h
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

#ifndef _OCLCRACK_H_
#define _OCLCRACK_H_

#include <string>
#include <CL/cl.h>

class HashStore;

using std::string;

class OCLCrack
{
private:
	cl_context context;
	cl_command_queue commands;
	cl_program program;
	cl_kernel compute;
	cl_mem generatedHashes;
    cl_mem charsetBuffer;
    cl_mem baseBuffer;
	
	size_t groupSize;
	string charset;
	size_t maxPlainTextsPerRound;
    unsigned int minPlainTextLength;
	unsigned int maxPlainTextLength;
	unsigned int charsetLength;
    bool verbose;

    unsigned int smallLength;
	
	unsigned int * result;
	unsigned char * base;
	
    HashStore * hashStore;
	
    int initializeOpenCL(const char * kernel);
	void inline runKernel(unsigned int length, unsigned int plaintexts) const;
	void inline advanceBase(unsigned int length, unsigned int plaintexts);
public:
    OCLCrack(HashStore * hs, int minlen, int maxlen,
             string c, bool v) throw (int);
	~OCLCrack();
	
	void Crack();
};

#endif
