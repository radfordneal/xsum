	/* AUTOMATIC CORRECTNESS CHECKS FOR FUNCTIONS FOR EXACT SUMMATION. */

/* Copyright 2015, 2018, 2021, 2024 Radford M. Neal

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


/* Program to check correctness of the small and large superaccumulator
   methods.  May be passed a -e argument to echo details of tests.  
   If -d is given instead, it both echos and prints debug information.
   If -dL# is given, it prints debug information only for test # in
   section L. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "xsum.h"
#include "pbinary.h"

#define pow2_16 (1.0 / (1 << 16))
#define pow2_32 (pow2_16 * pow2_16)
#define pow2_64 (pow2_32 * pow2_32)
#define pow2_128 (pow2_64 * pow2_64)
#define pow2_256 (pow2_128 * pow2_128)
#define pow2_512 (pow2_256 * pow2_256)
#define pow2_1024 (pow2_512 * pow2_512)

#define pow2_52 (1.0 / (1 << 22) / (1 << 30))

#define Lnormal (2*((.5/pow2_1024)-(.25/pow2_1024)*pow2_52))
#define Snormal (4*pow2_1024)
#define Ldenorm (4*pow2_1024 - 4*pow2_1024*pow2_52)
#define Sdenorm (4*pow2_1024*pow2_52)

double zero = 0.0;

/* Tests with one term.  Answer should be the same as the term. 
   All are also done with negation of value here. */

xsum_flt one_term[] = {

  0.0,             /* Some unexceptional examples of normal numbers */
  -0.0,
  -2.0,
  1.0,
  0.1,
  3.1,
  2.3e10,
  3.2e-10,
  123e123,
  54.11e-150,
  2*((.5/pow2_128)+(.25/pow2_128)*pow2_52),
  2*((.5/pow2_128)-(.25/pow2_128)*pow2_52),  /* Mantissa all 1s */
  Lnormal,                                /* Largest normal number */
  Snormal,                                /* Smallest normal number */
  Ldenorm,                                /* Largest denormalized number */
  Sdenorm,                                /* Smallest denormalized number > 0 */
  1.23e-309,                              /* Other denormalized numbers */
  4.57e-314,
  9.7e-322,
  Sdenorm/pow2_64/2,
};

/* Tests with two terms.  Answer should match ordinary floating point add. 
   All are also done with negation of the values here. */

xsum_flt two_term[] = {

1.0, 2.0,         /* Unexceptional adds of normal numbers */
0.1, 12.2,
1.0, -0.0,
0.0, -0.0,
-13.1, 0.0,
12.1, -11.3,
11.3, -12.1,
1.234567e14, 9.87654321,
1.234567e14, -9.87654321,
3.1e200, 1.7e-100,  /* Smaller term should disappear */
3.1e200, -1.7e-100,
1.7e-100, 3.1e200,
1.7e-100, -3.1e200,
1, pow2_52,       /* Test rounding */
1, pow2_52/2,
1, pow2_52/2+pow2_52/4096,
1, pow2_52/2+pow2_52/(1<<30)/(1<<10),
1, pow2_52/2-pow2_52/4096,
1 + pow2_52, pow2_52/2,
1 + pow2_52, pow2_52/2 - pow2_52*pow2_52,
pow2_256, 2*((.5/pow2_128)-(.25/pow2_128)*pow2_52),  /* Mantissa all 1s */
-pow2_256, -2*((.5/pow2_128)-(.25/pow2_128)*pow2_52),
pow2_256, -2*((.5/pow2_128)-(.25/pow2_128)*pow2_52),
1.7976931348623157e+308, 1,
1.7976931348623157e+308, -1,
0.99999999999999989, 8.6736173798840355e-19,
0.99999999999999989, -8.6736173798840355e-19,
Sdenorm, 7.1,              /* Adds with denormalized numbers */
Sdenorm, -7.1,
7.1, Sdenorm,
-7.1, Sdenorm,
Ldenorm, Sdenorm,
Ldenorm, -Sdenorm,
Sdenorm, Sdenorm,
Sdenorm, -Sdenorm,
Ldenorm, Snormal,
Snormal, Ldenorm,
4.57e-314, 9.7e-322,
-4.57e-314, 9.7e-322,
9.7e-322, 4.57e-321,
9.7e-322, -4.57e-321,
3.57e-315, 8.7e-321,
-2.57e-315, 7.7e-321,
1.7e-316, 2.57e-320,
6.4e-321, -6.51e-321,
2.0, -2.0*(1+pow2_52),
Lnormal, Lnormal,              /* Overflow */
Lnormal, Lnormal*pow2_52/2,
1.0/0.0, 123,                  /* Infinity / NaN */
-1.0/0.0, 123,
1.0/0.0, -1.0/0.0,
0.0/0.0, 123,
123, 0.0/0.0,
(double)(1L<<55), 0.0,         /* Tests of rounding */
(double)(1L<<55), 1.0,
(double)(1L<<55), 2.0,
(double)(1L<<55), 3.0,
(double)(1L<<55), 4.0,
(double)(1L<<55), 5.0,
(double)(1L<<55), 6.0,
(double)(1L<<55), 7.0,
(double)(1L<<55)+8.0, 0.0,
(double)(1L<<55)+8.0, 1.0,
(double)(1L<<55)+8.0, 2.0,
(double)(1L<<55)+8.0, 3.0,
(double)(1L<<55)+8.0, 4.0,
(double)(1L<<55)+8.0, 5.0,
(double)(1L<<55)+8.0, 6.0,
(double)(1L<<55)+8.0, 7.0,
(double)(1L<<55)+16.0, 0.0,
(double)(1L<<55)+16.0, 1.0,
(double)(1L<<55)+16.0, 2.0,
(double)(1L<<55)+16.0, 3.0,
(double)(1L<<55)+16.0, 4.0,
(double)(1L<<55)+16.0, 5.0,
(double)(1L<<55)+16.0, 6.0,
(double)(1L<<55)+24.0, 7.0,
(double)(1L<<55)+24.0, 0.0,
(double)(1L<<55)+24.0, 1.0,
(double)(1L<<55)+24.0, 2.0,
(double)(1L<<55)+24.0, 3.0,
(double)(1L<<55)+24.0, 4.0,
(double)(1L<<55)+24.0, 5.0,
(double)(1L<<55)+24.0, 6.0,
(double)(1L<<55)+24.0, 7.0,
(double)(1L<<55), 0.0+1e-45,
(double)(1L<<55), 1.0+1e-45,
(double)(1L<<55), 2.0+1e-45,
(double)(1L<<55), 3.0+1e-45,
(double)(1L<<55), 4.0+1e-45,
(double)(1L<<55), 5.0+1e-45,
(double)(1L<<55), 6.0+1e-45,
(double)(1L<<55), 7.0+1e-45,
(double)(1L<<55)+8.0, 0.0+1e-45,
(double)(1L<<55)+8.0, 1.0+1e-45,
(double)(1L<<55)+8.0, 2.0+1e-45,
(double)(1L<<55)+8.0, 3.0+1e-45,
(double)(1L<<55)+8.0, 4.0+1e-45,
(double)(1L<<55)+8.0, 5.0+1e-45,
(double)(1L<<55)+8.0, 6.0+1e-45,
(double)(1L<<55)+8.0, 7.0+1e-45,
(double)(1L<<55)+16.0, 0.0+1e-45,
(double)(1L<<55)+16.0, 1.0+1e-45,
(double)(1L<<55)+16.0, 2.0+1e-45,
(double)(1L<<55)+16.0, 3.0+1e-45,
(double)(1L<<55)+16.0, 4.0+1e-45,
(double)(1L<<55)+16.0, 5.0+1e-45,
(double)(1L<<55)+16.0, 6.0+1e-45,
(double)(1L<<55)+24.0, 7.0+1e-45,
(double)(1L<<55)+24.0, 0.0+1e-45,
(double)(1L<<55)+24.0, 1.0+1e-45,
(double)(1L<<55)+24.0, 2.0+1e-45,
(double)(1L<<55)+24.0, 3.0+1e-45,
(double)(1L<<55)+24.0, 4.0+1e-45,
(double)(1L<<55)+24.0, 5.0+1e-45,
(double)(1L<<55)+24.0, 6.0+1e-45,
(double)(1L<<55)+24.0, 7.0+1e-45,
(double)(1L<<55), 0.0-1e-45,
(double)(1L<<55), 1.0-1e-45,
(double)(1L<<55), 2.0-1e-45,
(double)(1L<<55), 3.0-1e-45,
(double)(1L<<55), 4.0-1e-45,
(double)(1L<<55), 5.0-1e-45,
(double)(1L<<55), 6.0-1e-45,
(double)(1L<<55), 7.0-1e-45,
(double)(1L<<55)+8.0, 0.0-1e-45,
(double)(1L<<55)+8.0, 1.0-1e-45,
(double)(1L<<55)+8.0, 2.0-1e-45,
(double)(1L<<55)+8.0, 3.0-1e-45,
(double)(1L<<55)+8.0, 4.0-1e-45,
(double)(1L<<55)+8.0, 5.0-1e-45,
(double)(1L<<55)+8.0, 6.0-1e-45,
(double)(1L<<55)+8.0, 7.0-1e-45,
(double)(1L<<55)+16.0, 0.0-1e-45,
(double)(1L<<55)+16.0, 1.0-1e-45,
(double)(1L<<55)+16.0, 2.0-1e-45,
(double)(1L<<55)+16.0, 3.0-1e-45,
(double)(1L<<55)+16.0, 4.0-1e-45,
(double)(1L<<55)+16.0, 5.0-1e-45,
(double)(1L<<55)+16.0, 6.0-1e-45,
(double)(1L<<55)+24.0, 7.0-1e-45,
(double)(1L<<55)+24.0, 0.0-1e-45,
(double)(1L<<55)+24.0, 1.0-1e-45,
(double)(1L<<55)+24.0, 2.0-1e-45,
(double)(1L<<55)+24.0, 3.0-1e-45,
(double)(1L<<55)+24.0, 4.0-1e-45,
(double)(1L<<55)+24.0, 5.0-1e-45,
(double)(1L<<55)+24.0, 6.0-1e-45,
(double)(1L<<55)+24.0, 7.0-1e-45,
};

/* Tests with three terms.  Answers are given here as a fourth number,
   some computed/verified using Rmpfr in check.r.  All are also done 
   with negation of the values here. */

xsum_flt three_term[] = {
Lnormal, Sdenorm, -Lnormal, Sdenorm,
-Lnormal, Sdenorm, Lnormal, Sdenorm,
Sdenorm, Snormal, -Sdenorm, Snormal,
12345.6, Snormal, -12345.6, Snormal,
12345.6, -Snormal, -12345.6, -Snormal,
12345.6, Ldenorm, -12345.6, Ldenorm,
12345.6, -Ldenorm, -12345.6, -Ldenorm,
2.0, -2.0*(1+pow2_52), pow2_52/8, -2*pow2_52+pow2_52/8,
1.0, 0.0, 0.0, 1.0,
-1.0, -0.0, -0.0, -1.0,
0.0, 0.0, 0.0, 0.0,
-0.0, -0.0, -0.0, 0.0,
1.0, 2.0, 3.0, 6.0,
12.0, 3.5, 2.0, 17.5,
3423.34e12, -93.431, -3432.1e11, 3080129999999906.5,
432457232.34, 0.3432445, -3433452433, -3000995200.3167553,
};

/* Tests with ten terms.  Answers are given here as an eleventh number,
   some computed/verified using Rmpfr in check.r. */

xsum_flt ten_term[] = {
Lnormal, Lnormal, Lnormal, Lnormal, Lnormal, Lnormal, -Lnormal, -Lnormal, -Lnormal, -Lnormal, 1.0/0.0,
Lnormal, Lnormal, Lnormal, Lnormal, 0.125, 0.125, -Lnormal, -Lnormal, -Lnormal, -Lnormal, 0.25,
2.0*(1+pow2_52), -2.0, -pow2_52, -pow2_52, 0, 0, 0, 0, 0, 0, 0,
1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1111111111e0,
1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 0.0, 1e8, 111111111e0,
1.234e88, -93.3e-23, 994.33, 1334.3, 457.34, -1.234e88, 93.3e-23, -994.33, -1334.3, -457.34, 0,
1., -23., 456., -78910., 1112131415., -161718192021., 22232425262728., -2930313233343536., 373839404142434445., -46474849505152535455., -46103918342424313856.,
2342423.3423, 34234.450, 945543.4, 34345.34343, 1232.343, 0.00004343, 43423.0, -342344.8343, -89544.3435, -34334.3, 2934978.4009734304,
0.9101534, 0.9048397, 0.4036596, 0.1460245, 0.2931254, 0.9647649, 0.1125303, 0.1574193, 0.6522300, 0.7378597, 5.2826068,
428.366070546, 707.3261930632, 103.29267289, 9040.03475821, 36.2121638, 19.307901408, 1.4810709160, 8.077159101, 1218.907244150, 778.068267017, 12341.0735011012,
1.1e-322, 5.3443e-321, -9.343e-320, 3.33e-314, 4.41e-322, -8.8e-318, 3.1e-310, 4.1e-300, -4e-300, 7e-307, 1.0000070031003328e-301,
};

/* Vectors of length two for testing dot product and squared norm.  These a
   of 'float' type so that casting to double and multiplying will give the
   exact product.  Adding the two terms will then give the correctly-rounded
   result to check against. */

float dot_term[] = {
  1.0f, 1.0f,
  -1.0f, 1.0f,
  0.3f, 1.4f,
  313.5f, -14.33f,
  0.0f, 0.0f,
  2.0f, 0.0f,
  0.0f, 2.0f,
  12.2f, 1.1e-20f,
  5.5e15f, -4.1,
};

#if 1
#define REP1 (1 << 23) /* Repeat factor for second set of one term tests */
#define REP10 (1 << 13) /* Repeat factor for second set of ten term tests */
#define REPDOT (1 << 13) /* Repeat factor for dot product / sqnorm tests */
#else
#define REP1 2 /* Small repetition count maybe sometimes useful for debugging */
#define REP10 2
#define REPDOT 2
#endif

int echo, debug_all, debug_letter, debug_number;

int different (double a, double b)
{ 
  return isnan(a) != isnan(b) || !isnan(a) && !isnan(b) && a != b;
}

void small_result (xsum_small_accumulator *sacc, double s, int i)
{
  double r, r2;
  if (xsum_debug) printf("SMALL RESULT:\n");
  if (xsum_debug) xsum_small_display(sacc);
  r = xsum_small_round (sacc);
  r2 = xsum_small_round (sacc);
  if (xsum_debug) xsum_small_display(sacc);
  if (different(r,r2))
  { printf("%3d small: Different second time %.16le %.16le\n", i, r, r2);
  }
  if (different(r,s))
  { printf("%3d small: Result incorrect %.16le %.16le\n", i, r, s);
    printf("    "); pbinary_double(r); printf("\n");
    printf("    "); pbinary_double(s); printf("\n");
  }
  if (xsum_debug) printf("END RESULT\n");
}

void large_result (xsum_large_accumulator *lacc, double s, int i)
{
  double r, r2;
  if (xsum_debug) printf("LARGE RESULT:\n");
  if (xsum_debug) xsum_large_display(lacc);
  r = xsum_large_round (lacc);
  r2 = xsum_large_round (lacc);
  if (xsum_debug) xsum_large_display(lacc);
  if (different(r,r2))
  { printf("%3d large: Different second time %.16le %.16le\n", i, r, r2);
  }
  if (different(r,s))
  { printf("%3d large: Result incorrect %.16le %.16le\n", i, r, s);
    printf("    "); pbinary_double(r); printf("\n");
    printf("    "); pbinary_double(s); printf("\n");
  }
  if (xsum_debug) printf("END RESULT\n");
}

void small_div_result (xsum_small_accumulator *sacc, double s, long val, int i)
{
  double r;
  if (xsum_debug) printf("SMALL RESULTS:\n");
  if (xsum_debug) xsum_small_display(sacc);
  if (val <= INT_MAX)
  { r = xsum_small_div_int (sacc, (int)val);
    if (different(r,s))
    { printf("%3d small int: Result incorrect %.16le %.16le\n", i, r, s);
      printf("    "); pbinary_double(r); printf("\n");
      printf("    "); pbinary_double(s); printf("\n");
    }
  }
  if (val >= 0)
  { r = xsum_small_div_unsigned (sacc, (unsigned)val);
    if (different(r,s))
    { printf("%3d sml unsgnd: Result incorrect %.16le %.16le\n", i, r, s);
      printf("    "); pbinary_double(r); printf("\n");
      printf("    "); pbinary_double(s); printf("\n");
    }
  }
  if (xsum_debug) printf("END RESULT\n");
}

void large_div_result (xsum_large_accumulator *lacc, double s, long val, int i)
{
  double r;
  if (xsum_debug) printf("LARGE RESULTS:\n");
  if (xsum_debug) xsum_large_display(lacc);
  if (val <= INT_MAX)
  { r = xsum_large_div_int (lacc, (int)val);
    if (different(r,s))
    { printf("%3d large int: Result incorrect %.16le %.16le\n", i, r, s);
      printf("    "); pbinary_double(r); printf("\n");
      printf("    "); pbinary_double(s); printf("\n");
    }
  }
  if (val >= 0)
  { r = xsum_large_div_unsigned (lacc, (unsigned)val);
    if (different(r,s))
    { printf("%3d lrg unsgnd: Result incorrect %.16le %.16le\n", i, r, s);
      printf("    "); pbinary_double(r); printf("\n");
      printf("    "); pbinary_double(s); printf("\n");
    }
  }
  if (xsum_debug) printf("END RESULT\n");
}

int main (int argc, char **argv)
{
  xsum_small_accumulator sacc, sacc2;
  xsum_large_accumulator lacc, lacc2;
  int tstno;
  double s;
  char section;
  int done;
  int i, j;

  if (argc>2 || argc==2 && strcmp(argv[1],"-e")!=0 
                        && (argv[1][0]!='-' || argv[1][1]!='d'))
  { fprintf(stderr,"Usage: xsum-check [ -e | -d[L#] ]\n");
    exit(1);
  }
 
  echo = argc > 1;
  debug_all = debug_letter = debug_number = 0;
  if (argc > 1 && argv[1][1]=='d')
  { debug_all = argv[1][2]==0;
    if (!debug_all)
    { debug_letter = argv[1][2];
      debug_number = atoi(argv[1]+3);
    }
  }

  /* Print out special values in binary, if enabled, to check that they are
     as intended. */

  if (0)
  { printf("1:\n  "); pbinary_double(1.0); printf("\n");
    printf("pow2_16:\n  "); pbinary_double(pow2_16); printf("\n");
    printf("pow2_52:\n  "); pbinary_double(pow2_52); printf("\n");
    printf("1/pow2_52:\n  "); pbinary_double(1/pow2_52); printf("\n");
    printf("pow2_1024:\n  "); pbinary_double(pow2_1024); printf("\n");
    printf("Lnormal:\n  "); pbinary_double(Lnormal); printf("\n");
    printf("Snormal:\n  "); pbinary_double(Snormal); printf("\n");
    printf("Ldenorm:\n  "); pbinary_double(Ldenorm); printf("\n");
    printf("Sdenorm:\n  "); pbinary_double(Sdenorm); printf("\n");
    printf("(1L<<55)+8.0:\n  "); pbinary_double((1L<<55)+8.0); printf("\n");
    printf("\n");
  }

  /* On an Intel machine, set the 387 FPU to do double rounding, in order
     to get correct IEEE 64-bit floating point.  (Only relevant if SSE2
     instructions not used instead of FPU.)  This will disable higher
     precision for long double, however! */

# ifdef DOUBLE
  { unsigned int mode = 0x27f;
    __asm__ ("fldcw %0" : : "m" (*&mode));
  }
# endif

  /* Check that the 'different' function works. */

  if (different(3.1,3.1) 
   || different(1.0/0.0,2.0/0.0) 
   || different(-1.0/0.0,-2.0/0.0)
   || different(0.0/0.0,0.0/0.0)
   || different(0.0,-0.0)
   || !different(3.1,3.2)
   || !different(-1.0/0.0,1.0/0.0)
   || !different(1.0/0.0,0.0/0.0))
  { printf("!! 'different' function not working !!\n");
  }

  xsum_flt *repten = (xsum_flt*) calloc (10*REP10, sizeof *repten);

  printf("\nCORRECTNESS TESTS\n");

  section = 'A';

  printf("\n%c: ZERO TESTS\n",section);

  if (echo) printf(" \n-- TEST 0: \n");
  if (echo) printf("   ANSWER:  %.16le\n",0.0);

  xsum_debug = debug_all || debug_letter==section;

  xsum_small_init (&sacc);
  small_result(&sacc,0,0);

  xsum_large_init (&lacc);
  large_result(&lacc,0,0);

  xsum_small_init (&sacc);
  xsum_small_addv (&sacc, NULL, 0);
  small_result(&sacc,0,0);

  xsum_large_init (&lacc);
  xsum_large_addv (&lacc, NULL, 0);
  large_result(&lacc,0,0);

  xsum_small_init (&sacc);
  xsum_small_add1 (&sacc, 0.0);
  small_result(&sacc,0,0);

  xsum_large_init (&lacc);
  xsum_large_add1 (&lacc, 0.0);
  large_result(&lacc,0,0);

  xsum_small_init (&sacc);
  xsum_small_add1 (&sacc, -0.0);
  small_result(&sacc,0,0);

  xsum_large_init (&lacc);
  xsum_large_add1 (&lacc, -0.0);
  large_result(&lacc,0,0);

  printf("\n%c: ONE TERM TESTS\n",++section);

  for (i = 0; i < sizeof one_term / sizeof *one_term; i += 1)
  { 
    if (echo) printf(" \n-- TEST %2d: %.16le\n",i,one_term[i]);
    s = one_term[i];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i;

    xsum_small_init (&sacc);
    xsum_small_add1 (&sacc, s);
    small_result(&sacc,s,i);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, &s, 1);
    large_result(&lacc,s,i);
  }

  printf("\n%c: ONE TERM TESTS, NEGATED\n",++section);

  for (i = 0; i < sizeof one_term / sizeof *one_term; i += 1)
  { 
    if (echo) printf(" \n-- TEST %2d: %.16le\n",i,-one_term[i]);
    s = -one_term[i];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i;

    xsum_small_init (&sacc);
    xsum_small_add1 (&sacc, s);
    small_result(&sacc,s,i);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, &s, 1);
    large_result(&lacc,s,i);
  }

  printf("\n%c: ONE TERM TESTS TIMES %d\n",++section,REP1);

  for (i = 0; i < sizeof one_term / sizeof *one_term; i += 1)
  { 
    double v = one_term[i];
    if (echo) printf(" \n-- TEST %2d: %.16le\n",i,v);
    s = v * REP1;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i;

    xsum_small_init (&sacc);
    for (j = 0; j < REP1; j++) xsum_small_add1 (&sacc, v);
    small_result(&sacc,s,i);

    xsum_large_init (&lacc);
    for (j = 0; j < REP1; j++) xsum_large_addv (&lacc, &v, 1);
    large_result(&lacc,s,i);
  }

  printf("\n%c: ONE TERM TESTS TIMES %d, NEGATED\n",++section,REP1);

  for (i = 0; i < sizeof one_term / sizeof *one_term; i += 1)
  { 
    double v = -one_term[i];
    if (echo) printf(" \n-- TEST %2d: %.16le\n",i,v);
    s = v * REP1;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i;

    xsum_small_init (&sacc);
    for (j = 0; j < REP1; j++) xsum_small_add1 (&sacc, v);
    small_result(&sacc,s,i);

    xsum_large_init (&lacc);
    for (j = 0; j < REP1; j++) xsum_large_addv (&lacc, &v, 1);
    large_result(&lacc,s,i);
  }

  printf("\n%c: TWO TERM TESTS\n",++section);

  for (i = 0; i < sizeof two_term / sizeof *two_term; i += 2)
  { 
    double v[2] = { two_term[i], two_term[i+1] };
    if (echo) printf(" \n-- TEST %2d: %.16le %.16le\n",i/2,v[0],v[1]);
    s = v[0]+v[1];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i/2;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 2);
    small_result(&sacc,s,i/2);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 2);
    large_result(&lacc,s,i/2);
  }

  printf("\n%c: TWO TERM TESTS, NEGATED\n",++section);

  for (i = 0; i < sizeof two_term / sizeof *two_term; i += 2)
  { 
    double v[2] = { -two_term[i], -two_term[i+1] };
    if (echo) printf(" \n-- TEST %2d: %.16le %.16le\n",i/2,v[0],v[1]);
    s = v[0]+v[1];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i/2;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 2);
    small_result(&sacc,s,i/2);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 2);
    large_result(&lacc,s,i/2);
  }

  printf("\n%c: TWO TERM TESTS, WITH MULTIPLYING FACTORS\n",++section);

  static double factors[] = { /* must be powers of two, so multiply is exact */
    2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
    1, 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625,
    -2, -4, -8, -16, -32, -64, -128, -256, -512, -1024, -2048, -4096, -8192,
    -1, -0.5, -0.25, -0.125, -0.0625, -0.03125, -0.015625,
  0 };

  tstno = 0;
  for (i = 0; i < sizeof two_term / sizeof *two_term; i += 2)
  { for (j = 0; factors[j] != 0; j++)
    { 
      double v[2] = { factors[j]*two_term[i], factors[j]*two_term[i+1] };
      if (echo) printf(" \n-- TEST %2d: %.16le %.16le\n",tstno,v[0],v[1]);
      s = v[0]+v[1];
      if (echo) printf("   ANSWER:  %.16le\n",s);

      xsum_debug = debug_all || debug_letter==section && debug_number==i/2;

      xsum_small_init (&sacc);
      xsum_small_addv (&sacc, v, 2);
      small_result(&sacc,s,i/2);

      xsum_large_init (&lacc);
      xsum_large_addv (&lacc, v, 2);
      large_result(&lacc,s,i/2);

      tstno += 1;
    }
  }

  printf("\n%c: TWO TERM TESTS, WITH MULTIPLYING FACTORS, AND +-EXTRAS\n",
          ++section);

  tstno = 0;
  for (i = 0; i < sizeof two_term / sizeof *two_term; i += 2)
  { for (j = 0; factors[j] != 0; j++)
    { 
      double v[2] = { factors[j]*two_term[i], factors[j]*two_term[i+1] };
      if (echo) printf(" \n-- TEST %2d: %.16le %.16le\n",tstno,v[0],v[1]);
      s = v[0]+v[1];
      if (echo) printf("   ANSWER:  %.16le\n",s);

      xsum_debug = debug_all || debug_letter==section && debug_number==i/2;

      xsum_small_init (&sacc);
      xsum_small_add1 (&sacc, 123.4e200);
      xsum_small_add1 (&sacc, -543.2e-200);
      xsum_small_addv (&sacc, v, 2);
      xsum_small_add1 (&sacc, -123.4e200);
      xsum_small_add1 (&sacc, 543.2e-200);
      small_result(&sacc,s,i/2);

      xsum_large_init (&lacc);
      xsum_large_add1 (&lacc, 123.4e200);
      xsum_large_add1 (&lacc, -543.2e-200);
      xsum_large_addv (&lacc, v, 2);
      xsum_large_add1 (&lacc, -123.4e200);
      xsum_large_add1 (&lacc, 543.2e-200);
      large_result(&lacc,s,i/2);

      tstno += 1;
    }
  }

  printf("\n%c: THREE TERM TESTS\n",++section);

  for (i = 0; i < sizeof three_term / sizeof *three_term; i += 4)
  { 
    double v[3] = { three_term[i], three_term[i+1], three_term[i+2] };
    if (echo) printf(" \n-- TEST %2d: %.16le %.16le %.16le\n",
                      i/4,v[0],v[1],v[2]);
    s = three_term[i+3];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i/4;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 3);
    small_result(&sacc,s,i/4);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 3);
    large_result(&lacc,s,i/4);
  }

  printf("\n%c: THREE TERM TESTS, NEGATED\n",++section);

  for (i = 0; i < sizeof three_term / sizeof *three_term; i += 4)
  { 
    double v[3] = { -three_term[i], -three_term[i+1], -three_term[i+2] };
    if (echo) printf(" \n-- TEST %2d: %.16le %.16le %.16le\n",
                      i/4,v[0],v[1],v[2]);
    s = -three_term[i+3];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i/4;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 3);
    small_result(&sacc,s,i/4);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 3);
    large_result(&lacc,s,i/4);
  }

  printf("\n%c: TEN TERM TESTS\n",++section);

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { ten_term[i+0], ten_term[i+1], ten_term[i+2], 
      ten_term[i+3], ten_term[i+4], ten_term[i+5], 
      ten_term[i+6], ten_term[i+7], ten_term[i+8],
      ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",i/11);
    s = ten_term[i+10];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i/11;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 10);
    small_result(&sacc,s,i/11);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 10);
    large_result(&lacc,s,i/11);
  }

  printf("\n%c: TEN TERM TESTS, NEGATED\n",++section);

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { -ten_term[i+0], -ten_term[i+1], -ten_term[i+2], 
      -ten_term[i+3], -ten_term[i+4], -ten_term[i+5], 
      -ten_term[i+6], -ten_term[i+7], -ten_term[i+8],
      -ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",i/11);
    s = -ten_term[i+10];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==i/11;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 10);
    small_result(&sacc,s,i/11);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 10);
    large_result(&lacc,s,i/11);
  }

  printf("\n%c: TEN TERM TESTS TIMES %d\n",++section,REP10);

  tstno = 0;

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { ten_term[i+0], ten_term[i+1], ten_term[i+2], 
      ten_term[i+3], ten_term[i+4], ten_term[i+5], 
      ten_term[i+6], ten_term[i+7], ten_term[i+8],
      ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = ten_term[i+10] * REP10;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    xsum_small_init (&sacc);
    for (j = 0; j < REP10; j++) xsum_small_addv (&sacc, v, 10);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    for (j = 0; j < REP10; j++) xsum_large_addv (&lacc, v, 10);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { ten_term[i+0], ten_term[i+1], ten_term[i+2], 
      ten_term[i+3], ten_term[i+4], ten_term[i+5], 
      ten_term[i+6], ten_term[i+7], ten_term[i+8],
      ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = ten_term[i+10] * REP10;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    for (j = 0; j < 10*REP10; j++)
    { repten[j] = v[j%10];
    }

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, repten, 10*REP10);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, repten, 10*REP10);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  printf("\n%c: TEN TERM TESTS TIMES %d, NEGATED\n",++section,REP10);

  tstno = 0;

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { -ten_term[i+0], -ten_term[i+1], -ten_term[i+2], 
      -ten_term[i+3], -ten_term[i+4], -ten_term[i+5], 
      -ten_term[i+6], -ten_term[i+7], -ten_term[i+8],
      -ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = -ten_term[i+10] * REP10;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    xsum_small_init (&sacc);
    for (j = 0; j < REP10; j++) xsum_small_addv (&sacc, v, 10);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    for (j = 0; j < REP10; j++) xsum_large_addv (&lacc, v, 10);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { -ten_term[i+0], -ten_term[i+1], -ten_term[i+2], 
      -ten_term[i+3], -ten_term[i+4], -ten_term[i+5], 
      -ten_term[i+6], -ten_term[i+7], -ten_term[i+8],
      -ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = -ten_term[i+10] * REP10;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    for (j = 0; j < 10*REP10; j++)
    { repten[j] = v[j%10];
    }

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, repten, 10*REP10);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, repten, 10*REP10);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  printf("\n%c: TESTS OF ADDING TOGETHER ACCUMULATORS\n",++section);

  done = 0;
  for (i = 0; !done; i += 1)
  { 
    xsum_debug = debug_all || debug_letter==section && debug_number==i;
    if (echo) printf(" \n-- TEST %2d\n",i);
    s = 1234.5;

    xsum_small_init (&sacc);
    xsum_small_init (&sacc2);
    xsum_large_init (&lacc);
    xsum_large_init (&lacc2);

    switch (i)
    { case 0:  /* add one small/large accumulator to another */
      { if (echo) printf("   ANSWER:  %.16le\n",s);
        double v1[3] = { 3.7e20, 888.8, 4.1e20 };
        double v2[4] = { s, -4.1e20, -3.7e20, -888.8 };
        xsum_small_addv (&sacc, v1, 3);
        xsum_small_addv (&sacc2, v2, 4);
        xsum_small_add_accumulator (&sacc, &sacc2);
        xsum_large_addv (&lacc, v1, 3);
        xsum_large_addv (&lacc2, v2, 4);
        xsum_large_add_accumulator (&lacc, &lacc2);
        small_result(&sacc,s,i);
        large_result(&lacc,s,i);
        break;
      }
      case 1:  /* add a small/large accumulator many times */
      { if (echo) printf("   ANSWER:  %.16le\n",s);
        double v1[3] = { 3.5, 888.75, 4.125 };
        double v2[3] = { -4.125, -3.5, -888.75 };
        int k;

        xsum_small_addv (&sacc2, v1, 3);
        for (k = 0; k < 1000; k++) xsum_small_add_accumulator (&sacc, &sacc2);
        xsum_small_add1 (&sacc, s);
        xsum_small_addv (&sacc2, v2, 3);
        xsum_small_addv (&sacc2, v2, 3);
        for (k = 0; k < 1000; k++) xsum_small_add_accumulator (&sacc, &sacc2);

        xsum_large_addv (&lacc2, v1, 3);
        for (k = 0; k < 1000; k++) xsum_large_add_accumulator (&lacc, &lacc2);
        xsum_large_add1 (&lacc, s);
        xsum_large_addv (&lacc2, v2, 3);
        xsum_large_addv (&lacc2, v2, 3);
        for (k = 0; k < 1000; k++) xsum_large_add_accumulator (&lacc, &lacc2);

        small_result(&sacc,s,i);
        large_result(&lacc,s,i);

        break;
      }
      case 2:  /* add a small/large accumulator many times, producing +inf */
      { if (echo) printf("   ANSWER:  +inf\n");

        double v1[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isinf(r) || r < 0) 
        { printf("%d small: Result not +inf: %.16le\n",i,r);
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isinf(r) || r < 0) 
        { printf("%d large: Result not +inf: %.16le\n",i,r);
        }

        break;
      }
      case 3:  /* add a small/large accumulator MANY times, producing NaN */
      { if (echo) printf("   ANSWER:  NaN\n");

        double v1[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isnan(r))
        { printf("%d small: Result not NaN: %.16le\n",i,r);
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isnan(r))
        { printf("%d large: Result not NaN: %.16le\n",i,r);
        }

        break;
      }
      case 4:  /* add a small/large accumulator many times, producing -inf */
      { if (echo) printf("   ANSWER:  -inf\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isinf(r) || r > 0) 
        { printf("%d small: Result not -inf: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isinf(r) || r > 0) 
        { printf("%d large: Result not -inf: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        break;
      }
      case 5:  /* add a small/large accumulator MANY times, producing NaN */
      { if (echo) printf("   ANSWER:  NaN\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        r = xsum_small_round(&sacc);
        if (!isnan(r))
        { printf("%d small: Result not NaN: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        xsum_large_addv (&lacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_large_accumulator t = lacc;
          xsum_large_add_accumulator (&lacc, &t);
        }

        r = xsum_large_round(&lacc);
        if (!isnan(r))
        { printf("%d large: Result not NaN: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        break;
      }
      case 6:  /* add a small/large accumulator many times, producing zero */
      { if (echo) printf("   ANSWER:  0\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double v2[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        xsum_small_addv (&sacc2, v2, 3);
        for (k = 0; k < 1015; k++)
        { xsum_small_accumulator t = sacc2;
          xsum_small_add_accumulator (&sacc2, &t);
        }

        xsum_small_add_accumulator (&sacc, &sacc2);

        r = xsum_small_round(&sacc);
        if (isnan(r) || r != 0)
        { printf("%d small: Result not 0: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        break;
      }
      case 7:  /* add a small/large accumulator MANY times, producing NaN */
      { if (echo) printf("   ANSWER:  NaN\n");

        double v1[3] = { -3.5, -888.75, -4.125 };
        double v2[3] = { 3.5, 888.75, 4.125 };
        double r;
        int k;

        xsum_small_addv (&sacc, v1, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc;
          xsum_small_add_accumulator (&sacc, &t);
        }

        xsum_small_addv (&sacc2, v2, 3);
        for (k = 0; k < 2000; k++)
        { xsum_small_accumulator t = sacc2;
          xsum_small_add_accumulator (&sacc2, &t);
        }

        xsum_small_add_accumulator (&sacc, &sacc2);

        r = xsum_small_round(&sacc);
        if (!isnan(r))
        { printf("%d small: Result not NaN: %.16le\n",i,r);
          printf("    "); pbinary_double(r); printf("\n");
        }

        break;
      }
      case 8:  /* add a small/large accumulator to itself */
      { if (echo) printf("   ANSWER:  %.16le\n",s);
        double v1[4] = { 3.7e20, 888.8, 42.1e20, s };
        double v2[4] = { -42.1e20*2, -3.7e20*2, -888.8*2, -s };
        xsum_small_addv (&sacc, v1, 4);
        xsum_small_add_accumulator (&sacc, &sacc);
        xsum_small_addv (&sacc, v2, 4);
        xsum_large_addv (&lacc, v1, 4);
        xsum_large_add_accumulator (&lacc, &lacc);
        xsum_large_addv (&lacc, v2, 4);
        small_result(&sacc,s,i);
        large_result(&lacc,s,i);
        done = 1;
      }
    }
  }

  printf("\n%c: TESTS INVOLVING NEGATION\n",++section);

  tstno = 0;

  if (echo) printf(" \n-- TEST %2d\n",tstno);
  s = -1.0/0.0;
  if (echo) printf("   ANSWER:  %.16le\n",s);

  xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

  xsum_small_init (&sacc);
  xsum_small_add1 (&sacc, 1.0/0.0);
  xsum_small_negate (&sacc);
  small_result(&sacc,s,tstno);

  xsum_large_init (&lacc);
  xsum_large_add1 (&lacc, 1.0/0.0);
  xsum_large_negate (&lacc);
  large_result(&lacc,s,tstno);

  tstno += 1;

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { ten_term[i+0], ten_term[i+1], ten_term[i+2], 
      ten_term[i+3], ten_term[i+4], ten_term[i+5], 
      ten_term[i+6], ten_term[i+7], ten_term[i+8],
      ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = -ten_term[i+10];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 10);
    xsum_small_negate (&sacc);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 10);
    xsum_large_negate (&lacc);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { ten_term[i+0], ten_term[i+1], ten_term[i+2], 
      ten_term[i+3], ten_term[i+4], ten_term[i+5], 
      ten_term[i+6], ten_term[i+7], ten_term[i+8],
      ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = 123456789;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 10);
    xsum_small_negate (&sacc);
    xsum_small_add1 (&sacc, s);
    xsum_small_addv (&sacc, v, 10);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 10);
    xsum_large_negate (&lacc);
    xsum_large_add1 (&lacc, s);
    xsum_large_addv (&lacc, v, 10);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { -ten_term[i+0], -ten_term[i+1], -ten_term[i+2], 
      -ten_term[i+3], -ten_term[i+4], -ten_term[i+5], 
      -ten_term[i+6], -ten_term[i+7], -ten_term[i+8],
      -ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = ten_term[i+10];
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 10);
    xsum_small_negate (&sacc);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 10);
    xsum_large_negate (&lacc);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { -ten_term[i+0], -ten_term[i+1], -ten_term[i+2], 
      -ten_term[i+3], -ten_term[i+4], -ten_term[i+5], 
      -ten_term[i+6], -ten_term[i+7], -ten_term[i+8],
      -ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = 123456789;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, v, 10);
    xsum_small_negate (&sacc);
    xsum_small_add1 (&sacc, s);
    xsum_small_addv (&sacc, v, 10);
    small_result(&sacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, v, 10);
    xsum_large_negate (&lacc);
    xsum_large_add1 (&lacc, s);
    xsum_large_addv (&lacc, v, 10);
    large_result(&lacc,s,tstno);

    tstno += 1;
  }

  printf("\n%c: TESTS ON TEN TERMS WITH ACCUMULATOR ADDITION AND TRANSFER\n",
         ++section);

  tstno = 0;

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { ten_term[i+0], ten_term[i+1], ten_term[i+2], 
      ten_term[i+3], ten_term[i+4], ten_term[i+5], 
      ten_term[i+6], ten_term[i+7], ten_term[i+8],
      ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = ten_term[i+10] * REP10;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    for (j = 0; j < 10*REP10; j++)
    { repten[j] = v[j%10];
    }

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, repten, 9);
    xsum_small_accumulator sacc2 = sacc;
    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, repten+9, 10*REP10-9);
    xsum_small_add_accumulator (&sacc, &sacc2);
    small_result(&sacc,s,tstno);
    xsum_small_to_large_accumulator (&lacc, &sacc);
    large_result(&lacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, repten, 9);
    xsum_large_accumulator lacc2 = lacc;
    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, repten+9, 10*REP10-9);
    xsum_large_add_accumulator (&lacc, &lacc2);
    large_result(&lacc,s,tstno);
    xsum_large_to_small_accumulator (&sacc, &lacc);
    small_result(&sacc,s,tstno);

    tstno += 1;
  }

  for (i = 0; i < sizeof ten_term / sizeof *ten_term; i += 11)
  { 
    double v[10] = 
    { -ten_term[i+0], -ten_term[i+1], -ten_term[i+2], 
      -ten_term[i+3], -ten_term[i+4], -ten_term[i+5], 
      -ten_term[i+6], -ten_term[i+7], -ten_term[i+8],
      -ten_term[i+9]
    };
    if (echo) printf(" \n-- TEST %2d\n",tstno);
    s = -ten_term[i+10] * REP10;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

    for (j = 0; j < 10*REP10; j++)
    { repten[j] = v[j%10];
    }

    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, repten, 9);
    xsum_small_accumulator sacc2 = sacc;
    xsum_small_init (&sacc);
    xsum_small_addv (&sacc, repten+9, 10*REP10-9);
    xsum_small_add_accumulator (&sacc, &sacc2);
    small_result(&sacc,s,tstno);
    xsum_small_to_large_accumulator (&lacc, &sacc);
    large_result(&lacc,s,tstno);

    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, repten, 9);
    xsum_large_accumulator lacc2 = lacc;
    xsum_large_init (&lacc);
    xsum_large_addv (&lacc, repten+9, 10*REP10-9);
    xsum_large_add_accumulator (&lacc, &lacc2);
    large_result(&lacc,s,tstno);
    xsum_large_to_small_accumulator (&sacc, &lacc);
    small_result(&sacc,s,tstno);

    tstno += 1;
  }

  printf("\n%c: SPECIAL TESTS\n",++section);

  done = 0;
  for (i = 0; !done; i += 1)
  { 
    xsum_debug = debug_all || debug_letter==section && debug_number==i;
    if (echo) printf(" \n-- TEST %2d\n",i);
    s = 1234.5;
    if (echo) printf("   ANSWER:  %.16le\n",s);

    xsum_small_init (&sacc);
    xsum_large_init (&lacc);

    switch (i)
    { case 0:  /* add positive zero to 1234.5 */
      { double v[2] = { s, zero };
        xsum_small_addv (&sacc, v, 2);
        xsum_large_addv (&lacc, v, 2);
        break;
      }
      case 1:  /* add negative zero to 1234.5 */
      { double v[2] = { s, -zero };
        xsum_small_addv (&sacc, v, 2);
        xsum_large_addv (&lacc, v, 2);
        break;
      }
      case 2:  /* cause simulataneous moves of pair from large to small */
      { double v[20001];
        int j;
        for (j = 0; j < 10000; j += 2)
        { v[j] = 4567;
          v[j+1] = 10000;
        }
        for (j = 10000; j < 20000; j += 2)
        { v[j] = -4567;
          v[j+1] = -10000;
        }
        v[20000] = s;
        xsum_small_addv (&sacc, v, 20001);
        xsum_large_addv (&lacc, v, 20001);
        break;
      }
      case 3:  /* cause move of pair from large to small same time as init */
      { double v [2 * (1 << XSUM_LCOUNT_BITS) + 3];
        int j;
        for (j = 0; j < (1 << XSUM_LCOUNT_BITS); j += 1)
        { v[j] = 2345;
        }
        v [(1 << XSUM_LCOUNT_BITS)] = 2345;
        v [(1 << XSUM_LCOUNT_BITS) + 1] = s;
        for (j = (1 << XSUM_LCOUNT_BITS) + 2; 
             j < 2 * (1 << XSUM_LCOUNT_BITS) + 3; 
             j += 1)
        { v[j] = -2345;
        }
        xsum_small_addv (&sacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        xsum_large_addv (&lacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        break;
      }
      case 4:  /* as above, but with order of pair involved reversed */
      { double v [2 * (1 << XSUM_LCOUNT_BITS) + 3];
        int j;
        for (j = 0; j < (1 << XSUM_LCOUNT_BITS); j += 1)
        { v[j] = 2345;
        }
        v [(1 << XSUM_LCOUNT_BITS)] = s;
        v [(1 << XSUM_LCOUNT_BITS) + 1] = 2345;
        for (j = (1 << XSUM_LCOUNT_BITS) + 2; 
             j < 2 * (1 << XSUM_LCOUNT_BITS) + 3; 
             j += 1)
        { v[j] = -2345;
        }
        xsum_small_addv (&sacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        xsum_large_addv (&lacc, v, 2 * (1 << XSUM_LCOUNT_BITS) + 3);
        break;
      }
      case 5:  /* test transfer to small accumulator */
      { double v[3] = { 5100, s, -5100 };
        xsum_large_addv (&lacc, v, 3);
        xsum_large_to_small_accumulator (&sacc, &lacc);
        break;
      }
      case 6:  /* test transfer to large accumulator */
      { double v[3] = { 5100, s, -5100 };
        xsum_small_addv (&sacc, v, 3);
        xsum_small_to_large_accumulator (&lacc, &sacc);
        done = 1;
      }
    }

    small_result(&sacc,s,i);
    large_result(&lacc,s,i);
  }

  printf("\n%c: TESTS OF SQUARED NORM\n",++section);

  tstno = 0;
  for (i = 0; i < sizeof dot_term / sizeof *dot_term; i += 2)
  { int si, k;
    for (si = -1; si <= 1; si+=2)
    { double vi[3] = { 0.0, (double)si*dot_term[i], (double)si*dot_term[i+1] };
      if (echo) printf(" \n-- TEST %2d\n",tstno);
      s = (double)vi[1]*vi[1] + (double)vi[2]*vi[2];
      if (echo) printf("   ANSWER:  %.16le\n",s);

      xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

      xsum_small_init (&sacc);
      xsum_small_add_sqnorm (&sacc, vi+1, 2);
      small_result(&sacc,s,tstno);

      xsum_large_init (&lacc);
      xsum_large_add_sqnorm (&lacc, vi+1, 2);
      large_result(&lacc,s,tstno);

      tstno += 1;

      if (echo) printf(" \n-- TEST %2d\n",tstno);
      s = (double)vi[1]*vi[1] + (double)vi[2]*vi[2];
      if (echo) printf("   ANSWER:  %.16le\n",s);

      xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

      xsum_small_init (&sacc);
      xsum_small_add_sqnorm (&sacc, vi, 3);
      small_result(&sacc,s,tstno);

      xsum_large_init (&lacc);
      xsum_large_add_sqnorm (&lacc, vi, 3);
      large_result(&lacc,s,tstno);

      tstno += 1;

      if (echo) printf(" \n-- TEST %2d\n",tstno);
      s = REPDOT * ((double)vi[1]*vi[1] + (double)vi[2]*vi[2]);
      if (echo) printf("   ANSWER:  %.16le\n",s);

      xsum_debug = debug_all || debug_letter==section && debug_number==tstno;

      xsum_small_init (&sacc);
      for (k = 0; k < REPDOT; k++) xsum_small_add_sqnorm (&sacc, vi+1, 2);
      small_result(&sacc,s,tstno);

      xsum_large_init (&lacc);
      for (k = 0; k < REPDOT; k++) xsum_large_add_sqnorm (&lacc, vi+1, 2);
      large_result(&lacc,s,tstno);

      tstno += 1;
    }
  }

  printf("\n%c: TESTS OF DOT PRODUCT\n",++section);

  tstno = 0;
  for (i = 0; i < sizeof dot_term / sizeof *dot_term; i += 2)
  { int si, sj, k;
    for (si = -1; si <= 1; si+=2)
    { double vi[3] = { 0.0, (double)si*dot_term[i], (double)si*dot_term[i+1] };
      for (j = 0; dot_term[j] != 0; j += 2)
      { for (sj = -1; sj <= 1; sj += 2)
        { double vj[3] = {0.0,(double)sj*dot_term[j],(double)sj*dot_term[j+1] };
          if (echo) printf(" \n-- TEST %2d\n",tstno);
          s = (double)vi[1]*vj[1] + (double)vi[2]*vj[2];
          if (echo) printf("   ANSWER:  %.16le\n",s);

          xsum_debug = debug_all || debug_letter==section&&debug_number==tstno;

          xsum_small_init (&sacc);
          xsum_small_add_dot (&sacc, vi+1, vj+1, 2);
          small_result(&sacc,s,tstno);

          xsum_large_init (&lacc);
          xsum_large_add_dot (&lacc, vi+1, vj+1, 2);
          large_result(&lacc,s,tstno);

          tstno += 1;

          if (echo) printf(" \n-- TEST %2d\n",tstno);
          s = (double)vi[1]*vj[1] + (double)vi[2]*vj[2];
          if (echo) printf("   ANSWER:  %.16le\n",s);

          xsum_debug = debug_all || debug_letter==section&&debug_number==tstno;

          xsum_small_init (&sacc);
          xsum_small_add_dot (&sacc, vi, vj, 3);
          small_result(&sacc,s,tstno);

          xsum_large_init (&lacc);
          xsum_large_add_dot (&lacc, vi, vj, 3);
          large_result(&lacc,s,tstno);

          tstno += 1;

          if (echo) printf(" \n-- TEST %2d\n",tstno);
          s = REPDOT * ((double)vi[1]*vj[1] + (double)vi[2]*vj[2]);
          if (echo) printf("   ANSWER:  %.16le\n",s);

          xsum_debug = debug_all || debug_letter==section&&debug_number==tstno;

          xsum_small_init (&sacc);
          for (k = 0; k < REPDOT; k++) xsum_small_add_dot(&sacc, vi+1, vj+1, 2);
          small_result(&sacc,s,tstno);

          xsum_large_init (&lacc);
          for (k = 0; k < REPDOT; k++) xsum_large_add_dot(&lacc, vi+1, vj+1, 2);
          large_result(&lacc,s,tstno);

          tstno += 1;
        }
      }
    }
  }

  printf("\n%c: TESTS OF DIVISION OF ONE TERM BY VARIOUS DIVISORS\n",++section);

  for (i = 0; i < sizeof one_term / sizeof *one_term; i += 1)
  {
    long div[] = { 0, 1, 2, 3, 255, 256, 257, 32766, 32767, 32768, 32769,
                   2000000000, 4000000000, 2000000001, 4000000001, 3210987654,
                   INT_MAX, INT_MIN, UINT_MAX, INT_MAX-1, INT_MIN+1, UINT_MAX-1
                 };

    int w, sign;

    sign = 1;
    do 
    { for (w = 0; w < sizeof div / sizeof *div; w++)
      { long val = sign*div[w];
        if (val < INT_MIN)
        { continue;
        }
        if (echo) printf(" \n-- TEST %2d: %.16le / %ld\n",i,one_term[i],val);
        s = one_term[i];
        if (echo) printf("   ANSWER:  %.16le\n",s/val);

        xsum_debug = debug_all || debug_letter==section && debug_number==i;

        xsum_small_init (&sacc);
        xsum_small_add1 (&sacc, s);
        small_div_result(&sacc,s/val,val,i);

        xsum_large_init (&lacc);
        xsum_large_add1 (&lacc, s);
        large_div_result(&lacc,s/val,val,i);
      }
      sign = -sign;
    } while (sign == -1);
  }

  printf("\n%c: TESTS OF DIVISION WITH TWO TERMS BY POWERS OF TWO\n",++section);

  for (i = 0; i < sizeof two_term / sizeof *two_term; i += 2)
  {
    long divp2[] = { 1, 2, 4, 8, 256, 32768, 2147483648 };
    int w, sign;

    sign = 1;
    do 
    { for (w = 0; w < sizeof divp2 / sizeof *divp2; w++)
      { long val = sign*divp2[w];
        if (val < INT_MIN)
        { continue;
        }
        if (echo) printf(" \n-- TEST %2d: %.16le %.16le */ %ld\n",
                          i,two_term[i],two_term[i+1],val);
        s = (val*two_term[i] + val*two_term[i+1]) / val;
        if (echo) printf("   ANSWER:  %.16le\n",s/val);

        xsum_debug = debug_all || debug_letter==section && debug_number==i;

        xsum_small_init (&sacc);
        xsum_small_add1 (&sacc, val*two_term[i]);
        xsum_small_add1 (&sacc, val*two_term[i+1]);
        small_div_result(&sacc,s,val,i);

        xsum_large_init (&lacc);
        xsum_large_add1 (&lacc, val*two_term[i]);
        xsum_large_add1 (&lacc, val*two_term[i+1]);
        large_div_result(&lacc,s,val,i);
      }
      sign = -sign;
    } while (sign == -1);
  }
  
  printf("\nDONE\n\n");

  free(repten);
  
  return 0;
}
