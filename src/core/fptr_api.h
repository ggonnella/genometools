/*
  Copyright (c) 2006-2008 Gordon Gremme <gremme@@zbh.uni-hamburg.de>
  Copyright (c) 2006-2008 Center for Bioinformatics, University of Hamburg

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

#ifndef FPTR_API_H
#define FPTR_API_H

/* The generic function pointers. */

/* Return less than 0 if *a < *b,
   0 if *a == *b, and
   greater 0 if *a > * b.
   Do not count on these functions to return -1, 0, or 1!
 */
typedef int  (*GtCompare)(const void *a, const void *b);
typedef int  (*GtCompareWithData)(const void*, const void*, void *data);
typedef void (*GT_FreeFunc)(void*);

#endif
