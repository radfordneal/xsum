/* TEST PROGRAM FOR FUNCTIONS FOR EXACT SUMMATION. */

/* Copyright 2015, 2018, 2024 Radford M. Neal

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


/* Manual test program.  Reads lines containing one or more numbers
   (terminating on EOF), and prints these numbers and their sum, as 
   computed with small and large superaccumulators, and with simple 
   long double and double arithmetic. A number may be followed by 
   *times, in which case it is added "times" times. If invoked with 
   an argument, debug output is enabled. The line may end with '/'
   followed by an integer, in which case the result of dividing the
   sum by that integer is printed. */

#include <stdio.h>
#include <limits.h>
#include "xsum.h"
#include "pbinary.h"

#define MAXLINE 10000

int main (int argc, char **argv)
{
  xsum_small_accumulator sacc;
  xsum_large_accumulator lacc;
  xsum_flt result_s, result_l;
  long double ls;
  double s;
  double v;
  char in[MAXLINE+1];
  char *inp;
  int c, p, r, N;
  int have_div;
  long div;
 
  xsum_debug = argc > 1;

  for (;;)
  { 
    printf("\n> ");
    inp = in;
    while ((c = getchar()) != EOF && c != '\n')
    { *inp++ = c;
      if (inp==in+MAXLINE)
      { printf("\nLine too long - ignored\n");
        break;
      }
    }
    if (c == EOF && inp==in) 
    { printf("\n"); return 0;
    }
    if (inp==in+MAXLINE)
    { continue;
    }
    *inp = 0;
    inp = in; p = 0;

    xsum_small_init(&sacc);
    xsum_large_init(&lacc);
    ls = s = 0;
    N = 0;
    have_div = 0;

    for (;;)
    { if (sscanf(inp," /%ld%n",&div,&p) > 0)
      { printf("DIVIDING BY: %ld\n\n      ",div);
        have_div = 1;
        inp += p;
        break;
      }
      if (sscanf(inp,"%lf%n",&v,&p) < 1) break;
      inp += p;
      r = 1;
      if (*inp=='*' && sscanf(++inp,"%d%n",&r,&p) > 0)
      { inp += p;
        printf("ADDING: %+.18le repeated %d times:\n      ",v,r);
      }
      else
      { printf("ADDING: %+.18le:\n      ",v);
      }
      pbinary_double(v);
      printf("\n\n");
      while (r > 0)
      { xsum_small_add1(&sacc,v);
        if (xsum_debug) xsum_small_display(&sacc);
        xsum_large_addv(&lacc,&v,1);
        if (xsum_debug) xsum_large_display(&lacc);
        ls += v;
        s += v;
        N += 1;
        r -= 1;
      }
    }

    if (have_div && (div > UINT_MAX || div < INT_MIN))
    { div = div > UINT_MAX ? (long)UINT_MAX : (long)INT_MIN;
      printf("      Divisor out of range, with be set to %ld\n",div);
    }

    while (*inp == ' ') inp += 1;
    if (*inp!=0) printf("ignored junk: %s\n",inp);

    if (have_div)
    { result_s = div > INT_MAX ? xsum_small_div_unsigned(&sacc,div)
                               : xsum_small_div_int(&sacc,div);
      result_l = div > INT_MAX ? xsum_large_div_unsigned(&lacc,div)
                               : xsum_large_div_int(&lacc,div);
      ls /= (double) div;
      s /= (double) div;
    }
    else
    { result_s = xsum_small_round(&sacc); 
      result_l = xsum_large_round(&lacc);
    }

    printf ("ROUNDED RESULTS:\n");
    printf ("small %+.18le\n      ", (double)result_s);
    pbinary_double(result_s); printf("\n");
    printf ("large %+.18le\n      ", (double)result_l);
    pbinary_double(result_l); printf("\n");
    printf ("L dbl %+.18le\n      ", (double)ls);
    pbinary_double(ls); printf("\n");
    printf ("  dbl %+.18le\n      ", (double)s);
    pbinary_double(s); printf("\n");

    if (xsum_debug) xsum_small_display(&sacc);
    if (xsum_debug) xsum_large_display(&lacc);

    if (!have_div)
    {
      if (result_s != (double) xsum_small_round(&sacc))
      { printf ("RESULT DIFFERS AFTER ROUNDING SMALL ACCUMULATOR TWICE\n");
      }

      if (result_l != (double) xsum_large_round(&lacc))
      { printf ("RESULT DIFFERS AFTER ROUNDING LARGE ACCUMULATOR TWICE\n");
      }
    }

    if (result_s != result_l)
    { printf("RESULTS DIFFER FOR SMALL AND LARGE ACCUMULATORS\n");
    }

    if (N <= 2 && result_s != s)
    { printf("RESULTS DIFFER FOR DOUBLE AND SMALL ACCUMULATOR\n");
    }

    if (N <= 2 && result_l != s)
    { printf("RESULTS DIFFER FOR DOUBLE AND LARGE ACCUMULATOR\n");
    }
  }
}
