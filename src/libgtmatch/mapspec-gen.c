/*
  Copyright (c) 2007 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#include <errno.h>
#include "libgtcore/env.h"
#include "libgtcore/str.h"
#include "types.h"
#include "fmi-bwtbound.h"
#include "intbits.h"
#include "safecast-gen.h"
#include "mapspec-def.h"
#include "stamp.h"

#define ASSIGNPTR2STARTPTR(TYPE)\
        if (mapspec->numofunits == 0)\
        {\
          *((TYPE **) mapspec->startptr) = NULL;\
        } else\
        {\
          voidptr = (((void *) ptr) + byteoffset);\
          *((TYPE **) mapspec->startptr) = voidptr;\
        }

#define ALIGNSIZE sizeof(void *)

#define WRITEACTIONWITHTYPE(TYPE)\
        if (fwrite(*((TYPE **) mapspecptr->startptr),\
                   mapspecptr->sizeofunit,\
                   (size_t) mapspecptr->numofunits, fp) !=\
                    (size_t) mapspecptr->numofunits)\
        {\
          env_error_set(env,"cannot write %lu items of size %u: "\
                            "errormsg=\"%s\"",\
                        (unsigned long) mapspecptr->numofunits,\
                        (unsigned int) mapspecptr->sizeofunit,\
                        strerror(errno));\
          haserr = true;\
        }


static uint64_t detexpectedaccordingtomapspec(const ArrayMapspecification 
                                              *mapspectable)
{
  uint64_t sumup = 0;
  Mapspecification *mapspecptr;

  for (mapspecptr = mapspectable->spaceMapspecification;
       mapspecptr < mapspectable->spaceMapspecification +
                    mapspectable->nextfreeMapspecification; mapspecptr++)
  {
    sumup += (uint64_t) mapspecptr->sizeofunit * 
             (uint64_t) mapspecptr->numofunits;
    if(sumup % ALIGNSIZE > 0)
    {
      sumup += (ALIGNSIZE - (sumup % ALIGNSIZE));
    }
  }
  return sumup;
}

static void showmapspec(const Mapspecification *mapspec)
{
  printf("(%s,size=%lu,elems=%lu)",
           mapspec->name,
           (unsigned long) mapspec->sizeofunit,
           mapspec->numofunits);
}

static int assigncorrecttype(Mapspecification *mapspec,
                             void *ptr,
                             unsigned long byteoffset,
                             Env *env)
{
  void *voidptr;
  bool haserr = false;

  env_error_check(env);
  switch (mapspec->typespec)
  {
    case UcharType:
      ASSIGNPTR2STARTPTR(Uchar);
      break;
    case UshortType:
      ASSIGNPTR2STARTPTR(Ushort);
      break;
    case Uint32Type:
      ASSIGNPTR2STARTPTR(uint32_t);
      break;
    case Uint64Type:
      ASSIGNPTR2STARTPTR(uint64_t);
      break;
    case BitstringType:
      ASSIGNPTR2STARTPTR(Bitstring);
      break;
    case SeqposType:
      ASSIGNPTR2STARTPTR(Seqpos);
      break;
    case BwtboundType:
      ASSIGNPTR2STARTPTR(Bwtbound);
      break;
    case PairBwtidxType:
      ASSIGNPTR2STARTPTR(PairBwtidx);
      break;
    default:
      env_error_set(env,"no assignment specification for size %lu",
                    (unsigned long) mapspec->sizeofunit);
      haserr = true;
  }
  if (!haserr)
  {
    printf("# assign pointer");
    showmapspec(mapspec);
    printf(" at byteoffset %lu\n",byteoffset);
  }
  return haserr ? -1 : 0;
}

 DECLARESAFECASTFUNCTION(uint64_t,uint64_t,unsigned long,unsigned_long)

int fillmapspecstartptr(Assignmapspec assignmapspec,
                        void **mappeduserptr,
                        void *assignmapinfo,
                        const Str *tmpfilename,
                        unsigned long expectedsize,
                        Env *env)
{
  void *mapptr;
  uint64_t expectedaccordingtomapspec;
  unsigned long byteoffset = 0;
  size_t numofbytes;
  ArrayMapspecification mapspectable;
  Mapspecification *mapspecptr;
  bool haserr = false;
  unsigned long totalpadunits = 0;

  env_error_check(env);
  INITARRAY(&mapspectable,Mapspecification);
  assignmapspec(&mapspectable,assignmapinfo,env);
  mapptr = env_fa_mmap_read(env,str_get(tmpfilename), &numofbytes);
  if (mapptr == NULL)
  {
    env_error_set(env,"could not map datafile %s",str_get(tmpfilename));
    haserr = true;
  }
  *mappeduserptr = mapptr;
  if (!haserr)
  {
    if (assigncorrecttype(mapspectable.spaceMapspecification,
                          mapptr,0,env) != 0)
    {
      haserr = true;
    }
  }
  if (!haserr)
  {
    expectedaccordingtomapspec = detexpectedaccordingtomapspec(&mapspectable);
    if (expectedaccordingtomapspec != (uint64_t) numofbytes)
    {
      env_error_set(env,"%lu bytes read from %s, but " Formatuint64_t 
                         " expected",
                         (unsigned long) numofbytes,
                         str_get(tmpfilename),
                         expectedaccordingtomapspec);
      haserr = true;
    }
  }
  if (!haserr)
  {
    mapspecptr = mapspectable.spaceMapspecification;
    assert(mapspecptr != NULL);
    byteoffset = CALLCASTFUNC(uint64_t,unsigned_long,
                              (uint64_t) (mapspecptr->sizeofunit * 
                                          mapspecptr->numofunits));
    if(byteoffset % (unsigned long) ALIGNSIZE > 0)
    {
      size_t padunits = ALIGNSIZE - (byteoffset % ALIGNSIZE);
      byteoffset += (unsigned long) padunits;
      totalpadunits += (unsigned long) padunits;
    }
    for (mapspecptr++;
         mapspecptr < mapspectable.spaceMapspecification +
                      mapspectable.nextfreeMapspecification; mapspecptr++)
    {
      if (assigncorrecttype(mapspecptr,mapptr,byteoffset,env) != 0)
      {
        haserr = true;
        break;
      }
      byteoffset = CALLCASTFUNC(uint64_t,unsigned_long,
                                (uint64_t) (byteoffset + 
                                            mapspecptr->sizeofunit * 
                                            mapspecptr->numofunits));
      if(byteoffset % (unsigned long) ALIGNSIZE > 0)
      {
        size_t padunits = ALIGNSIZE - (byteoffset % ALIGNSIZE);
        byteoffset += (unsigned long) padunits;
        totalpadunits += (unsigned long) padunits;
      }
    }
  }
  if (!haserr)
  {
    if (expectedsize + totalpadunits != byteoffset)
    {
      env_error_set(env,"mapping: expected size of index is %lu bytes, "
                        "but index has %lu bytes",
                        expectedsize,byteoffset);
      haserr = true;
    }
  }
  FREEARRAY(&mapspectable,Mapspecification);
  return haserr ? -1 : 0;
}

int flushtheindex2file(FILE *fp,
                       Assignmapspec assignmapspec,
                       void *assignmapinfo,
                       unsigned long expectedsize,
                       Env *env)
{
  ArrayMapspecification mapspectable;
  Mapspecification *mapspecptr;
  unsigned long byteoffset = 0;
  bool haserr = false;
  Uchar padbuffer[ALIGNSIZE-1] = {0};
  unsigned long totalpadunits = 0;

  env_error_check(env);
  INITARRAY(&mapspectable,Mapspecification);
  assignmapspec(&mapspectable,assignmapinfo,env);
  assert(mapspectable.spaceMapspecification != NULL);
  for (mapspecptr = mapspectable.spaceMapspecification;
       mapspecptr < mapspectable.spaceMapspecification +
                    mapspectable.nextfreeMapspecification; 
       mapspecptr++)
  {
    printf("# flushtheindex2file");
    showmapspec(mapspecptr);
    printf(" at byteoffset %lu\n",byteoffset);
    if (mapspecptr->numofunits > 0)
    {
      switch (mapspecptr->typespec)
      {
        case UcharType:
          WRITEACTIONWITHTYPE(Uchar);
          break;
        case UshortType:
          WRITEACTIONWITHTYPE(Ushort);
          break;
        case Uint32Type:
          WRITEACTIONWITHTYPE(uint32_t);
          break;
        case Uint64Type:
          WRITEACTIONWITHTYPE(uint64_t);
          break;
        case BitstringType:
          WRITEACTIONWITHTYPE(Bitstring);
          break;
        case SeqposType:
          WRITEACTIONWITHTYPE(Seqpos);
          break;
        case BwtboundType:
          WRITEACTIONWITHTYPE(Bwtbound);
          break;
        case PairBwtidxType:
          WRITEACTIONWITHTYPE(PairBwtidx);
          break;
        default:
           env_error_set(env,"no map specification for size %lu",
                         (unsigned long) mapspecptr->sizeofunit);
           haserr = true;
      }
    }
    if (haserr)
    {
      break;
    }
    byteoffset = CALLCASTFUNC(uint64_t,unsigned_long,
                              (uint64_t) (byteoffset + 
                                          mapspecptr->sizeofunit * 
                                          mapspecptr->numofunits));
    if(byteoffset % (unsigned long) ALIGNSIZE > 0)
    {
      size_t padunits = ALIGNSIZE - (byteoffset % ALIGNSIZE);
      if(fwrite(padbuffer,
                sizeof(Uchar),padunits,fp) != padunits)
      {
        env_error_set(env,"cannot write %lu items of size %u: "
                          "errormsg=\"%s\"",
                           (unsigned long) padunits,
                           (unsigned int) sizeof(Uchar),
                           strerror(errno));
        haserr = true; 
      }
      byteoffset += (unsigned long) padunits;
      totalpadunits += (unsigned long) padunits;
    }
  }
  if (!haserr)
  {
    if (expectedsize + totalpadunits != byteoffset)
    {
      env_error_set(env,"flushindex: expected size of index is %lu bytes, "
                        "but index has %lu bytes",
                        expectedsize,
                        byteoffset);
      haserr = true;
    }
  }
  FREEARRAY(&mapspectable,Mapspecification);
  return haserr ? -1 : 0;
}
