/*
 *  HashStore.cpp
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

#include <cstdlib>
#include "HashStore.h"

HashStore::HashStore(const char * hashFile) throw (int)
{
	string temp, name;
	char * dummy = NULL;
	ifstream file(hashFile);
	if (!file.is_open()) throw 1;
	
	while (getline(file, temp))
	{
		name = temp.substr(0, temp.length() - 33);
		temp = temp.substr(temp.length() - 32);
		if (isMD5Hash(temp))
		{
			MD5 md5;
			for (int i = 0; i < 16; ++i)
				md5.value[i] = strtol(temp.substr(i * 2, 2).c_str(), &dummy, 16);
			hashes[md5] = name;
		}
	}
	file.close();
}

bool HashStore::isMD5Hash(string hash) const
{
	if (hash.length() != 32) return false;
	
	for (int i = 0; i < 32; ++i)
	{
		if(!isxdigit(hash[i])) return false;
	}
	
	return true;
}

void HashStore::SetCharset(string str)
{
	charset = str;
	charsetLength = str.length();
}

void HashStore::CompareHashes(void * data, unsigned int num,
							  unsigned int length, unsigned char * base)
{
	string pt;
	for (unsigned int i = 0; i < num; ++i)
	{
		MD5HTIterator it = hashes.find(((MD5 *)data)[i]);
		if (it != hashes.end()) 
		{
			unsigned int counter = i;
			for (int j = 0, a = 0, carry = 0; j < length; ++j, counter /= charsetLength)
			{
				a = base[j] + carry + counter % charsetLength;
				carry = a / charsetLength;
				a -= carry * charsetLength;
				
				pt += charset[a];
			}
			
			pt = it->second + " : " + pt;
			cout << pt << endl;
			cracked.push_back(pt);
			pt.clear();
			hashes.erase(it);
		}
	}
}

void HashStore::DisplayCracked() const
{
	for(vector<string>::const_iterator it = cracked.begin();
		it != cracked.end();
		++it)
	{
		cout << *it << endl;
	}
}

unsigned int HashStore::NumberOfHashes()
{
	return hashes.size();
}
