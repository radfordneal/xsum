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

#include <assert.h>
#include <stdlib.h>

#include "double-utils.h"

const int interesting_exponents[] = {2046, 2045, 1994, 1995, 1993, 0, 1, 2, 1021, 1022, 1023, 1024, 1025, 1026};
const uint64_t interesting_significands[] = {0b1111111111111111111111111111111111111111111111111111, 0b1000000000000000000000000000000000000000000000000000, 0b1000000000000000000000000000000000000000000000000001, 0b1111111111111111111111111111111111111111111111111110, 0b111111111111111111111111111111111111111111111111111, 0, 1, 2};

const int num_interesting_exponents = sizeof(interesting_exponents) / sizeof(int);
const int num_interesting_significands = sizeof(interesting_significands) / sizeof(uint64_t);

double random_double() {
  int sign = rand() & 1;

  int exponent;
  if (rand() & 1) {
    exponent = rand() & 2047;
    while (exponent == 2047) exponent = rand() & 2047;
  } else {
    exponent = interesting_exponents[rand() % num_interesting_exponents];
  }

  uint64_t significand;
  if (rand() & 1) {
    // probably not necessary to be defensive against platforms with 16-bit RAND_MAX, but we might as well
    significand = ((uint64_t)rand() << 32) ^ ((uint64_t)rand() << 16) ^ (uint64_t)rand();
    significand &= ((1ULL << 52) - 1);
  } else {
    significand = interesting_significands[rand() % num_interesting_significands];
  }

  return construct_double(sign, exponent, significand);
}

double construct_double(bool sign, int exponent, uint64_t significand) {
  assert(exponent >= 0 && exponent <= 2046);
  assert(significand < (1ULL << 52));
  union {
    uint64_t u;
    double d;
  } u;
  u.u = ((uint64_t)sign << 63) | ((uint64_t)exponent << 52) | significand;
  return u.d;
}

void deconstruct_double(double value, bool* sign, int* exponent, uint64_t* significand) {
  union {
    uint64_t u;
    double d;
  } u;
  u.d = value;

  *sign = (u.u >> 63) & 1;
  *exponent = (u.u >> 52) & 0x7FF;
  *significand = u.u & 0xFFFFFFFFFFFFF;
}
