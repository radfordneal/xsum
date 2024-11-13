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

#include <math.h>
#include <assert.h>
#include <string.h>
#include <float.h>
#include <stdio.h>

#include "addition-with-bigints.h"
#include "double-utils.h"


void bigint_init(bigint* num) {
  num->is_negative = false;
  memset(num->digits, 0, sizeof(uint64_t) * ARRAY_SIZE);
}

void subtract_bigints(const bigint* a, const bigint* b, bigint* result);

// you are allowed to have result be one of the inputs
int add_bigints(const bigint* a, const bigint* b, bigint* result) {
  if (a->is_negative != b->is_negative) {
    subtract_bigints(a, b, result);
    return 0;
  }

  uint64_t carry = 0;

  for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
    uint64_t sum = a->digits[i] + b->digits[i] + carry;
    carry = (sum < a->digits[i]) || (sum < b->digits[i]);
    result->digits[i] = sum;
  }

  result->is_negative = a->is_negative;
  return carry;
}

int compare_abs_bigints(const bigint* a, const bigint* b) {
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    if (a->digits[i] > b->digits[i]) return 1;
    if (a->digits[i] < b->digits[i]) return -1;
  }
  return 0;
}

// computes a - (-b)
// requires that a and b have opposite sign
void subtract_bigints(const bigint* a, const bigint* b, bigint* result) {
  assert(a->is_negative == !b->is_negative);
  int cmp = compare_abs_bigints(a, b);
  const bigint* larger;
  const bigint* smaller;
  bool negate_result;

  if (cmp == 0) {
    bigint_init(result);
    return;
  } else if (cmp > 0) {
    larger = a;
    smaller = b;
    negate_result = a->is_negative;
  } else {
    larger = b;
    smaller = a;
    negate_result = !a->is_negative;
  }

  uint64_t borrow = 0;
  for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
    uint64_t diff;
    if (borrow == 1 && smaller->digits[i] == 0xffffffffffffffff) {
      // need this case to handle overflow of smaller->digits[i] + borrow
      diff = larger->digits[i];
      borrow = 1;
    } else if (larger->digits[i] >= smaller->digits[i] + borrow) {
      diff = larger->digits[i] - smaller->digits[i] - borrow;
      borrow = 0;
    } else {
      diff = (UINT64_MAX - smaller->digits[i] - borrow) + 1 + larger->digits[i];
      borrow = 1;
    }
    result->digits[i] = diff;
  }

  result->is_negative = negate_result;
}

// returns -1 when the input is 0
int floor_log2_bigint(const bigint* a) {
  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (a->digits[i] != 0) {
      for (int bit = 63; bit >= 0; bit--) {
        if ((a->digits[i] & (1ULL << bit)) != 0) {
          return (ARRAY_SIZE - i - 1) * 64 + bit;
        }
      }
    }
  }
  return -1;
}

// start is inclusive, end is exclusive
uint64_t slice_bits(const uint64_t* digits, int start, int end) {
  assert(start >= 0 && end <= TOTAL_BITS && start <= end && end - start < 64);
  if (start == end) return 0;

  // end is passed as exclusive but it's easier to manage inclusive
  end -= 1;

  uint64_t result = 0;
  int start_block = start / 64;
  int end_block = end / 64;
  int start_bit = start % 64;
  int end_bit = end % 64;

  if (start_block == end_block) {
    uint64_t mask = start_bit == 0 ? UINT64_MAX : (1ULL << (64 - start_bit)) - 1;
    result = (digits[start_block] & (mask)) >> (63 - end_bit);
  } else {
    uint64_t high_part = (digits[start_block] & ((1ULL << (64 - start_bit)) - 1)) << (end_bit + 1);
    uint64_t low_part = digits[end_block] >> (63 - end_bit);
    result = high_part | low_part;
  }

  return result;
}

bool any_bits_set_at_or_after(const uint64_t* digits, int bit) {
  assert(bit >= 0 && bit <= TOTAL_BITS);
  if (bit == TOTAL_BITS) {
    return false;
  }
  int block = bit / 64;
  int bit_in_block = bit % 64;
  uint64_t mask = bit_in_block == 0 ? UINT64_MAX : (1ULL << (64 - bit_in_block)) - 1;
  if ((digits[block] & mask) > 0) {
    return true;
  }
  for (int i = block + 1; i < ARRAY_SIZE; ++i) {
    if (digits[i] > 0) {
      return true;
    }
  }

  return false;
}

int get_bit(const uint64_t* digits, int bit) {
  if (bit == TOTAL_BITS) return 0;
  assert(bit >= 0 && bit < TOTAL_BITS);
  int block = bit / 64;
  int bit_in_block = bit % 64;
  return (digits[block] & ((1ULL << (63 - bit_in_block)))) > 0;
}

void set_bits(uint64_t* digits, int offset, uint64_t bits) {
  assert(offset >= 0 && offset <= TOTAL_BITS - 64);
  int start_index = offset / 64;
  int start_bit = offset % 64;

  if (start_bit == 0) {
    digits[start_index] |= bits;
  } else {
    int bits_in_first = 64 - start_bit;
    uint64_t mask_first = bits >> start_bit;
    digits[start_index] |= mask_first;

    uint64_t mask_second = bits << bits_in_first;
    digits[start_index + 1] |= mask_second;
  }
}

void double_to_bigint(double value, bigint* result) {
  assert(!isnan(value) && !isinf(value));

  bool sign;
  int exponent;
  uint64_t significand;
  deconstruct_double(value, &sign, &exponent, &significand);

  bigint_init(result);
  result->is_negative = sign;
  if (exponent == 0) {
    // subnormal or 0
    result->digits[ARRAY_SIZE - 1] = significand;
  } else {
    significand += 1ULL << 52;
    // exponent counts from the end, we want to count from the start
    // subtract 1 from exponent because otherwise the last bit is always 0
    set_bits(result->digits, (TOTAL_BITS - 64) - (exponent - 1), significand);
  }
}

double bigint_to_double(const bigint* arg) {
  int num_bits = floor_log2_bigint(arg) + 1;
  if (num_bits <= 52) {
    // subnormal or 0
    return construct_double(arg->is_negative, 0, arg->digits[ARRAY_SIZE - 1]);
  }
  int exponent = num_bits - 52;
  uint64_t significand = slice_bits(arg->digits, TOTAL_BITS - (exponent + 52) + 1, TOTAL_BITS - exponent + 1);

  int at_least_halfway_to_next = get_bit(arg->digits, TOTAL_BITS - exponent + 1);
  if (at_least_halfway_to_next) {
    int odd_before_rounding = get_bit(arg->digits, TOTAL_BITS - exponent);
    // if up is towards-even we always round up; otherwise we round up iff it is strictly above the halfway point, i.e. there is a non-zero bit after the first
    if (odd_before_rounding || any_bits_set_at_or_after(arg->digits, TOTAL_BITS - exponent + 2)) {
      significand += 1;
      if (significand == (1ULL << 52)) {
        significand = 0;
        exponent += 1;
      }
    }
  }
  if (exponent > 2046) {
    return arg->is_negative ? -INFINITY : INFINITY;
  }

  return construct_double(arg->is_negative, exponent, significand);
}


// debug utilities, not used

void print_bigint(const bigint* num) {
  int i;
  for (i = 0; i < ARRAY_SIZE; i++) {
    if (num->digits[i] != 0) break;
  }
  if (i == ARRAY_SIZE) {
    printf("0\n");
    return;
  }
  if (num->is_negative) {
    printf("-");
  }
  printf("%llx", (long long unsigned) num->digits[i]);
  for (i++; i < ARRAY_SIZE; i++) {
    printf("%016llx", (long long unsigned) num->digits[i]);
  }
  printf("\n");
}

void print_bigint_binary(const bigint* num) {
  int i, j;
  bool leading_zeros = true;

  if (num->is_negative) {
    printf("-");
  }

  for (i = 0; i < ARRAY_SIZE; i++) {
    if (leading_zeros && (num->digits[i] == 0)) {
      continue;
    }
    for (j = 63; j >= 0; j--) {
      int bit = (num->digits[i] >> j) & 1;
      if (bit == 1 || !leading_zeros) {
        printf("%d", bit);
        leading_zeros = false;
      }
    }
  }

  if (leading_zeros) {
    printf("0");
  }
  printf("\n");
}

void assert_roundtrips(double v) {
  bigint a;
  double_to_bigint(v, &a);
  double round_tripped = bigint_to_double(&a);
  if (v != round_tripped) {
    printf("FAILED TO ROUND TRIP %.17g (got %.17g)\n", v, round_tripped);
    print_bigint(&a);
    assert(false && "failed to round trip");
  }
}

