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
#include <float.h>

#include "shewchuk.h"

static const double MAX_DOUBLE = 1.79769313486231570815e+308;
static const double PENULTIMATE_DOUBLE = 1.79769313486231550856e+308;
static const double MAX_ULP = MAX_DOUBLE - PENULTIMATE_DOUBLE;
static const double TWO_POW_1023 = 8.98846567431158e+307;

static void vector_init(vector* vec) {
  vec->data = NULL;
  vec->size = 0;
  vec->capacity = 0;
}

static void vector_append(vector* vec, double value) {
  if (vec->size == vec->capacity) {
    vec->capacity = vec->capacity ? vec->capacity * 2 : 1;
    vec->data = realloc(vec->data, vec->capacity * sizeof(double));
  }
  vec->data[vec->size++] = value;
}

static void vector_shrink(vector* vec, size_t new_size) {
  vec->size = new_size;
}

void shewchuk_init(shewchuk* self) {
  self->is_negative_zero = true;
  vector_init(&self->partials);
  self->overflow = 0;
}

void shewchuk_reinit(shewchuk* self) {
  self->is_negative_zero = true;
  self->partials.size = 0;
  self->overflow = 0;
}

static inline void twosum(double x, double y, double* hi, double* lo) {
  *hi = x + y;
  *lo = y - (*hi - x);
}

void shewchuk_add(shewchuk* self, double x) {
  if (!(x == 0 && signbit(x)))
    self->is_negative_zero = false;

  unsigned actually_used_partials = 0;

  for (size_t i = 0; i < self->partials.size; ++i) {
    double y = self->partials.data[i];
    if (fabs(x) < fabs(y)) {
      double temp = x;
      x = y;
      y = temp;
    }

    double hi, lo;
    twosum(x, y, &hi, &lo);
    if (isinf(fabs(hi))) {
      double sign = hi < 0 ? -1 : 1;
      self->overflow += sign;

      x = (x - sign * TWO_POW_1023) - sign * TWO_POW_1023;
      if (fabs(x) < fabs(y)) {
        double temp = x;
        x = y;
        y = temp;
      }

      twosum(x, y, &hi, &lo);
    }

    if (lo != 0) {
      self->partials.data[actually_used_partials] = lo;
      ++actually_used_partials;
    }

    x = hi;
  }

  vector_shrink(&self->partials, actually_used_partials);

  if (x != 0)
    vector_append(&self->partials, x);
}

double shewchuk_compute(shewchuk* self) {
  if (self->is_negative_zero)
    return -0.0;

  int32_t n = (int32_t)self->partials.size - 1;
  double hi = 0;
  double lo = 0;

  if (self->overflow) {
    double next = n >= 0 ? self->partials.data[n] : 0;
    --n;
    if (fabs(self->overflow) > 1 || (self->overflow > 0 && next > 0) || (self->overflow < 0 && next < 0))
      return self->overflow > 0 ? INFINITY : -INFINITY;

    double pair_hi, pair_lo;
    twosum(self->overflow * TWO_POW_1023, next / 2, &pair_hi, &pair_lo);
    hi = pair_hi;
    lo = pair_lo * 2;

    if (isinf(fabs(hi * 2))) {
      if (hi > 0) {
        if (hi == TWO_POW_1023 && lo == -(MAX_ULP / 2) && n >= 0 && self->partials.data[n] < 0)
          return MAX_DOUBLE;
        return INFINITY;
      }

      if (hi == -TWO_POW_1023 && lo == (MAX_ULP / 2) && n >= 0 && self->partials.data[n] > 0)
        return -MAX_DOUBLE;
      return -INFINITY;
    }

    if (lo) {
      self->partials.data[n + 1] = lo;
      ++n;
      lo = 0;
    }

    hi *= 2;
  }

  while (n >= 0) {
    double x = hi;
    double y = self->partials.data[n];
    --n;

    twosum(x, y, &hi, &lo);

    if (lo)
      break;
  }

  if (n >= 0 && ((lo < 0 && self->partials.data[n] < 0) || (lo > 0 && self->partials.data[n] > 0))) {
    double y = lo * 2;
    double x = hi + y;
    double yr = x - hi;
    if (y == yr)
      hi = x;
  }

  return hi;
}
