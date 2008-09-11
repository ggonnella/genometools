/*
  Copyright (c) 2007-2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2007-2008 Center for Bioinformatics, University of Hamburg

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

#include "lauxlib.h"
#include "extended/extract_feat_sequence.h"
#include "extended/genome_node.h"
#include "extended/gff3_output.h"
#include "extended/luahelper.h"
#include "gtlua/genome_node_lua.h"
#include "gtlua/genome_visitor_lua.h"
#include "gtlua/gt_lua.h"
#include "gtlua/range_lua.h"
#include "gtlua/region_mapping_lua.h"

static int genome_feature_lua_new(lua_State *L)
{
  GT_GenomeNode **gf;
  GT_Range *range;
  GT_Strand strand;
  const char *seqid, *type, *strand_str;
  size_t length;
  GT_Str *seqid_str;
  assert(L);
  /* get/check parameters */
  seqid = luaL_checkstring(L, 1);
  type = luaL_checkstring(L, 2);
  range = check_range(L, 3);
  strand_str = luaL_checklstring(L, 4, &length);
  luaL_argcheck(L, length == 1, 4, "strand string must have length 1");
  luaL_argcheck(L, (strand = gt_strand_get(strand_str[0])) !=
                    GT_NUM_OF_STRAND_TYPES, 4, "invalid strand");
  /* construct object */
  gf = lua_newuserdata(L, sizeof (GT_GenomeNode*));
  seqid_str = gt_str_new_cstr(seqid);
  *gf = gt_genome_feature_new(seqid_str, type, *range, strand);
  gt_str_delete(seqid_str);
  assert(*gf);
  luaL_getmetatable(L, GENOME_NODE_METATABLE);
  lua_setmetatable(L, -2);
  return 1;
}

static int sequence_region_lua_new(lua_State *L)
{
  GT_GenomeNode **sr;
  const char *seqid;
  GT_Str *seqid_str;
  GT_Range *range;
  assert(L);
  /* get_check parameters */
  seqid = luaL_checkstring(L, 1);
  range = check_range(L, 2);
  /* construct object */
  sr = lua_newuserdata(L, sizeof (GT_GenomeNode*));
  seqid_str = gt_str_new_cstr(seqid);
  *sr = gt_sequence_region_new(seqid_str, *range);
  gt_str_delete(seqid_str);
  assert(*sr);
  luaL_getmetatable(L, GENOME_NODE_METATABLE);
  lua_setmetatable(L, -2);
  return 1;
}

static int genome_node_lua_get_filename(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  lua_pushstring(L, gt_genome_node_get_filename(*gn));
  return 1;
}

static int genome_node_lua_get_range(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  return gt_lua_range_push(L, gt_genome_node_get_range(*gn));
}

static int genome_node_lua_get_seqid(lua_State *L)
{
  GT_Str *seqid;
  GT_GenomeNode **gn = check_genome_node(L, 1);
  if ((seqid = gt_genome_node_get_seqid(*gn)))
    lua_pushstring(L, gt_str_get(seqid));
  else
    lua_pushnil(L);
  return 1;
}

static int genome_feature_lua_get_strand(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  GT_GenomeFeature *gf;
  char strand_char[2];
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  strand_char[0] = GT_STRAND_CHARS[gt_genome_feature_get_strand(gf)];
  strand_char[1] = '\0';
  lua_pushstring(L, strand_char);
  return 1;
}

static int genome_feature_lua_get_source(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  GT_GenomeFeature *gf;
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  lua_pushstring(L, gt_genome_feature_get_source(gf));
  return 1;
}

static int genome_feature_lua_get_score(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  GT_GenomeFeature *gf;
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  if (gt_genome_feature_score_is_defined(gf))
    lua_pushnumber(L, gt_genome_feature_get_score(gf));
  else
    lua_pushnil(L);
  return 1;
}

static int genome_feature_lua_get_attribute(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  const char *attr = NULL, *attrval = NULL;
  attr = luaL_checkstring(L, 2);
  GT_GenomeFeature *gf;
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  attrval = gt_genome_feature_get_attribute(*gn, attr);
  if (attrval)
    lua_pushstring(L, attrval);
  else
    lua_pushnil(L);
  return 1;
}

static int genome_feature_lua_get_exons(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  GT_Array *exons = gt_array_new(sizeof (GT_GenomeNode*));
  unsigned long i = 0;
  GT_GenomeFeature *gf;
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  gt_genome_feature_get_exons(gf, exons);
  lua_newtable(L);
  for (i = 0; i < gt_array_size(exons); i++) {
    lua_pushnumber(L, i+1);
    gt_lua_genome_node_push(L, gt_genome_node_ref(*(GT_GenomeNode**)
                            gt_array_get(exons, i)));
    lua_rawset(L, -3);
  }
  gt_array_delete(exons);
  return 1;
}

static int genome_feature_lua_set_source(lua_State *L)
{
  const char *source;
  GT_Str *source_str;
  GT_GenomeNode **gn = check_genome_node(L, 1);
  GT_GenomeFeature *gf;
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  source = luaL_checkstring(L, 2);
  source_str = gt_str_new_cstr(source);
  gt_genome_feature_set_source(*gn, source_str);
  gt_str_delete(source_str);
  return 0;
}

static int genome_node_lua_accept(lua_State *L)
{
  GT_GenomeNode **gn;
  GenomeVisitor **gv;
  GT_Error *err;
  gn = check_genome_node(L, 1);
  gv = check_genome_visitor(L, 2);
  err = gt_error_new();
  if (gt_genome_node_accept(*gn, *gv, err))
    return lua_gt_error(L, err);
  gt_error_delete(err);
  return 0;
}

static int genome_node_lua_add_child(lua_State *L)
{
  GT_GenomeNode **parent, **child;
  parent = check_genome_node(L, 1);
  child  = check_genome_node(L, 2);
  gt_genome_node_add_child(*parent, gt_genome_node_rec_ref(*child));
  return 0;
}

static int genome_node_lua_mark(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  gt_genome_node_mark(*gn);
  return 0;
}

static int genome_node_lua_is_marked(lua_State *L)
{
  GT_GenomeNode **gn = check_genome_node(L, 1);
  lua_pushboolean(L, gt_genome_node_is_marked(*gn));
  return 1;
}

static int genome_node_lua_contains_marked(lua_State *L)
{
  GT_GenomeNode **gn;
  gn = check_genome_node(L, 1);
  lua_pushboolean(L, gt_genome_node_contains_marked(*gn));
  return 1;
}

static int genome_feature_lua_output_leading(lua_State *L)
{
  GT_GenomeNode **gn;
  GT_GenomeFeature *gf;
  gn = check_genome_node(L, 1);
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  gff3_output_leading(gf, NULL);
  return 0;
}

static int genome_feature_lua_get_type(lua_State *L)
{
  GT_GenomeNode **gn;
  GT_GenomeFeature *gf;
  gn = check_genome_node(L, 1);
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  lua_pushstring(L, gt_genome_feature_get_type(gf));
  return 1;
}

static int genome_feature_lua_extract_sequence(lua_State *L)
{
  GT_GenomeNode **gn;
  GT_GenomeFeature *gf;
  const char *type;
  bool join;
  RegionMapping **region_mapping;
  GT_Str *sequence;
  GT_Error *err;
  gn = check_genome_node(L, 1);
  /* make sure we get a genome feature */
  gf = gt_genome_node_cast(gt_genome_feature_class(), *gn);
  luaL_argcheck(L, gf, 1, "not a genome feature");
  type = luaL_checkstring(L, 2);
  join = lua_toboolean(L, 3);
  region_mapping = check_region_mapping(L, 4);
  err = gt_error_new();
  sequence = gt_str_new();
  if (extract_feat_sequence(sequence, *gn, type, join, *region_mapping, err)) {
    gt_str_delete(sequence);
    return lua_gt_error(L, err);
  }
  if (gt_str_length(sequence))
    lua_pushstring(L, gt_str_get(sequence));
  else
    lua_pushnil(L);
  gt_str_delete(sequence);
  gt_error_delete(err);
  return 1;
}

static int gt_genome_node_lua_delete(lua_State *L)
{
  GT_GenomeNode **gn;
  gn = check_genome_node(L, 1);
  gt_genome_node_rec_delete(*gn);
  return 0;
}

static const struct luaL_Reg genome_node_lib_f [] = {
  { "genome_feature_new", genome_feature_lua_new },
  { "sequence_region_new", sequence_region_lua_new },
  { NULL, NULL }
};

static const struct luaL_Reg genome_node_lib_m [] = {
  { "get_filename", genome_node_lua_get_filename },
  { "get_range", genome_node_lua_get_range },
  { "get_seqid", genome_node_lua_get_seqid },
  { "get_strand", genome_feature_lua_get_strand },
  { "get_source", genome_feature_lua_get_source },
  { "set_source", genome_feature_lua_set_source },
  { "get_score", genome_feature_lua_get_score },
  { "get_attribute", genome_feature_lua_get_attribute },
  { "get_exons", genome_feature_lua_get_exons },
  { "accept", genome_node_lua_accept },
  { "add_child", genome_node_lua_add_child },
  { "mark", genome_node_lua_mark },
  { "is_marked", genome_node_lua_is_marked },
  { "contains_marked", genome_node_lua_contains_marked },
  { "output_leading", genome_feature_lua_output_leading },
  { "get_type", genome_feature_lua_get_type },
  { "extract_sequence", genome_feature_lua_extract_sequence },
  { NULL, NULL }
};

int luaopen_genome_node(lua_State *L)
{
  assert(L);
  luaL_newmetatable(L, GENOME_NODE_METATABLE);
  /* metatable.__index = metatable */
  lua_pushvalue(L, -1); /* duplicate the metatable */
  lua_setfield(L, -2, "__index");
  /* set its _gc field */
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, gt_genome_node_lua_delete);
  lua_settable(L, -3);
  /* register functions */
  luaL_register(L, NULL, genome_node_lib_m);
  lua_export_metatable(L, GENOME_NODE_METATABLE);
  luaL_register(L, "gt", genome_node_lib_f);
  return 1;
}

void gt_lua_genome_node_push(lua_State *L, GT_GenomeNode *gn)
{
  GT_GenomeNode **gn_lua;
  assert(L && gn);
  gn_lua = lua_newuserdata(L, sizeof (GT_GenomeNode**));
  *gn_lua = gn;
  luaL_getmetatable(L, GENOME_NODE_METATABLE);
  lua_setmetatable(L, -2);
}
