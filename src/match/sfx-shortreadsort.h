/*
  Copyright (c) 2011 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2011 Center for Bioinformatics, University of Hamburg

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

#ifndef SFX_SHORTREADSORT_H
#define SFX_SHORTREADSORT_H

#include "core/readmode_api.h"
#include "core/encseq_api.h"
#include "core/unused_api.h"
#include "sfx-lcpvalues.h"
#include "sfx-suffixgetset.h"
#include "spmsuftab.h"

#define GT_BSR_UPDATEMAXLCP(MAXVAL,LCP)\
        if ((MAXVAL) < (LCP))\
        {\
          MAXVAL = LCP;\
        }

typedef struct GtShortreadsortworkinfo GtShortreadsortworkinfo;

size_t gt_shortreadsort_size(unsigned long maxvalue);

GtShortreadsortworkinfo *gt_shortreadsort_new(unsigned long maxwidth,
                                              GtReadmode readmode,
                                              unsigned int bitsforrelpos,
                                              bool firstcodes);

void gt_shortreadsort_delete(GtShortreadsortworkinfo *srsw);

void gt_shortreadsort_assigntableoflcpvalues(
          GtShortreadsortworkinfo *srsw,GtLcpvalues *tableoflcpvalues);

const uint16_t *gt_shortreadsort_lcpvalues(const GtShortreadsortworkinfo *srsw);

void gt_shortreadsort_sssp_sort(GtShortreadsortworkinfo *srsw,
                                const GtEncseq *encseq,
                                GtReadmode readmode,
                                GtEncseqReader *esr,
                                GtSuffixsortspace *sssp,
                                unsigned long subbucketleft,
                                unsigned long width,
                                unsigned long depth);

void gt_shortreadsort_array_sort(unsigned long *suftab_bucket,
                                 unsigned long *seqnum_relpos_bucket,
                                 GtShortreadsortworkinfo *srsw,
                                 const GtEncseq *encseq,
                                 GtSpmsuftab *spmsuftab,
                                 unsigned long subbucketleft,
                                 unsigned long width,
                                 unsigned long depth);

#endif
