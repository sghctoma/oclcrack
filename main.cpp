/*
 *  main.cpp
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
 
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <CL/cl.h>

#include "HashStore.h"
#include "OCLCrack.h"

int minlen = 1;
int maxlen = 10;
bool verbose = false;
string charset;

inline void printUsage()
{
    std::cout << "Usage: oclcrack [--start=minimum] [--end=maximum] "
            "[--charset=charset] [--verbose] [--help] file" << std::endl;
    std::cout << "  --start=minlen\tMinimum plaintext length" << std::endl;
    std::cout << "  --end=maxlen\t\tMinimum plaintext length" << std::endl;
    std::cout << "  --charset=charset\tCharacters used to generate plaintexts "
            << std::endl;
    std::cout << "  --verbose\t\tVerbose output" << std::endl;
    std::cout << "  --help\t\tprints this help message." << std::endl;
}

void parseOptions(int argc, char ** argv)
{
    if ((string)argv[1] == "--help")
    {
        printUsage();
        exit(EXIT_SUCCESS);
    }

    string temp;
    for (int i = 1; i < argc - 1; ++i)
    {
        temp = (string)argv[i];
        if (temp.substr(0, 8) == "--start=")
        {
            minlen = atoi(temp.substr(8).c_str());
        }
        else if (temp.substr(0, 6) == "--end=")
        {
            maxlen = atoi(temp.substr(6).c_str());
        }
        else if (temp.substr(0, 10) == "--charset=")
        {
            charset = temp.substr(10);
        }
        else if (temp == "--verbose")
        {
            verbose = true;
        }
        else std::cerr << "Warning: " << temp << " is not a valid parameter!\n";
    }

    // Do some checks on the parameters
    if (minlen < 1 || minlen > 10)
    {
        std::cerr << "Warning: Invalid \"start\" value! Using default(1).\n";
        minlen = 1;
    }

    if (maxlen < 1 || maxlen > 10)
    {
        std::cerr << "Warning: Invalid \"end\" value! Using default(10).\n";
        maxlen = 10;
    }
    if (minlen > maxlen)
    {
        std::cerr << "Warning: \"start\" is bigger, than \"end\"."
                "Using default \"start\"(1).\n";
        minlen = 1;
    }

    if (charset.length() == 0)
    {
        charset =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    }
}

int main (int argc, char * argv[]) 
{	
	if (argc < 2)
	{
        printUsage();
        return 1;
	}    

    parseOptions(argc, argv);

	HashStore * hs;
	try { hs = new HashStore(argv[argc - 1]); }
	catch (...) 
	{
        std::cerr << "Hash file does not exist! Exiting...\n" << std::endl;
		return EXIT_FAILURE;
	}
	
	OCLCrack * cracker;
    try { cracker = new OCLCrack(hs, minlen, maxlen, charset, verbose); }
	catch (...) { return EXIT_FAILURE; }
	
	cracker->Crack();
	
	delete cracker;
	delete hs;
	
    return 0;
}
