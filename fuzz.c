/*
  Fuzz xsum against Shewchuck and addition-with-bigints.
  Does not check -Infinity / Infinity / NaN in inputs.
  Also does not check the case where inputs are all -0.
  Checks only summation, not vector norm, dot product, or division.

    Usage: fuzz iterations [ seed ]

  If seed is absent, it is set from the time. 
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

/* Radford Neal, Nov. 2024:  Added command-line options to set the number
   of iterations and to set the seed. */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "double-utils.h"
#include "shewchuk.h"
#include "addition-with-bigints.h"
#include "xsum.h"

static void usage(void) 
{ fprintf(stderr,"Usage: fuzz iterations [ seed ]\n");
  exit(1);
}

int main(int argc, char **argv) {

  long iterations;
  double iters;
  char junk;

  if (argc != 2 && argc != 3) usage();

  if (sscanf(argv[1],"%lf%c",&iters,&junk) != 1 
       || (iterations=(long)iters) != iters || iterations < 0) usage();

  int seed = time(NULL);
  if (argc > 2)
  { if (sscanf(argv[2],"%d%c",&seed,&junk) != 1 || seed <= 0) usage();
  }

  printf("iterations: %ld, seed: %d\n", iterations, seed);
  srand(seed);

  xsum_small_accumulator sacc;
  xsum_large_accumulator lacc;

  shewchuk adder;
  shewchuk_init(&adder);

  bigint bigint_sum;
  bigint bigint_addend;

  double addends[10];
  for (long i = 0; i < iterations; ++i) {
    xsum_small_init(&sacc);
    xsum_large_init(&lacc);
    shewchuk_reinit(&adder);
    bigint_init(&bigint_sum);

    int count = (rand() % 7) + 2;
    for (int j = 0; j < count; ++j) {
      double v = random_double();
      addends[j] = v;
      xsum_small_add1(&sacc, v);
      xsum_large_add1(&lacc, v);
      shewchuk_add(&adder, v);

      double_to_bigint(v, &bigint_addend);
      add_bigints(&bigint_sum, &bigint_addend, &bigint_sum);
    }

    double result_s = xsum_small_round(&sacc);
    double result_l = xsum_large_round(&lacc);
    double result_shewchuk = shewchuk_compute(&adder);
    double result_bigint = bigint_to_double(&bigint_sum);

    if (result_shewchuk != result_bigint) {
      printf("!!! bigint / shewchuk disagreement !!!\n");
      printf("addends: ");
      for (int j = 0; j < count; ++j) {
        printf("%.17g, ", addends[j]);
      }
      printf("\n");
      printf("bigint:     %.17g\n", result_bigint);
      printf("shewchuk:   %.17g\n", result_shewchuk);
      return 1;
    }

    if (result_s != result_shewchuk || result_l != result_shewchuk) {
      printf("!!! xsum / shewchuk disagreement !!!\n");
      printf("addends: ");
      for (int j = 0; j < count; ++j) {
        printf("%.17g, ", addends[j]);
      }
      printf("\n");
      printf("shewchuk:   %.17g\n", result_shewchuk);
      printf("small xsum: %.17g\n", result_s);
      printf("large xsum: %.17g\n", result_l);
      return 1;
    }
  }
}
