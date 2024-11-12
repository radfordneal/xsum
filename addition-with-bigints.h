/*
Utilities for working with floating point numbers represented as 4160-bit integers.
Only used in test code.

For full-precision sums of doubles, you can convert many doubles to bigints and add them together,
then convert the result back to a double.

Usage:

bigint a;

// initialize to 0
bigint_init(&a);
// or initialize from a double
bigint_init(&a);
double_to_bigint(1.01, &a);

// a += b
bigint b;
double_to_bigint(2.02, &b);
add_bigints(&a, &b, &a);

// convert back to a double
printf("%.17g\n", bigint_to_double(&a));
*/

/* Copyright 2024 Kevin Gibbons

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#include <stdint.h>
#include <stdbool.h>

#define ARRAY_SIZE 65 // 4096 bits to hold any double, plus 64 bits to handle intermediate overflow
#define TOTAL_BITS (ARRAY_SIZE * 64)

// big-endian
typedef struct {
  bool is_negative;
  uint64_t digits[ARRAY_SIZE];
} bigint;

void bigint_init(bigint* num);

int add_bigints(const bigint* a, const bigint* b, bigint* result);

void double_to_bigint(double value, bigint* result);
double bigint_to_double(const bigint* arg);
