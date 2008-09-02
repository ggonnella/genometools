/*
  Copyright (c) 2007 Malte Mader <mmader@zbh.uni-hamburg.de>
  Copyright (c) 2007 Sascha Steinbiss, Christin Schaerfer, Malte Mader
  Copyright (c) 2007 Christin Schaerfer <cschaerfer@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg

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

#ifndef DIAGRAM_H
#define DIAGRAM_H

typedef struct Diagram Diagram;

typedef struct TracklineInfo {
  unsigned long total_lines,
                total_captionlines;
} TracklineInfo;

#include "libgtview/canvas.h"
#include "libgtview/style.h"
#include "libgtview/block.h"
#include "libgtview/feature_index.h"
#include "libgtcore/array.h"
#include "libgtcore/range.h"
#include "libgtcore/hashmap.h"
#include "libgtext/genome_feature.h"
#include "libgtext/genome_feature_type.h"

/* Create a new Diagram object representing the genome nodes in
   <feature_index> in region <seqid> overlapping with <range>. */
Diagram*    diagram_new(FeatureIndex *feature_index, const char *seqid,
                        const Range*, Style*);
Range       diagram_get_range(Diagram*);
void        diagram_set_config(Diagram*, Style*);
Hashmap*    diagram_get_tracks(const Diagram*);
void        diagram_get_lineinfo(const Diagram*, TracklineInfo*);
int         diagram_get_number_of_tracks(const Diagram*);
/* Render <diagram> on the given <canvas>. */
int         diagram_render(Diagram *diagram, Canvas *canvas);
int         diagram_unit_test(Error*);
void        diagram_delete(Diagram*);

#endif
