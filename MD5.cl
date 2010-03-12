/*
 *  MD5.cl
 *  oclcrack
 *
 *  Copyright (c) 2009 Tamas Szakaly - sghctoma@gmail.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the 'Software'), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

__kernel void ComputeHashes(__global uint4 * hashes,
                            __constant char * charset,
                            __constant char * base,
							const uint charsetLength,
							const uint plainTextLength)
{
	uint X[5];
	uint id = get_global_id(0);
	int counter = id;
	
	int oc, a = 0, carry = 0;
	X[0] = 0; X[1] = 0; X[2] = 0; X[3] = 0; X[4] = 0;
	
	for (int i = 0; i < plainTextLength; ++i)
	{
		oc = counter / charsetLength;
        a = base[i] + carry + counter - oc * charsetLength;
		if (a >= charsetLength) { a -= charsetLength; carry = 1; }
		else carry = 0;
        X[i >> 2] |= charset[a] << ((i & 3) << 3);
		counter = oc;
	}
	
	X[plainTextLength >> 2] |= ((uint)(0x00000080) << ((plainTextLength & 3) << 3));
	
	uint A, B, C, D;
	
#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a,b,c,d,k,s,t)										\
{																\
	a += F(b,c,d) + X[k] + t; a = S(a,s) + b;					\
}																\

#define P0(a,b,c,d,k,s,t)										\
{																\
	a += F(b,c,d) + t; a = S(a,s) + b;							\
}																\

#define P14(a,b,c,d,k,s,t)										\
{																\
	a += F(b,c,d) + (plainTextLength << 3) + t; a = S(a,s) + b;	\
}																\

    A = 0x67452301;
    B = 0xefcdab89;
    C = 0x98badcfe;
    D = 0x10325476;
	
#define F(x,y,z) (z ^ (x & (y ^ z)))

    P( A, B, C, D,  0,  7, 0xD76AA478 );
    P( D, A, B, C,  1, 12, 0xE8C7B756 );
    P( C, D, A, B,  2, 17, 0x242070DB );
    P( B, C, D, A,  3, 22, 0xC1BDCEEE );
    P( A, B, C, D,  4,  7, 0xF57C0FAF );
    P0( D, A, B, C,  5, 12, 0x4787C62A );
    P0( C, D, A, B,  6, 17, 0xA8304613 );
    P0( B, C, D, A,  7, 22, 0xFD469501 );
    P0( A, B, C, D,  8,  7, 0x698098D8 );
    P0( D, A, B, C,  9, 12, 0x8B44F7AF );
    P0( C, D, A, B, 10, 17, 0xFFFF5BB1 );
    P0( B, C, D, A, 11, 22, 0x895CD7BE );
    P0( A, B, C, D, 12,  7, 0x6B901122 );
    P0( D, A, B, C, 13, 12, 0xFD987193 );
    P14( C, D, A, B, 14, 17, 0xA679438E );
    P0( B, C, D, A, 15, 22, 0x49B40821 );

#undef F

#define F(x,y,z) (y ^ (z & (x ^ y)))

    P( A, B, C, D,  1,  5, 0xF61E2562 );
    P0( D, A, B, C,  6,  9, 0xC040B340 );
	P0( C, D, A, B, 11, 14, 0x265E5A51 );
    P( B, C, D, A,  0, 20, 0xE9B6C7AA );
    P0( A, B, C, D,  5,  5, 0xD62F105D );
    P0( D, A, B, C, 10,  9, 0x02441453 );
    P0( C, D, A, B, 15, 14, 0xD8A1E681 );
    P( B, C, D, A,  4, 20, 0xE7D3FBC8 );
    P0( A, B, C, D,  9,  5, 0x21E1CDE6 );
    P14( D, A, B, C, 14,  9, 0xC33707D6 );
    P( C, D, A, B,  3, 14, 0xF4D50D87 );
    P0( B, C, D, A,  8, 20, 0x455A14ED );
    P0( A, B, C, D, 13,  5, 0xA9E3E905 );
    P( D, A, B, C,  2,  9, 0xFCEFA3F8 );
    P0( C, D, A, B,  7, 14, 0x676F02D9 );
    P0( B, C, D, A, 12, 20, 0x8D2A4C8A );

#undef F
    
#define F(x,y,z) (x ^ y ^ z)

    P0( A, B, C, D,  5,  4, 0xFFFA3942 );
    P0( D, A, B, C,  8, 11, 0x8771F681 );
    P0( C, D, A, B, 11, 16, 0x6D9D6122 );
    P14( B, C, D, A, 14, 23, 0xFDE5380C );
    P( A, B, C, D,  1,  4, 0xA4BEEA44 );
    P( D, A, B, C,  4, 11, 0x4BDECFA9 );
    P0( C, D, A, B,  7, 16, 0xF6BB4B60 );
    P0( B, C, D, A, 10, 23, 0xBEBFBC70 );
    P0( A, B, C, D, 13,  4, 0x289B7EC6 );
    P( D, A, B, C,  0, 11, 0xEAA127FA );
    P( C, D, A, B,  3, 16, 0xD4EF3085 );
    P0( B, C, D, A,  6, 23, 0x04881D05 );
    P0( A, B, C, D,  9,  4, 0xD9D4D039 );
    P0( D, A, B, C, 12, 11, 0xE6DB99E5 );
    P0( C, D, A, B, 15, 16, 0x1FA27CF8 );
    P( B, C, D, A,  2, 23, 0xC4AC5665 );

#undef F

#define F(x,y,z) (y ^ (x | ~z))

    P( A, B, C, D,  0,  6, 0xF4292244 );
    P0( D, A, B, C,  7, 10, 0x432AFF97 );
    P14( C, D, A, B, 14, 15, 0xAB9423A7 );
    P0( B, C, D, A,  5, 21, 0xFC93A039 );
    P0( A, B, C, D, 12,  6, 0x655B59C3 );
    P( D, A, B, C,  3, 10, 0x8F0CCC92 );
    P0( C, D, A, B, 10, 15, 0xFFEFF47D );
    P( B, C, D, A,  1, 21, 0x85845DD1 );
    P0( A, B, C, D,  8,  6, 0x6FA87E4F );
    P0( D, A, B, C, 15, 10, 0xFE2CE6E0 );
    P0( C, D, A, B,  6, 15, 0xA3014314 );
    P0( B, C, D, A, 13, 21, 0x4E0811A1 );
    P( A, B, C, D,  4,  6, 0xF7537E82 );
    P0( D, A, B, C, 11, 10, 0xBD3AF235 );
    P( C, D, A, B,  2, 15, 0x2AD7D2BB );
    P0( B, C, D, A,  9, 21, 0xEB86D391 );

#undef F
	
	hashes[id].x = A + 0x67452301;
	hashes[id].y = B + 0xefcdab89;
	hashes[id].z = C + 0x98badcfe;
	hashes[id].w = D + 0x10325476;
}
