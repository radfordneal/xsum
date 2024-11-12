/*
Shewchuk's algorithm for exactly floating point addition
https://www-2.cs.cmu.edu/afs/cs/project/quake/public/papers/robust-arithmetic.ps
as implemented in Python's fsum: https://github.com/python/cpython/blob/48dfd74a9db9d4aa9c6f23b4a67b461e5d977173/Modules/mathmodule.c#L1359-L1474
adapted to handle overflow via an additional "biased" partial, representing 2**1024 times its actual value

Only used in test code.

Usage:

shewchuk adder;
shewchuk_init(&adder);

shewchuk_add(&adder, random_double());
shewchuk_add(&adder, random_double());
shewchuk_add(&adder, random_double());

double result = shewchuk_compute(&adder);

Reset the adder to 0:
shewchuk_reinit(&adder)
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

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  double* data;
  size_t size;
  size_t capacity;
} vector;

typedef struct {
  bool is_negative_zero;
  vector partials;
  double overflow;
} shewchuk;

void shewchuk_init(shewchuk* self);
void shewchuk_reinit(shewchuk* self);
void shewchuk_add(shewchuk* self, double x);
double shewchuk_compute(shewchuk* self);
