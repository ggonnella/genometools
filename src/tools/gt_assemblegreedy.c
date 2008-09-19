/*
  Copyright (c) 2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
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

#include "core/bioseq.h"
#include "core/ma.h"
#include "core/unused_api.h"
#include "exercise/fragment_overlaps.h"
#include "exercise/greedy_assembly.h"
#include "tools/gt_assemblegreedy.h"

typedef struct {
  unsigned long minlength;
  bool showoverlaps,
       showpath;
} AssemblegreedyArguments;

static void* gt_assemblegreedy_arguments_new(void)
{
  return gt_malloc(sizeof (AssemblegreedyArguments));
}

static void gt_assemblegreedy_arguments_delete(void *tool_arguments)
{
  AssemblegreedyArguments *arguments = tool_arguments;
  if (!arguments) return;
  gt_free(arguments);
}

static GtOptionParser* gt_assemblegreedy_option_parser_new(void *tool_arguments)
{
  GtOptionParser *op;
  GtOption *option, *showoverlaps_option, *showpath_option;
  AssemblegreedyArguments *arguments = tool_arguments;
  assert(arguments);
  op = gt_option_parser_new("[option ...] fragment_file",
                         "Assemble fragments given in fragment_file in greedy "
                         "fashion.");
  option = gt_option_new_ulong("minlength", "set the minimum length an overlap "
                            "must have to be considered", &arguments->minlength,
                            5);
  gt_option_parser_add_option(op, option);
  showoverlaps_option = gt_option_new_bool("showoverlaps", "show only the "
                                        "overlaps between the fragments",
                                        &arguments->showoverlaps, false);
  gt_option_parser_add_option(op, showoverlaps_option);
  showpath_option = gt_option_new_bool("showpath",
                                       "show the assembled fragment "
                                       "path instead of the assembled sequence",
                                       &arguments->showpath, false);
  gt_option_parser_add_option(op, showpath_option);
  gt_option_exclude(showoverlaps_option, showpath_option);
  gt_option_parser_set_min_max_args(op, 1, 1);
  return op;
}

static int gt_assemblegreedy_runner(GT_UNUSED int argc, const char **argv,
                                 int parsed_args, void *tool_arguments,
                                 GtError *err)
{
  AssemblegreedyArguments *arguments = tool_arguments;
  GtBioseq *fragments;
  int had_err = 0;

  gt_error_check(err);
  assert(arguments);

  /* init */
  fragments = gt_bioseq_new(argv[parsed_args], err);
  if (!fragments)
     had_err = -1;

  if (!had_err) {
    GtFragmentOverlaps *fragment_overlaps;
    fragment_overlaps = gt_fragment_overlaps_new(fragments,
                                                 arguments->minlength);

    /* greedy assembly */
    if (arguments->showoverlaps)
      gt_fragment_overlaps_show(fragment_overlaps);
    else {
      GreedyAssembly *greedy_assembly;
      gt_fragment_overlaps_sort(fragment_overlaps);
      greedy_assembly = greedy_assembly_new(fragments, fragment_overlaps);
      if (arguments->showpath)
        greedy_assembly_show_path(greedy_assembly);
      else
        greedy_assembly_show(greedy_assembly, fragments);
      greedy_assembly_delete(greedy_assembly);
    }
    gt_fragment_overlaps_delete(fragment_overlaps);
  }

  /* free */
  gt_bioseq_delete(fragments);

  return had_err;
}

GtTool* gt_assemblegreedy(void)
{
  return gt_tool_new(gt_assemblegreedy_arguments_new,
                  gt_assemblegreedy_arguments_delete,
                  gt_assemblegreedy_option_parser_new,
                  NULL,
                  gt_assemblegreedy_runner);
}
