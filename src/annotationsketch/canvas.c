/*
  Copyright (c) 2008 Sascha Steinbiss <steinbiss@zbh.uni-hamburg.de>
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

#include <math.h>
#include <string.h>
#include "core/assert_api.h"
#include "core/class_alloc.h"
#include "core/ensure.h"
#include "core/ma.h"
#include "core/minmax.h"
#include "core/range.h"
#include "core/str.h"
#include "core/unused_api.h"
#include "annotationsketch/block.h"
#include "annotationsketch/canvas.h"
#include "annotationsketch/canvas_members.h"
#include "annotationsketch/canvas_rep.h"
#include "annotationsketch/graphics.h"
#include "annotationsketch/line.h"
#include "annotationsketch/style.h"

struct GtCanvasClass {
  size_t size;
  GtCanvasVisitLayoutFunc  visit_layout_pre,
                           visit_layout_post;
  GtCanvasVisitTrackFunc   visit_track_pre,
                           visit_track_post;
  GtCanvasVisitLineFunc    visit_line_pre,
                           visit_line_post;
  GtCanvasVisitBlockFunc   visit_block;
  GtCanvasVisitElementFunc visit_element;
  GtCanvasDrawRulerFunc    draw_ruler_func;
  GtCanvasFreeFunc         free;
};

const GtCanvasClass* gt_canvas_class_new(size_t size,
                                         GtCanvasVisitLayoutFunc la_visit_pre,
                                         GtCanvasVisitLayoutFunc la_visit_post,
                                         GtCanvasVisitTrackFunc t_visit_pre,
                                         GtCanvasVisitTrackFunc t_visit_post,
                                         GtCanvasVisitLineFunc l_visit_pre,
                                         GtCanvasVisitLineFunc l_visit_post,
                                         GtCanvasVisitBlockFunc b_visit,
                                         GtCanvasVisitElementFunc e_visit,
                                         GtCanvasDrawRulerFunc draw_ruler_func,
                                         GtCanvasFreeFunc free)
{
  GtCanvasClass *c_class = gt_class_alloc(sizeof *c_class);
  c_class->size = size;
  c_class->visit_layout_pre   = la_visit_pre;
  c_class->visit_layout_post  = la_visit_post;
  c_class->visit_track_pre    = t_visit_pre;
  c_class->visit_track_post   = t_visit_post;
  c_class->visit_line_pre     = l_visit_pre;
  c_class->visit_line_post    = l_visit_post;
  c_class->visit_block        = b_visit;
  c_class->visit_element      = e_visit;
  c_class->draw_ruler_func    = draw_ruler_func;
  c_class->free = free;
  return c_class;
}

/* Formats a given position number for short display in the ruler. */
void format_ruler_label(char *txt, unsigned long pos, size_t buflen)
{
  gt_assert(txt);
  double fpos;
  int logval;
  GtStr *formatstring;

  logval = (int) floor(log10(pos));
  formatstring = gt_str_new_cstr("%.");

  if (pos >= 1000000000)
  {
    fpos = (double) pos / 1000000000;
    while (pos % 10 == 0)
    {
      pos /= 10;
      logval--;
    }
    gt_str_append_ulong(formatstring, (unsigned long) logval);
    gt_str_append_cstr(formatstring, "fG");
    (void) snprintf(txt, buflen, gt_str_get(formatstring), fpos);
  }
  else if (pos >= 1000000)
  {
    fpos = (double) pos / 1000000;
    while (pos % 10 == 0)
    {
      pos /= 10;
      logval--;
    }
    gt_str_append_ulong(formatstring, (unsigned long) logval);
    gt_str_append_cstr(formatstring, "fM");
    (void) snprintf(txt, buflen, gt_str_get(formatstring), fpos);
  }
  else if (pos >= 1000)
  {
    fpos = (double) pos / 1000;
    while (pos % 10 == 0)
    {
      pos /= 10;
      logval--;
    }
    gt_str_append_ulong(formatstring, (unsigned long) logval);
    gt_str_append_cstr(formatstring, "fK");
    (void) snprintf(txt, buflen, gt_str_get(formatstring), fpos);
  } else
    (void) snprintf(txt, buflen, "%li", pos);
  gt_str_delete(formatstring);
}

void gt_canvas_draw_ruler(GtCanvas *canvas, GtRange rng)
{
  gt_assert(canvas);
  return canvas->c_class->draw_ruler_func(canvas, rng);
}

GtCanvas* gt_canvas_create(const GtCanvasClass *cc)
{
  GtCanvas *c;
  gt_assert(cc && cc->size);
  c = gt_calloc(1, cc->size);
  c->c_class = cc;
  c->pvt = gt_calloc(1, sizeof (GtCanvasMembers));
  return c;
}

void gt_canvas_delete(GtCanvas *canvas)
{
  if (!canvas) return;
  gt_assert(canvas->c_class);
  if (canvas->c_class->free)
    canvas->c_class->free(canvas);
  if (canvas->pvt->g)
    gt_graphics_delete(canvas->pvt->g);
  if (canvas->pvt->bt)
    gt_bittab_delete(canvas->pvt->bt);
  gt_free(canvas->pvt);
  gt_free(canvas);
}

void* gt_canvas_cast(GT_UNUSED const GtCanvasClass *cc, GtCanvas *c)
{
  gt_assert(cc && c && c->c_class == cc);
  return c;
}

void* gt_canvas_try_cast(GT_UNUSED const GtCanvasClass *cc, GtCanvas *c)
{
  gt_assert(cc && c);
  if (c->c_class == cc)
    return c;
  return NULL;
}

unsigned long gt_canvas_get_height(GtCanvas *canvas)
{
  gt_assert(canvas);
  return canvas->pvt->height;
}

int gt_canvas_visit_layout_pre(GtCanvas *canvas, GtLayout* layout)
{
  gt_assert(canvas && layout);
  if (canvas->c_class->visit_layout_pre)
    return canvas->c_class->visit_layout_pre(canvas, layout);
  else
    return 0;
}

int gt_canvas_visit_layout_post(GtCanvas *canvas, GtLayout* layout)
{
  gt_assert(canvas && layout);
  if (canvas->c_class->visit_layout_post)
    return canvas->c_class->visit_layout_post(canvas, layout);
  else
    return 0;
}

int gt_canvas_visit_track_pre(GtCanvas *canvas, GtTrack *track)
{
  gt_assert(canvas && track);
  if (canvas->c_class->visit_track_pre)
    return canvas->c_class->visit_track_pre(canvas, track);
  else
    return 0;
}

int gt_canvas_visit_track_post(GtCanvas *canvas, GtTrack *track)
{
  gt_assert(canvas && track);
  if (canvas->c_class->visit_track_post)
    return canvas->c_class->visit_track_post(canvas, track);
  else
    return 0;
}

int gt_canvas_visit_line_pre(GtCanvas *canvas, GtLine *line)
{
  gt_assert(canvas && line);
  if (canvas->c_class->visit_line_pre)
    return canvas->c_class->visit_line_pre(canvas, line);
  else
    return 0;
}

int gt_canvas_visit_line_post(GtCanvas *canvas, GtLine *line)
{
  gt_assert(canvas && line);
  if (canvas->c_class->visit_line_post)
    return canvas->c_class->visit_line_post(canvas, line);
  else
    return 0;
}

int gt_canvas_visit_block(GtCanvas *canvas, GtBlock *block)
{
  gt_assert(canvas && block);
  if (canvas->c_class->visit_block)
    return canvas->c_class->visit_block(canvas, block);
  else
    return 0;
}

int gt_canvas_visit_element(GtCanvas *canvas, GtElement *element)
{
  gt_assert(canvas && element);
  if (canvas->c_class->visit_element)
    return canvas->c_class->visit_element(canvas, element);
  else
    return 0;
}
