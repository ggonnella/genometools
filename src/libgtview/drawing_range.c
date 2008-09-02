/*
  Copyright (c) 2008 Sascha Steinbiss <ssteinbiss@stud.zbh.uni-hamburg.de>
  Copyright (c) 2008 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <assert.h>
#include "libgtcore/mathsupport.h"
#include "libgtview/drawing_range.h"

int drawing_range_compare(DrawingRange range_a, DrawingRange range_b)
{
  assert(range_a.start <= range_a.end && range_b.start <= range_b.end);

  if (double_equals_double(range_a.start, range_b.start)
        && double_equals_double(range_a.end, range_b.end))
    return 0; /* range_a == range_b */

  if ((range_a.start < range_b.start) ||
      (double_equals_double(range_a.start, range_b.start)
         && (range_a.end < range_b.end)))
    return -1; /* range_a < range_b */

  return 1; /* range_a > range_b */
}

bool drawing_range_overlap(DrawingRange range_a, DrawingRange range_b)
{
  if (range_a.start <= range_b.end && range_a.end >= range_b.start)
    return true;
  return false;
}

bool drawing_range_contains(DrawingRange range_a, DrawingRange range_b)
{
  if (range_a.start <= range_b.start && range_a.end >= range_b.end)
    return true;
  return false;
}

double drawing_range_length(DrawingRange range)
{
  return range.end - range.start;
}
