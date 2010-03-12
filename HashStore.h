/*
 *  HashStore.h
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

#ifndef _HASHSTORE_H_
#define _HASHSTORE_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// TODO: Implement an own hash table.. or just wait till
// C++0x becomes standard :)
#include <tr1/unordered_map>

using std::string;
using std::tr1::unordered_map;
using std::cout;
using std::endl;
using std::ifstream;
using std::vector;

struct MD5
{
	unsigned char value[16];
};

struct MD5Hash
{
	size_t operator()(const MD5 & x) const
	{
		return *(int32_t *)(x.value);
	}
};

struct MD5Equal
{
	bool operator()(const MD5 & x, const MD5 & y) const
	{
		for (int i = 0; i < 16; ++i)
			if (x.value[i] != y.value[i]) return false;
		return true;
	}
};

typedef unordered_map<MD5, string, MD5Hash, MD5Equal> MD5HashTable;
typedef MD5HashTable::iterator MD5HTIterator;

class HashStore
{
private:
	MD5HashTable hashes;
	vector<string> cracked;
	string charset;
	unsigned int charsetLength;
	
	bool isMD5Hash(string hash) const;
public:
	HashStore(const char * hashFile) throw (int);
	
	unsigned int NumberOfHashes();
	void CompareHashes(void * data, unsigned int num,
					   unsigned int length, unsigned char * base);
	void DisplayCracked() const;
	void SetCharset(string charset);
};

#endif
