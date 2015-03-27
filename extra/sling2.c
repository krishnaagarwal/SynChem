/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SLING.C
*
*    This module is the abstraction for the Sling data-structure.  It knows
*    how to manage the data-structure, also how to convert it to other
*    molecule formats, and how to calculate a canonical Sling which is very
*    very important.
*
*  Routines:
*
*    Sling_Canonical_Generate
*    Sling_Copy
*    Sling_CopyTrunc
*    Sling_Create
*    Sling_Destroy
*    Sling_Dump
*    Sling_Validate
*    Sling2String
*    Sling2Tsd
*    Sling2Tsd_PlusHydrogen
*    Sling2Xtr
*    String2Sling
*    Tsd2Sling
*    Tsd2SlingX
*    SAsiDump
*    SAtomLocate
*    SAtomRead
*    SBreak
*    SBrothersXlate
*    SChiralityCheck
*    SCodedTsdEqual
*    SCodedTsdLessThan
*    SCompact
*    SCompute
*    SDifferentiator
*    SExchangesCount
*    SExpand
*    SFirstIndexProcess
*    SIdCheck
*    SIdRestore
*    SLetter
*    SListAdd
*    SListPlaceFind
*    SListSearch
*    SListSmallestFind
*    SNumberSkip
*    SNumberRead
*    SOlefinAtomFix
*    SParityCheck
*    SListParityPushDown
*    SParitySkip
*    SParityUpdate
*    SParityUpdate1
*    SParityUpdate2
*    SParityUpdate3
*    SPointerMove
*    SPushDown
*    SRowCopy
*    SSwap
*    STieBreaker
*    STsdDump
*    STsdSwap
*    STsdUnique
*    SUniqueCompare
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PL/I code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
* 07-Sep-01  Miller     Fixed memory leak in Sling_Canonical_Generate().
******************************************************************************/

#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ARRAY
#include "array.h"
#endif

#ifndef _H_UTL
#include "utl.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_TSD
#include "tsd.h"
#endif

#define SLING_GLOBALS 1
#ifndef _H_SLING
#include "sling.h"
#endif
#undef SLING_GLOBALS

/* Static Routine Prototypes */

static void      SAsiDump        (Array_t *, Array_t *, Array_t *);
static U16_t     SAtomLocate     (const char *);
static U8_t     *SAtomRead       (U8_t *, Tsd_t *, U16_t);
static void      SBreak          (Array_t *, Array_t *, U16_t);
static void      SBrothersXlate  (Array_t *, Array_t *, String_t *, U16_t);
static void      SChiralityCheck (Boolean_t *, CodedTsd_t *, Array_t *, U16_t);
static Boolean_t SCodedTsdEqual  (CodedTsd_t *, CodedTsd_t *);
static Boolean_t SCodedTsdLessThan (CodedTsd_t *, CodedTsd_t *);
static Tsd_t    *SCompact        (Tsd_t *, Array_t *, Boolean_t);
static void      SCompute        (Array_t *, Array_t *, Tsd_t *, Tsd_t *,
  Array_t *, Tsd_t *, Array_t *, U16_t);
static void      SDifferentiator (Array_t *, Array_t *, Boolean_t *, Array_t *,
  Tsd_t *, Array_t *, Array_t *);
static U16_t     SExchangesCount (U16_t, U16_t, U16_t, U16_t *, Boolean_t *);
static Tsd_t    *SExpand         (Tsd_t *);
static void      SFirstIndexProcess (Stack_t *, Stack_t *, Stack_t *,
  Boolean_t *, Array_t *, Tsd_t *, Array_t *, Array_t *, Array_t *);
static U16_t     SIdCheck        (U16_t);
static U16_t     SIdRestore      (U16_t);
static String_t  SLetter        (S16_t);
static void      SListAdd        (CodedTsd_t **, CodedTsd_t **, U16_t *,
  Tsd_t *, Array_t *, Array_t *, Array_t *);
static void      SListPlaceFind  (CodedTsd_t *, CodedTsd_t *, CodedTsd_t **,
  CodedTsd_t **, U16_t *);
static CodedTsd_t *SListSearch (CodedTsd_t *, CodedTsd_t *, U16_t *);
static void      SListSmallestFind (CodedTsd_t *, Array_t *, Boolean_t *,
  Boolean_t *, U16_t);
static void      SNumberSkip     (Slng2TsdCB_t *);
static U8_t      SNumberRead     (Slng2TsdCB_t *);
static void      SOlefinAtomFix  (Tsd_t *, Array_t *, U16_t, U8_t);
static void      SParityCheck    (Tsd_t *, Array_t *, U16_t,  Slng2TsdCB_t *);
static void      SParityPushDown (Boolean_t *, CodedTsd_t *, Array_t *,
  Array_t *, U16_t);
static void      SParitySkip     (Slng2TsdCB_t *);
static void      SParityUpdate   (Array_t *, Array_t *, Array_t *, Array_t *,
  Array_t *, Array_t *, Array_t *, U16_t);
static void      SParityUpdate1  (U16_t, S8_t, Array_t *, Array_t *, Array_t *,
  Array_t *);
static void      SParityUpdate2  (U16_t, Array_t *, Array_t *);
static void      SParityUpdate3  (U16_t, Array_t *, Array_t *);
static void      SPointerMove    (Stack_t *, Slng2TsdCB_t *);
static void      SPushDown       (Stack_t *, Stack_t *, Stack_t *);
static void      SRowCopy        (U16_t, U16_t, U16_t *, U16_t, U16_t, Tsd_t *,
  Array_t *);
static void      SSwap           (U16_t *, U16_t *);
static void      STieBreaker     (Array_t *, Array_t *, U16_t, Boolean_t *,
  Boolean_t *, Array_t *, Tsd_t *, Array_t *, Array_t *, Array_t *, Array_t *,
  Array_t *, Array_t *, CodedTsd_t **, Array_t *, Tsd_t *, Array_t *);
static void      STsdDump        (Tsd_t *, FileDsc_t *);
static void      STsdSwap        (Tsd_t *, U16_t, U8_t, U8_t);
static Tsd_t    *STsdUnique      (Tsd_t *, AtomArray_t **, Array_t *, 
  Array_t *, Array_t *, Array_t *, Array_t *, Boolean_t *, U16_t, U16_t *);
static void      SUniqueCompare  (Array_t *, Array_t *, Array_t *);

#define SAtomVerify(asym)\
  (Atomsymbol2Id (asym) == 0 ? FALSE : TRUE)

/* For Debugging purposes only!!! */


/****************************************************************************
*
*  Function Name:                 Sling_Canonical_Generate
*
*    This routine controls the discovery of THE Canonical Sling for the
*    input molecule.  The algorithm is long and convoluted so see the 
*    documentation.
*
*  Used to be:
*
*    name:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    May call IO_Exit_Error
*
******************************************************************************/
void Sling_Canonical_Generate
  (
  Tsd_t        *tsd_p,                      /* Basic TSD for molecule */
  Tsd_t       **canonical_tsd_pp,           /* Output canonical TSD */
  AtomArray_t **parityvector_pp,            /* Head of list of parity vector */
  Array_t      *cebs_p,                     /* 1-d word, Const. Equiv. */
  Array_t      *sebs_p,                     /* 1-d word, Stereo Chem. Equiv */
  String_t     *canonical_str_p,            /* Output canonical string */
  Array_t      *paritybits_p,               /* 1-d bit, even parity (Output) */
  String_t     *ce_str_p,                   /* Output CE string */
  String_t     *se_str_p,                   /* Output SE string */
  Array_t      *asymmetric_p,               /* 1-d bit, asymmtric flgs (Out) */
  Array_t      *map_p,                      /* 1-d word, orig -> canon (Out) */
  Boolean_t     compaction,                 /* Flag for whether to compact */
  Boolean_t    *chiral_p,                   /* Flag for atom chirality */
  U16_t        *num_carbons_p,              /* Output # carbons, array size */
  U16_t        *compact_size_p              /* # atoms with mult.  bonds */
  )
{
  Tsd_t        *tsd_short_p;                /* Short TSD */
  Tsd_t        *tsd_uniq_p;                 /* Unique TSD */
  U16_t         num_atoms;                  /* # atoms in input TSD */
  U16_t         atom;                       /* Counter */
  U16_t         atomid;                     /* Atomic identifier */
  U16_t         newid;                      /* Different atomic "number" */
  U16_t         long_atoms;                 /* Number of atoms in long TSD */
  U16_t         short_atoms;                /* Number of atoms in short TSD */
  Array_t       uniqmap;                    /* 1-d word, unique TSD */
  Array_t       shortmap;                   /* 1-d word, mapping short to long
    TSD */
  Array_t       uniqparity;                 /* 1-d byte, parity indicators */
  Sling_t       sling;                      /* Result from TSD2SlingX */

  if (tsd_p == NULL)
    IO_Exit_Error (R_SLING, X_LIBCALL,
      "NULL TSD provided to Sling_Canonical_Generate");

#ifdef _DEBUG_
printf("entering Sling_Canonical_Generate\n");
#endif
  DEBUG (R_SLING, DB_SLINGCANONGEN, TL_PARAMS, (outbuf, 
    "Entering Sling_Canonical_Generate, tsd = %p, map = %p, compaction = %u", 
    tsd_p, map_p, compaction));

  TIMETRACE (R_SLING, DB_SLINGCANONGEN, TL_TRACE, (outbuf,
    "Check point #0 in Sling_Canonical_Generate"));

  num_atoms = Tsd_NumAtoms_Get (tsd_p);
  for (atom = 0; atom < num_atoms ; atom++)
    {
    atomid = Tsd_Atomid_Get (tsd_p, atom);

    /* Check atom id of each atom in input tsd */

    newid = SIdCheck (atomid);
    Tsd_Atomid_Put (tsd_p, atom, newid * CANON_ATOMID_PERM);
    }

  TRACE_DO (DB_SLINGCANONGEN, TL_MAJOR,
    {
    IO_Put_Trace (R_SLING, "Permuted TSD:");
    STsdDump (tsd_p, &GTraceFile);
    });

  TIMETRACE (R_SLING, DB_SLINGCANONGEN, TL_TRACE, (outbuf,
    "Check point #1 in Sling_Canonical_Generate"));
#ifdef _DEBUG_
printf("SCG1\n");
#endif

  long_atoms = num_atoms;
  tsd_short_p = SCompact (tsd_p, &shortmap, compaction);
  short_atoms = Tsd_NumAtoms_Get (tsd_short_p);

  TRACE_DO (DB_SLINGCANONGEN, TL_MAJOR,
    {
    IO_Put_Trace (R_SLING, "Compacted TSD:");
    STsdDump (tsd_short_p, &GTraceFile);
    });

  TIMETRACE (R_SLING, DB_SLINGCANONGEN, TL_TRACE, (outbuf,
    "Check point #2, in Sling_Canonical_Generate"));
#ifdef _DEBUG_
printf("SCG2\n");
#endif

  /* Have to make sure that cebs, sebs, parityvector, uniqparity, asymmetric_p,
     are all allocated in TsdUnique
  */

  tsd_uniq_p = STsdUnique (tsd_short_p, parityvector_pp, cebs_p, sebs_p,
    &uniqparity, asymmetric_p, &uniqmap, chiral_p, long_atoms, num_carbons_p);
#ifdef _DEBUG_
printf("SCG2.5\n");
#endif

  Tsd_Destroy (tsd_short_p);

  TIMETRACE (R_SLING, DB_SLINGCANONGEN, TL_TRACE, (outbuf,
    "Check point #3, in Sling_Canonical_Generate"));
#ifdef _DEBUG_
printf("SCG3\n");
#endif

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("map_p", "sling{1}", map_p, num_atoms, WORDSIZE);
#else
  Array_1d_Create (map_p, num_atoms, WORDSIZE);
#endif

  for (atom = 0; atom < num_atoms; atom++)
    if (Array_1d16_Get (&shortmap, atom) != TSD_INVALID)
      {
      Array_1d16_Put (map_p, atom, Array_1d16_Get (&uniqmap, Array_1d16_Get (
        &shortmap, atom)));
      }
    else
      Array_1d16_Put (map_p, atom, TSD_INVALID);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&shortmap", "sling", &shortmap);
  mind_Array_Destroy ("&uniqmap", "sling", &uniqmap);
#else
  Array_Destroy (&shortmap);
  Array_Destroy (&uniqmap);
#endif

  /* Copy unique tsd into canonical (output) tsd */

  for (atom = 0; atom < num_atoms; atom++)
    {
    atomid = Tsd_Atomid_Get (tsd_uniq_p, atom) / CANON_ATOMID_PERM;
    Tsd_Atomid_Put (tsd_uniq_p, atom, SIdRestore (atomid));
    }

  TIMETRACE (R_SLING, DB_SLINGCANONGEN, TL_TRACE, (outbuf,
    "Check point #4, in Sling_Canonical_Generate"));
#ifdef _DEBUG_
printf("SCG4\n");
#endif

  TRACE_DO (DB_SLINGCANONGEN, TL_MAJOR,
    {
    IO_Put_Trace (R_SLING, "Canonical TSD:");
    Tsd_Dump (tsd_uniq_p, &GTraceFile);
    });

  sling = Tsd2SlingX (tsd_uniq_p, &uniqparity, cebs_p, sebs_p, asymmetric_p,
    paritybits_p, ce_str_p, se_str_p, short_atoms, *num_carbons_p);

  TRACE_DO (DB_SLINGCANONGEN, TL_MAJOR,
    {
    IO_Put_Trace (R_SLING, "CE class vector");
    Array_Dump (cebs_p, &GTraceFile);
    IO_Put_Trace (R_SLING, "SE class vector");
    Array_Dump (sebs_p, &GTraceFile);
    IO_Put_Trace (R_SLING, "Input to canonical TSD map");
    Array_Dump (map_p, &GTraceFile);
    IO_Put_Trace (R_SLING, "Canonical Sling");
    Sling_Dump (sling, &GTraceFile);
    IO_Put_Trace (R_SLING, "CE string");
    IO_Put_Trace (R_SLING, (char *)String_Value_Get (*ce_str_p));
    IO_Put_Trace (R_SLING, "SE string");
    IO_Put_Trace (R_SLING, (char *)String_Value_Get (*se_str_p));
    IO_Put_Trace (R_SLING, "Parity bits");
    Array_Dump (paritybits_p, &GTraceFile);
    IO_Put_Trace (R_SLING, "Asymmetric bits");
    Array_Dump (asymmetric_p, &GTraceFile);
    IO_Put_Trace (R_SLING, "Input TSD");
    STsdDump (tsd_p, &GTraceFile);
    IO_Put_Trace (R_SLING, "Canonical TSD");
    Tsd_Dump (tsd_uniq_p, &GTraceFile);
    });

  *canonical_tsd_pp = tsd_uniq_p;
  *compact_size_p = short_atoms;
  *canonical_str_p = Sling2String (sling);
  Sling_Destroy (sling); /* former memory leak! */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&uniqparity", "sling", &uniqparity);
#else
  Array_Destroy (&uniqparity);
#endif

  TIMETRACE (R_SLING, DB_SLINGCANONGEN, TL_TRACE, (outbuf,
    "Check point #5, Sling_Canonical_Generate"));
#ifdef _DEBUG_
printf("SCG5\n");
#endif

  DEBUG (R_SLING, DB_SLINGCANONGEN, TL_PARAMS, (outbuf,
    "Leaving Sling_Canonical_Generate, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Sling_Copy
*
*    This function makes a copy of a Sling.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Copy of Sling
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Sling_t Sling_Copy
  (
  Sling_t       input                      /* Sling to copy */
  )
{
  Sling_t       output;                    /* Result of copy */
  U8_t         *tmp;                       /* Temp. for allocation */

  DEBUG (R_SLING, DB_SLINGCOPY, TL_PARAMS, (outbuf,
    "Entering Sling_Copy, Sling not shown"));

#ifdef _MIND_MEM_
  mind_malloc ("tmp", "sling{2}", &tmp, Sling_Alloc_Get (input));
#else
  Mem_Alloc (U8_t *, tmp, Sling_Alloc_Get (input), GLOBAL);
#endif
  Sling_Name_Put (output, tmp);

  DEBUG (R_SLING, DB_SLINGCOPY, TL_MEMORY, (outbuf,
    "Allocated memory for a Sling name in Sling_Copy at %p",
    Sling_Name_Get (output)));

  if (Sling_Name_Get (output) == NULL)
    IO_Exit_Error (R_SLING, X_LIBCALL,
      "No memory for Sling name in Sling_Copy");

  memcpy (Sling_Name_Get (output), Sling_Name_Get (input), Sling_Alloc_Get (
    input));

  Sling_Length_Put (output, Sling_Length_Get (input));

  DEBUG (R_SLING, DB_SLINGCOPY, TL_PARAMS, (outbuf,
    "Leaving Sling_Copy, output not shown"));

  return output;
}


/****************************************************************************
*
*  Function Name:                 Sling_CopyTrunc
*
*    This routine makes a copy of the sling with the ceb's (and seb's)
*    truncated.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    allocates memory for sling.
*
******************************************************************************/
Sling_t Sling_CopyTrunc
  (
  Sling_t       input
  )
{
  Sling_t       trunc;                    /* Result of copy */
  char         *tmp;
  char         *end;
  char         *newstr;

  tmp = (char *) Sling_Name_Get (input);
  end = strchr (tmp, '|');
  if (end != NULL)
    {
    trunc = Sling_Create (end - tmp);
    newstr = (char *) Sling_Name_Get (trunc);
    memcpy (newstr, tmp, end - tmp);
    *(newstr + (end - tmp)) = '\0';
    }
  else
    trunc = Sling_Copy (input);

  return (trunc);
}
/*  End of Sling_CopyTrunc  */


/****************************************************************************
*
*  Function Name:                 Sling_Create
*
*    This function creates a new Sling, but only the name portion is allocated
*    in the heap.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    New Sling structure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Sling_t Sling_Create
  (
  U16_t         length                     /* # chars in Sling */
  )
{
  Sling_t       output;                    /* Result */
  U8_t         *tmp;                       /* For allocation */

  DEBUG (R_SLING, DB_SLINGCREATE, TL_PARAMS, (outbuf,
    "Entering Sling_Create, # chars = %u", length));

#ifdef _MIND_MEM_
  mind_malloc ("tmp", "sling{3}", &tmp, length + 1);
#else
  Mem_Alloc (U8_t *, tmp, length + 1, GLOBAL);
#endif
  Sling_Name_Put (output, tmp);

  DEBUG (R_SLING, DB_SLINGCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Sling name in Sling_Create at %p",
    Sling_Name_Get (output)));

  if (Sling_Name_Get (output) == NULL)
    IO_Exit_Error (R_SLING, X_LIBCALL,
      "No memory for Sling name in Sling_Create");

  memset (Sling_Name_Get (output), 0, length + 1);
  output.name[length] = '\0';
  Sling_Length_Put (output, length);

  DEBUG (R_SLING, DB_SLINGCREATE, TL_PARAMS, (outbuf,
    "Leaving Sling_Create, Sling not shown"));

  return output;
}

/****************************************************************************
*
*  Function Name:                 Sling_Destroy
*
*    This routine destroys a Sling structure.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Deallocates memory
*
******************************************************************************/
void Sling_Destroy
  (
  Sling_t       sling                      /* Sling to destroy */
  )
{
  DEBUG (R_SLING, DB_SLINGDESTROY, TL_PARAMS, (outbuf,
    "Entering Sling_Destroy, Sling not shown"));

  DEBUG_DO (DB_SLINGDESTROY, TL_MEMORY,
    {
    memset (Sling_Name_Get (sling), 0, Sling_Length_Get (sling) + 1);
    });

#ifdef _MIND_MEM_
  mind_free ("Sling_Name_Get(sling)", "sling", Sling_Name_Get (sling));
#else
  Mem_Dealloc (Sling_Name_Get (sling), Sling_Length_Get (sling) + 1, GLOBAL);
#endif

  DEBUG (R_SLING, DB_SLINGDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Sling name in Sling_Destroy at %p",
     Sling_Name_Get (sling)));

  DEBUG (R_SLING, DB_SLINGDESTROY, TL_PARAMS, (outbuf,
    "Leaving Sling_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Sling_Dump
*
*    This function prints a formatted dump of a Sling.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void Sling_Dump
  (
  Sling_t        sling,                      /* Sling to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (!Sling_Length_Get (sling) || Sling_Name_Get (sling) == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Sling\n");
    return;
    }

  DEBUG_ADDR (R_SLING, DB_SLINGCREATE, &sling);
  fprintf (f, "Dump of Sling = %s\n", Sling_Name_Get (sling));

  return;
}

/****************************************************************************
*
*  Function Name:                 Sling_Validate
*
*    This function validates the syntax of a Sling and calculates the number
*    of atoms in the Sling.
*
*  Used to be:
*
*    slingok:, slngtsd:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    TRUE - Sling has valid syntax
*    FALSE - Sling has invalid syntax
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Sling_Validate
  (
  Sling_t       sling,                     /* Sling to validate */
  U16_t        *num_atoms_p                /* Number of atoms found */
  )
{
  U8_t         *paren_p;                   /* For string traversal */
  U16_t         num_atoms;                 /* Number of atoms found */
  U16_t         i;                         /* Counter */
  S16_t         j, k;                      /* Useful counters */
  U8_t          next_char;                 /* Next character */
  Boolean_t     slash_found;               /* Flag for ring-closure */
  U8_t          atombuf[8];                /* Buffer for atom verification */
  S8_t          forward_pos[500];          /* To check forward validity */
  U8_t          forward_pos_index;
  S8_t          retrace_pos_count;         /* To check retrace validity */
  U8_t          trace_count;               /* Forward or retrace value */
  U8_t          trace_sign;                /* Forward or retrace sign */
  Slng2TsdCB_t  stcb;                      /* Control block */

  DEBUG (R_SLING, DB_SLINGVALIDATE, TL_PARAMS, (outbuf,
    "Entering Sling_Validate, Sling not shown"));

  if (num_atoms_p != NULL)
    *num_atoms_p = 0;

  FILL (stcb, 0);

  S2TCB_Offset_Put (&stcb, Sling_Name_Get (sling));
  S2TCB_End_Put (&stcb, Sling_Name_Get (sling) + Sling_Length_Get (sling) - 1);
  S2TCB_NextChar_Put (&stcb, Sling_Name_Get (sling)[0]);

  if (S2TCB_NextChar_Get (&stcb) == DONTCARE_SYM)
    {
    S2TCB_NextChar_Set (&stcb);
    }

  /* Find the number of atoms, validate the characters in the Sling */

  num_atoms = 0;
  forward_pos_index = 0;
  forward_pos[forward_pos_index++] = -1; /* no connection */
  retrace_pos_count = 0;
  slash_found = FALSE;

  for ( ; !S2TCB_AtSlingEnd (&stcb); )
    {
    next_char = *S2TCB_Offset_Get (&stcb);
#ifdef _DEBUG_
printf("seeking atom: next_char='%c' forward_pos_index=%d retrace_pos_count=%d\n",next_char,forward_pos_index,retrace_pos_count);
#endif
    if (isalpha (next_char))
      {
      atombuf[0] = next_char;
      atombuf[1] = '\0';
      if (!SAtomVerify (atombuf))
        {
        TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
          {
          Sling_Dump (sling, &GTraceFile);
          IO_Put_Trace (R_SLING,
            "Error in Sling_Validate, expected an atom, found something else");
          });
        return FALSE;
        }

      forward_pos[forward_pos_index++]=num_atoms++;
      retrace_pos_count++;
      S2TCB_NextChar_Set (&stcb);
      }
    else
      if (next_char == GROUP_START_SYM)
        {
        for (paren_p = S2TCB_Offset_Get (&stcb) + 1, i = 0; paren_p <=
             S2TCB_End_Get (&stcb) && *paren_p != GROUP_END_SYM && i <
             sizeof (atombuf); paren_p++, i++)
          atombuf[i] = *paren_p;

        if (paren_p > S2TCB_End_Get (&stcb) || i == sizeof (atombuf) - 1)
          {
          TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
            {
            Sling_Dump (sling, &GTraceFile);
            IO_Put_Trace (R_SLING,
              "Error in Sling_Validate, unmatched parentheses");
            });
          return FALSE;
          }
        else
          atombuf[i] = '\0';

        if (!SAtomVerify (atombuf))
          {
          TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
            {
            Sling_Dump (sling, &GTraceFile);
            IO_Put_Trace (R_SLING,
              "Error in Sling_Validate, bad non-atom found");
            });
          return FALSE;
          }

        S2TCB_Offset_Put (&stcb, paren_p + 1);
        forward_pos[forward_pos_index++]=num_atoms++;
        retrace_pos_count++;
        }
    else
      if (!slash_found)
        {
        TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
          {
          Sling_Dump (sling, &GTraceFile);
          IO_Put_Trace (R_SLING, "Error in Sling_Validate, bad character"
           " found, when atomic description expected");
          });
        return FALSE;
        }

    if (!S2TCB_AtSlingEnd (&stcb))
      {
      S2TCB_NextChar_Put (&stcb, *S2TCB_Offset_Get (&stcb));
      SParitySkip (&stcb);
      if (S2TCB_AtSlingEnd (&stcb))
        continue;

#ifdef _DEBUG_
printf("seeking other: next_char='%c' forward_pos_index=%d retrace_pos_count=%d\n",S2TCB_NextChar_Get(&stcb),forward_pos_index,retrace_pos_count);
#endif
      if ((S2TCB_NextChar_Get (&stcb) == FORTRACE_SYM) || (S2TCB_NextChar_Get (
          &stcb) == RETRACE_SYM) || isdigit (S2TCB_NextChar_Get (&stcb)))
        {
        trace_sign = FORTRACE_SYM; /* by default */

        if ((S2TCB_NextChar_Get (&stcb) == FORTRACE_SYM) || (S2TCB_NextChar_Get (
          &stcb) == RETRACE_SYM))
          {
          trace_sign = S2TCB_NextChar_Get (&stcb);

          S2TCB_NextChar_Set (&stcb);
          SLING_LOGICAL_END_CHECK (&stcb);
          }

        if (!isdigit (S2TCB_NextChar_Get (&stcb)))
          {
          TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
            {
            Sling_Dump (sling, &GTraceFile);
            IO_Put_Trace (R_SLING, "Error in Sling_Validate, bad character"
              " found when a number was expected");
            });
          return FALSE;
          }

        trace_count = SNumberRead (&stcb);
        if (trace_sign == FORTRACE_SYM)
          {
          if (trace_count >= num_atoms)
            {
            TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
              {
              Sling_Dump (sling, &GTraceFile);
              IO_Put_Trace (R_SLING, "Error in Sling_Validate, jump to"
                " atom position not yet encountered");
              });
            return (FALSE);
            }
          forward_pos[forward_pos_index++] = -1; /* no connection */
          forward_pos[forward_pos_index++] = trace_count;
          retrace_pos_count = 0;
          }
        else
          {
          if (retrace_pos_count - trace_count <= 0)
            {
            TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
              {
              Sling_Dump (sling, &GTraceFile);
              IO_Put_Trace (R_SLING, "Error in Sling_Validate, retrace to a"
                " position preceding beginning of sling");
              });
            return (FALSE);
            }
          retrace_pos_count -= trace_count;
          for (j = 0, k = forward_pos_index - 1; j < trace_count; j++) while (forward_pos[--k] < 0 && k > 0);
          if (k < 0)
            {
            printf ("Error in Sling_Validate: retrace exceeded forward_pos array\n");
            exit (1);
            }
          forward_pos[forward_pos_index++] = -1; /* no connection */
          forward_pos[forward_pos_index++] = forward_pos[k];
          }
        }

      SNumberSkip (&stcb);
      SLING_LOGICAL_END_CHECK (&stcb);
      SParitySkip (&stcb);

      if (S2TCB_AtSlingEnd (&stcb))
        continue;

      next_char = S2TCB_NextChar_Get (&stcb);
      if ((next_char == BOND_DOUBLE_SYM) || (next_char == BOND_TRIPLE_SYM) ||
          (next_char == BOND_RESONANT_SYM) || (next_char == BOND_NONE_SYM) ||
          (next_char == BOND_VARIABLE_SYM))
        {
        S2TCB_NextChar_Set (&stcb);
        SLING_LOGICAL_END_CHECK (&stcb);
        }
 
      slash_found = FALSE;
      if (S2TCB_NextChar_Get (&stcb) == SLASH_SYM)
        {
        S2TCB_NextChar_Set (&stcb);
        SLING_LOGICAL_END_CHECK (&stcb);

        trace_sign = FORTRACE_SYM; /* by default */

        if ((S2TCB_NextChar_Get (&stcb) == FORTRACE_SYM) ||
            (S2TCB_NextChar_Get (&stcb) == RETRACE_SYM))
          {
          trace_sign = S2TCB_NextChar_Get (&stcb);
          S2TCB_NextChar_Set (&stcb);
          SLING_LOGICAL_END_CHECK (&stcb);
          }

        if (!isdigit (S2TCB_NextChar_Get (&stcb)))
          {
          TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
            {
            Sling_Dump (sling, &GTraceFile);
            IO_Put_Trace (R_SLING, "Error in Sling_Validate, bad character\
 found when a number was expected");
            });
          return FALSE;
          }
        else
          {
          trace_count = SNumberRead (&stcb);
          if (trace_sign == FORTRACE_SYM)
            {
            if (trace_count >= num_atoms)
              {
              TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
                {
                Sling_Dump (sling, &GTraceFile);
                IO_Put_Trace (R_SLING, "Error in Sling_Validate, bonding to atom"
                  " position not yet encountered");
                });
              return (FALSE);
              }
            else if (forward_pos_index > 0 && trace_count == forward_pos[forward_pos_index - 1])
              {
#ifdef _DEBUG_
printf("trace_count=%d forward_pos_index=%d\n",trace_count,forward_pos_index);
#endif
              TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
                {
                Sling_Dump (sling, &GTraceFile);
                IO_Put_Trace (R_SLING, "Error in Sling_Validate, bonding current atom"
                  " to itself");
                });
              return (FALSE);
              }
            else if (forward_pos_index > 1 && forward_pos[forward_pos_index - 1] >= 0 &&
              trace_count == forward_pos[forward_pos_index - 2])
              {
#ifdef _DEBUG_
printf("trace_count=%d forward_pos_index=%d\n",trace_count,forward_pos_index);
#endif
              TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
                {
                Sling_Dump (sling, &GTraceFile);
                IO_Put_Trace (R_SLING, "Error in Sling_Validate, rebonding current atom"
                  " to its immediate neighbor");
                });
              return (FALSE);
              }
            forward_pos[forward_pos_index++]=trace_count;
            retrace_pos_count++;
            }
          else
            {
            if (retrace_pos_count - trace_count <= 0)
              {
              TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
                {
                Sling_Dump (sling, &GTraceFile);
                IO_Put_Trace (R_SLING, "Error in Sling_Validate, bonding to retrace"
                  " position preceding beginning of sling or previous retrace or jump");
                });
              return (FALSE);
              }
            else if (trace_count == 0)
              {
              TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
                {
                Sling_Dump (sling, &GTraceFile);
                IO_Put_Trace (R_SLING, "Error in Sling_Validate, bonding current atom"
                  " to itself");
                });
              return (FALSE);
              }
            else if (trace_count == 1)
              {
              TRACE_DO (DB_SLINGVALIDATE, TL_MAJOR,
                {
                Sling_Dump (sling, &GTraceFile);
                IO_Put_Trace (R_SLING, "Error in Sling_Validate, rebonding current atom"
                  " to its immediate neighbor");
                });
              return (FALSE);
              }
            retrace_pos_count -= trace_count;
            for (j = 0, k = forward_pos_index - 1; j < trace_count; j++) while (forward_pos[--k] < 0 && k > 0);
            if (k < 0)
              {
              printf ("Error in Sling_Validate: retrace exceeded forward_pos array\n");
              exit (1);
              }
            forward_pos[forward_pos_index++] = forward_pos[k];
            }
          SNumberSkip (&stcb);
          slash_found = TRUE;
          }
        }
      }          /* End if (!S2TCB_AtSlingEnd ) */
    }            /* End of for-nothing loop */

  if (num_atoms_p != NULL)
    *num_atoms_p = num_atoms;

  DEBUG (R_SLING, DB_SLINGVALIDATE, TL_PARAMS, (outbuf,
    "Leaving Sling_Validate, status = TRUE, # atoms = %u", num_atoms));

  return TRUE;
}

/****************************************************************************
*
*  Function Name:                 Sling2String
*
*    This function converts a Sling structure into a String structure.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    String structure of conversion
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
String_t Sling2String
  (
  Sling_t        sling                       /* Sling to convert */
  )
{
  String_t       string;                     /* Result of conversion */

  string = String_Create (NULL, Sling_Alloc_Get (sling));

  memcpy (String_Value_Get (string), Sling_Name_Get (sling),
   Sling_Alloc_Get (sling));

  String_Length_Put (string, Sling_Length_Get (sling));

  return string;
}

/****************************************************************************
*
*  Function Name:                 Sling2Tsd
*
*    This function converts a Sling format molecule into a TSD format 
*    molecule.  The first pass across the Sling counts the number of atoms
*    and verifys that all the characters in the Sling are correct and legal.
*    Then it creates a TSD and makes a second pass across the Sling to
*    extract the atom ids and bond parity information.  Then olefin bonds
*    are fixed up, and parity is switched for atoms for which it was noted
*    as wrong during the first pass.
*
*  Used to be:
*
*    slngtsd:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    TSD that is created, or NULL if Sling was defective
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Tsd_t *Sling2Tsd
  (
  Sling_t       sling                      /* Sling to convert */
  )
{
  Tsd_t        *tsd_p;                     /* Temporary TSD */
  Stack_t      *stack_p;                   /* Which atom id for retrace */
  U16_t         num_atoms;                 /* Number of atoms found */
  U16_t         atom;                      /* Counter */
  U16_t         temp_slot;                 /* For connecting atoms */
  U16_t         nextid;                    /* Next atom index in TSD */
  U16_t         neighid;                   /* Atom index of neighbor */
  U16_t         new_neighid;               /* Atom index of new neighbor */
  U8_t          neigh;                     /* Counter */
  U8_t          bondsize;                  /* Multiplicity of the bond */
  U8_t          num_olefins;               /* Olefin bond count */
  U8_t          bond_pos;                  /* Also neighbor index, olefin */
  Boolean_t     nostereo;                  /* Stereochemistry not used flag */
  Slng2TsdCB_t  stcb;                      /* Control block */
  Array_t       olefin_done;               /* 1-d bit, olefin checked flag */
  Array_t       parity_wrong;              /* 1-d bit, parity wrong flag */

  if (!Sling_Validate (sling, &num_atoms))
    {
    printf ("Invalid sling: %s\n", Sling_Name_Get (sling));
    exit (-1);
    }

  DEBUG (R_SLING, DB_SLING2TSD, TL_PARAMS, (outbuf,
    "Entering Sling2Tsd, sling not shown"));

  FILL (stcb, 0);

  S2TCB_Offset_Put (&stcb, Sling_Name_Get (sling));
  S2TCB_End_Put (&stcb, Sling_Name_Get (sling) + Sling_Length_Get (sling) - 1);
  S2TCB_NextChar_Put (&stcb, Sling_Name_Get (sling)[0]);

  if (S2TCB_NextChar_Get (&stcb) == DONTCARE_SYM)
    { 
    /* Ignore all stereochemistry, (i.e., all asymmetric centers are to be
       considered don't-cares)
    */

    nostereo = TRUE;
    S2TCB_NextChar_Set (&stcb);
    }
  else
    nostereo = FALSE;

  /* Start second pass.  Since we have reached here, no bad characters have 
     been found and we have counted up the number of atoms that appear in
     the Sling.  We are now ready to convert it into a TSD.
  */
  tsd_p = Tsd_Create (num_atoms);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&parity_wrong", "sling{4}", &parity_wrong, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&olefin_done", "sling{4}", &olefin_done, num_atoms, BITSIZE);
#else
  Array_1d_Create (&parity_wrong, num_atoms, BITSIZE);
  Array_1d_Create (&olefin_done, num_atoms, BITSIZE);
#endif
#ifdef _DEBUG_
printf("Array_Storage_Get(%p)=%p\n",&parity_wrong,Array_Storage_Get(&parity_wrong));
#endif
  stack_p = Stack_Create (STACK_SCALAR);

  Array_Set (&parity_wrong, FALSE);
  Array_Set (&olefin_done, FALSE);

  if (nostereo)
    {
    S2TCB_Offset_Put (&stcb, Sling_Name_Get (sling) + 1);
    }
  else
    S2TCB_Offset_Put (&stcb, Sling_Name_Get (sling));

  nextid = 0;
  S2TCB_Offset_Put (&stcb, SAtomRead (S2TCB_Offset_Get (&stcb), tsd_p,
    nextid));
  Stack_PushU16 (stack_p, nextid);
  nextid++;
  /* OK, let's go */

  if (!S2TCB_AtSlingEnd (&stcb))
    /* Check for ? and ~ at curpos */
    SParityCheck (tsd_p, &parity_wrong, Stack_TopU16 (stack_p), &stcb);

  for ( ; !S2TCB_AtSlingEnd (&stcb); )
    {
    S2TCB_NextChar_Put (&stcb, *S2TCB_Offset_Get (&stcb));
    switch (S2TCB_NextChar_Get (&stcb))
      {
      case BOND_DOUBLE_SYM:

        bondsize = BOND_DOUBLE;
        S2TCB_NextChar_Set (&stcb);
        break;

      case BOND_TRIPLE_SYM:

        bondsize = BOND_TRIPLE;
        S2TCB_NextChar_Set (&stcb);
        break;

      case BOND_RESONANT_SYM:

        bondsize = BOND_RESONANT;
        S2TCB_NextChar_Set (&stcb);
        break;

      case BOND_NONE_SYM:

        bondsize = BOND_NONE;
        S2TCB_NextChar_Set (&stcb);
        break;

      case BOND_VARIABLE_SYM:

        bondsize = BOND_VARIABLE;
        S2TCB_NextChar_Set (&stcb);
        break;

      default:

        bondsize = BOND_SINGLE;
        break;
      }

    if (S2TCB_NextChar_Get (&stcb) == SLASH_SYM)
      {
      S2TCB_NextChar_Set (&stcb);
      temp_slot = Stack_TopU16 (stack_p);
      SPointerMove (stack_p, &stcb);
      Tsd_Atom_Connect (tsd_p, Stack_TopU16 (stack_p), temp_slot,
        bondsize);
      }
    else
      {
      S2TCB_Offset_Put (&stcb, SAtomRead (S2TCB_Offset_Get (&stcb), tsd_p,
        nextid));
      S2TCB_NextChar_Put (&stcb, *S2TCB_Offset_Get (&stcb));
      Tsd_Atom_Connect (tsd_p, Stack_TopU16 (stack_p), nextid, bondsize);
      Stack_PushU16 (stack_p, nextid);
      nextid++;
      }

    if (!S2TCB_AtSlingEnd (&stcb))
      {
      SParityCheck (tsd_p, &parity_wrong, Stack_TopU16 (stack_p), &stcb);

      if (S2TCB_AtSlingEnd (&stcb))
        continue;

      SPointerMove (stack_p, &stcb);
      SParityCheck (tsd_p, &parity_wrong, Stack_TopU16 (stack_p), &stcb);
      }
    }                /* End of for-nothing loop */
  /* Set (optionally) don't-care and asymmetry bits */

  if (nostereo)
    for (atom = 0; atom < num_atoms; atom++)
      {
      Tsd_AtomFlags_DontCare_Put (tsd_p, atom, TRUE);
      Tsd_AtomFlags_Asymmetry_Put (tsd_p, atom, TRUE);
      }
  else
    for (atom = 0; atom < num_atoms; atom++)
      Tsd_AtomFlags_Asymmetry_Put (tsd_p, atom, TRUE);

  DEBUG (R_SLING, DB_SLING2TSD, TL_MAJOR, (outbuf,
    "Finished second pass of Sling2Tsd, now to clean up"));

  /* Put olefin carbons in desired form for TSD
     Find carbons which are at the ends of unprocessed olefin chains
  */

  for (atom = 0; atom < num_atoms; atom++)
    {
    if (Tsd_Atomid_Get (tsd_p, atom) == CARBON && !Array_1d1_Get (&olefin_done,
        atom))
      {
      /* See if the carbon atom has 1 olefin bond */

      for (neigh = 0, num_olefins = 0; neigh < MX_NEIGHBORS; neigh++)
        if (Tsd_Atom_NeighborId_Get (tsd_p, atom, neigh) != TSD_INVALID)
          if ((Tsd_Atomid_Get (tsd_p, Tsd_Atom_NeighborId_Get (tsd_p, atom,
               neigh)) == CARBON) && Tsd_Atom_NeighborBond_Get (tsd_p, atom,
               neigh) ==  BOND_DOUBLE)
            {
            num_olefins++;
            bond_pos = neigh;
            }
      if (num_olefins == 1)
        {
        /* Olefin found.  Put olefin bond in "in" column, remaining bonds in
           "up" and "down" columns so as to correctly represent parity.
        */

        neighid = Tsd_Atom_NeighborId_Get (tsd_p, atom, bond_pos);

        /* Fix the atom row of tsd and bond mult array */

        Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_IN, neighid);
        Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_IN,
          Tsd_Atom_NeighborBond_Get (tsd_p, atom, bond_pos));

        /* Moves the non-olefin bonds to the up and down columns so as to
           reflect the proper parity.
        */

        SOlefinAtomFix (tsd_p, &olefin_done, atom, bond_pos);

        for (neigh = 0, num_olefins = 0; neigh < MX_NEIGHBORS; neigh++)
          if (Tsd_Atom_NeighborId_Get (tsd_p, neighid, neigh) != TSD_INVALID)
            if ((Tsd_Atomid_Get (tsd_p, Tsd_Atom_NeighborId_Get (tsd_p,
                neighid, neigh)) == CARBON) &&
                Tsd_Atom_NeighborBond_Get (tsd_p, neighid, neigh) ==
                BOND_DOUBLE)
              num_olefins++;

        /* Do for the entire olefin chain */

        while (num_olefins == 2)
          {
          /* While still in the middle of the olefin chain, put bond to the
             previous atom in "out" column and bond to next atom in "in"
             column.
          */

          for (neigh = 0; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (
               tsd_p, neighid, neigh) != atom; neigh++)
            /* Empty loop body */ ;
   
          if (neigh == BOND_DIR_UP)
            {
            new_neighid = Tsd_Atom_NeighborId_Get (tsd_p, neighid,
              BOND_DIR_DOWN);
            Tsd_Atom_NeighborId_Put (tsd_p, neighid, BOND_DIR_OUT,
              Tsd_Atom_NeighborId_Get (tsd_p, neighid, BOND_DIR_UP));
            Tsd_Atom_NeighborId_Put (tsd_p, neighid, BOND_DIR_IN, new_neighid);
            Tsd_Atom_NeighborBond_Put (tsd_p, neighid, BOND_DIR_OUT,
              Tsd_Atom_NeighborBond_Get (tsd_p, neighid, BOND_DIR_UP));
            Tsd_Atom_NeighborBond_Put (tsd_p, neighid, BOND_DIR_IN,
              Tsd_Atom_NeighborBond_Get (tsd_p, neighid, BOND_DIR_DOWN));
            }
          else
            {
            new_neighid = Tsd_Atom_NeighborId_Get (tsd_p, neighid,
              BOND_DIR_UP);
            Tsd_Atom_NeighborId_Put (tsd_p, neighid, BOND_DIR_OUT,
              Tsd_Atom_NeighborId_Get (tsd_p, neighid, BOND_DIR_DOWN));
            Tsd_Atom_NeighborId_Put (tsd_p, neighid, BOND_DIR_IN, new_neighid);
            Tsd_Atom_NeighborBond_Put (tsd_p, neighid, BOND_DIR_IN,
              Tsd_Atom_NeighborBond_Get (tsd_p, neighid, BOND_DIR_UP));
            Tsd_Atom_NeighborBond_Put (tsd_p, neighid, BOND_DIR_OUT, 
              Tsd_Atom_NeighborBond_Get (tsd_p, neighid, BOND_DIR_DOWN));
            }

          Tsd_Atom_NeighborId_Put (tsd_p, neighid, BOND_DIR_UP, TSD_INVALID);
          Tsd_Atom_NeighborId_Put (tsd_p, neighid, BOND_DIR_DOWN, TSD_INVALID);
          Tsd_Atom_NeighborBond_Put (tsd_p, neighid, BOND_DIR_UP, BOND_NONE);
          Tsd_Atom_NeighborBond_Put (tsd_p, neighid, BOND_DIR_DOWN, BOND_NONE);

          atom = neighid;
          Array_1d1_Put (&olefin_done, atom, TRUE);
          neighid = new_neighid;
          for (neigh = 0, num_olefins = 0; neigh < MX_NEIGHBORS; neigh++)
            if (Tsd_Atom_NeighborId_Get (tsd_p, neighid, neigh) != TSD_INVALID)
              if ((Tsd_Atomid_Get (tsd_p, Tsd_Atom_NeighborId_Get (tsd_p,
                  neighid, neigh)) == CARBON) && Tsd_Atom_NeighborBond_Get (
                  tsd_p, neighid, neigh) == BOND_DOUBLE)
                num_olefins++;
          }                      /* End while num_olefins == 2 loop */

        if (num_olefins == 1)
          {
          /* Put bond to olefin chain in "out" column and correctly sort
             remaining bonds.
          */

          for (neigh = 0; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (
               tsd_p, neighid, neigh) != atom; neigh++)
            /* Empty loop body */ ;

          Tsd_Atom_NeighborId_Put (tsd_p, neighid, BOND_DIR_OUT,
            Tsd_Atom_NeighborId_Get (tsd_p, neighid, neigh));
          Tsd_Atom_NeighborBond_Put (tsd_p, neighid, BOND_DIR_OUT,
            Tsd_Atom_NeighborBond_Get (tsd_p, neighid, neigh));
          SOlefinAtomFix (tsd_p, &olefin_done, neighid, neigh);
          } 
        }                         /* End of if num_olefins == 1 */
      }                           /* End if Carbon atom & !OlefinDone (atom) */
    }                             /* End for-atoms loop */
  for (atom = 0; atom < num_atoms; atom++)
    if (Array_1d1_Get (&parity_wrong, atom))
      {
      neighid = Tsd_Atom_NeighborId_Get (tsd_p, atom, BOND_DIR_UP);
      Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_UP,
        Tsd_Atom_NeighborId_Get (tsd_p, atom, BOND_DIR_DOWN));
      Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_DOWN, neighid);
      bondsize = Tsd_Atom_NeighborBond_Get (tsd_p, atom, BOND_DIR_UP);
      Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_UP,
        Tsd_Atom_NeighborBond_Get (tsd_p, atom, BOND_DIR_DOWN));
      Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_DOWN, bondsize);
      }

#ifdef _DEBUG_
printf("Array_Storage_Get(%p)=%p\n",&parity_wrong,Array_Storage_Get(&parity_wrong));
#endif
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&parity_wrong", "sling", &parity_wrong);
  mind_Array_Destroy ("&olefin_done", "sling", &olefin_done);
#else
  Array_Destroy (&parity_wrong);
  Array_Destroy (&olefin_done);
#endif
  Stack_Destroy (stack_p);

  DEBUG (R_SLING, DB_SLING2TSD, TL_PARAMS, (outbuf,
    "Leaving Sling2Tsd, tsd = %p", tsd_p));

  return tsd_p;
}

/****************************************************************************
*
*  Function Name:                 Sling2Tsd_PlusHydrogen
*
*    This function converts a Sling into a TSD and adds all the implied 
*    Hydrogen atoms that may not have been in the Sling.
*
*  Used to be:
*
*    hslntsd:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Address of TSD
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Tsd_t *Sling2Tsd_PlusHydrogen
  (
  Sling_t       sling                      /* Sling to convert */
  )
{
  Tsd_t        *tsd_p;                     /* Result */
  U16_t         num_atoms;                 /* # atoms in molecule */
  U16_t         old_numatoms;              /* # atoms before hydrogenation */
  U16_t         hyd_idx;                   /* Atom index for hydrogen */
  U16_t         atom;                      /* Counter */
  U8_t          neigh;                     /* Counter */
  U8_t          bonds;                     /* Bonds used for this atom */
  U8_t          num_resonant;              /* # resonant bonds for this atom */
  Array_t       num_hyd;                   /* 1-d byte, # hydrogens per atom */

  DEBUG (R_SLING, DB_SLING2TSDPLHY, TL_PARAMS, (outbuf,
    "Entering Sling2Tsd_PlusHydrogen, Sling not shown"));

  tsd_p = Sling2Tsd (sling);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&num_hyd", "sling{5}", &num_hyd, Tsd_NumAtoms_Get (tsd_p), BYTESIZE);
#else
  Array_1d_Create (&num_hyd, Tsd_NumAtoms_Get (tsd_p), BYTESIZE);
#endif

  for (atom = 0, num_atoms = 0; atom < Tsd_NumAtoms_Get (tsd_p); atom++)
    if (Tsd_Atomid_Get (tsd_p, atom) == CARBON)
      {
      for (neigh = 0, num_resonant = 0, bonds = 0; neigh < MX_NEIGHBORS;
           neigh++)
        {
        if (Tsd_Atom_NeighborBond_Get (tsd_p, atom, neigh) == BOND_RESONANT)
          {
          num_resonant++;
          if (((num_resonant % 2) == 0))
            bonds += 2;
          else
            bonds += 1;
          }
        else
          bonds += Tsd_Atom_NeighborBond_Get (tsd_p, atom, neigh);
        }

      if (bonds < 4)         /* Carbon valence is 4 */
        {
        Array_1d8_Put (&num_hyd, atom, 4 - bonds);
        num_atoms += (4 - bonds);
        }
      else
        Array_1d8_Put (&num_hyd, atom, 0);

      }
    else
      Array_1d8_Put (&num_hyd, atom, 0);

  if (!num_atoms)
    {
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&num_hyd", "sling", &num_hyd);
#else
    Array_Destroy (&num_hyd);
#endif

    DEBUG (R_SLING, DB_SLING2TSDPLHY, TL_PARAMS, (outbuf,
      "Leaving Sling2Tsd_PlusHydrogen, no hydrogens added, tsd = %p", tsd_p));

    return tsd_p;
    }

  old_numatoms = Tsd_NumAtoms_Get (tsd_p);
  Tsd_Expand (tsd_p, old_numatoms + num_atoms);

  for (atom = 0, hyd_idx = old_numatoms; atom < old_numatoms; atom++)
    for (neigh = 0; neigh < Array_1d8_Get (&num_hyd, atom); neigh++)
      {
      Tsd_Atomid_Put (tsd_p, hyd_idx, HYDROGEN);
      Tsd_Atom_Connect (tsd_p, atom, hyd_idx, BOND_SINGLE);
      hyd_idx++;
      }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&num_hyd", "sling", &num_hyd);
#else
  Array_Destroy (&num_hyd);
#endif

  DEBUG (R_SLING, DB_SLING2TSDPLHY, TL_PARAMS, (outbuf,
    "Leaving Sling2Tsd_PlusHydrogen, tsd = %p", tsd_p));

  return tsd_p;
}

/****************************************************************************
*
*  Function Name:                 String2Sling
*
*    This function converts a String structure into a Sling structure.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Sling structure of conversion
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Sling_t String2Sling
  (
  String_t       string                      /* String to convert */
  )
{
  Sling_t        sling;                      /* Result of conversion */

  sling = Sling_Create (String_Length_Get (string));

  memcpy (Sling_Name_Get (sling), String_Value_Get (string),
   String_Length_Get (string));

  return sling;
}

/****************************************************************************
*
*  Function Name:                 Tsd2Sling
*
*    This function converts a TSD molecule format into a Sling format.
*
*  Used to be:
*
*    tsdslna:, somewhat tsdslng:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Sling format of molecule
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Sling_t Tsd2Sling
  (
  Tsd_t        *tsd_p                      /* Molecule to convert */
  )
{
  Tsd_t        *tsd_tmp;                   /* Copy of TSD to munge */
  Stack_t      *stack_p;                   /* For traversing the m'cule */
  U16_t         num_atoms;                 /* # atoms in molecule */
  U16_t         retrace;                   /* Number of atoms to retrace */
  U16_t         atom;                      /* Counter */
  U16_t         pos;                       /* Index into Sling */
  U16_t         perm;                      /* Counter */
  S16_t         atomid;                    /* For conversion */
  S16_t         neighid;                   /* Neighbor's atom index */
  U8_t          neigh;                     /* Counter */
  U8_t          k;                         /* Counter */
  char          tbuf[6];                   /* Convert word to string */
  String_t      output;                    /* Result of the operation */
  String_t      temp_str;                  /* For conversion */
  Sling_t       sling;                     /* Output format */
  Array_t       bond_mult;                 /* 2-d byte, bond multiplicities */
  Array_t       parity_ok;                 /* 1-d bit, done with parity flag */
  Array_t       in_stack;                  /* 1-d bit, mark atom as in stack */
  Array_t       sling_pos;                 /* 1-d word, offsets into Sling */

  FILL (output, 0);

  if (tsd_p == NULL || !Tsd_NumAtoms_Get (tsd_p))
    return String2Sling (output);

  DEBUG (R_TSD, DB_TSD2SLING, TL_PARAMS, (outbuf,
    "Entering Tsd2Sling, tsd = %p", tsd_p));

  tsd_tmp = Tsd_Copy (tsd_p);
  num_atoms = Tsd_NumAtoms_Get (tsd_tmp);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&in_stock", "sling{6}", &in_stack, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&parity_ok", "sling{6}", &parity_ok, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&sling_pos", "sling{6}", &sling_pos, num_atoms, WORDSIZE);
  mind_Array_2d_Create ("&bond_mult", "sling{6}", &bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#else
  Array_1d_Create (&in_stack, num_atoms, BITSIZE);
  Array_1d_Create (&parity_ok, num_atoms, BITSIZE);
  Array_1d_Create (&sling_pos, num_atoms, WORDSIZE);
  Array_2d_Create (&bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#endif
  Array_Set (&in_stack, FALSE);
  Array_Set (&parity_ok, TRUE);
  Array_Set (&bond_mult, 0);
  output = String_Create (NULL, num_atoms << 2);

  /* Number of atoms to retrace is initially zero.  The position in the sling
     of the first atom is zero, and the index for the next one is one.
  */

  retrace = 0;
  Array_1d16_Put (&sling_pos, 0, 0);
  pos = 1;

  for (atom = 0; atom < num_atoms; atom++)
    {
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_NONE)
        {
        Array_2d8_Put (&bond_mult, atom, neigh, BOND_NONE_SYM);
        }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_SINGLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_SINGLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_DOUBLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_DOUBLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_TRIPLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_TRIPLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_VARIABLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_VARIABLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_RESONANT)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_RESONANT_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == (BOND_SINGLE
            | BOND_RESONANT))
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_SINGLE_OR_RESONANT_SYM);
          }
      else
        IO_Exit_Error (R_TSD, X_SYNERR,
          "Illegal bond found in TSD in Tsd2Sling");
      }
    }                      /* End for-atom loop */

  stack_p = Stack_Create (STACK_SCALAR);
  atomid = Tsd_Atomid_Get (tsd_tmp, 0);
  temp_str = SLetter (atomid);
  String_Concat (&output, temp_str);
  String_Destroy (temp_str);

  Stack_PushU16 (stack_p, 0);
  Array_1d1_Put (&in_stack, 0, TRUE);

  /* Continue while there remain bonds to be processed */

  while (Stack_Size_Get (stack_p) > 0)
    {
    atom = Stack_TopU16 (stack_p);

    DEBUG (R_TSD, DB_TSD2SLING, TL_MAJOR, (outbuf,
      "Sling = %s", String_Value_Get (output)));

    for (neigh = 0; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (tsd_tmp,
         atom, neigh) == TSD_INVALID; neigh++)
      /* Empty loop body */ ;

    /* Atom on top of stack has more neighbors */

    if (neigh < MX_NEIGHBORS)
      {
      if (retrace > 0)
        {
        String_Concat_c (&output, RETRACE_STR);
        Number2Char (retrace, tbuf);
        String_Concat_c (&output, tbuf);
        }

      retrace = 0;

      if (!Array_1d1_Get (&parity_ok, atom))
        {
        k = neigh;
        neigh++;
        for (; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (tsd_tmp,
             atom, neigh) == TSD_INVALID; neigh++)
          /* Empty loop body */ ;

        /* There is only one bond on which to exit from this atom, therefore
           since the parity is wrong . . .
        */

        if (neigh >= MX_NEIGHBORS)
          {
          String_Concat_c (&output, PARITY_SPACE_STR);
          neigh = k;
          }
        else
          Array_1d1_Put (&parity_ok, atom, TRUE);
        }

      neighid = Tsd_Atom_NeighborId_Get (tsd_tmp, atom, neigh);
      for (k = 0, perm = 0; k < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (
           tsd_tmp, neighid, k) != atom; k++)
        if (Tsd_Atom_NeighborId_Get (tsd_tmp, neighid, k) != TSD_INVALID)
          perm++;

      if ((perm % 2) == 1 && Tsd_Atomid_Get (tsd_tmp, neighid) == CARBON)
        Array_1d1_Put (&parity_ok, neighid, FALSE);

      Tsd_Atom_NeighborId_Put (tsd_tmp, atom, neigh, TSD_INVALID);
      Tsd_Atom_NeighborBond_Put (tsd_tmp, atom, neigh, BOND_NONE);
      Tsd_Atom_NeighborId_Put (tsd_tmp, neighid, k, TSD_INVALID);
      Tsd_Atom_NeighborBond_Put (tsd_tmp, neighid, k, BOND_NONE);
      tbuf[0] = Array_2d8_Get (&bond_mult, atom, neigh);
      tbuf[1] = '\0';
      String_Concat_c (&output, tbuf);

      if (!Array_1d1_Get (&in_stack, neighid))
        {
        temp_str = SLetter (Tsd_Atomid_Get (tsd_tmp, neighid));
        String_Concat (&output, temp_str);
        String_Destroy (temp_str);
        Array_1d1_Put (&in_stack, neighid, TRUE);
        Array_1d16_Put (&sling_pos, neighid, pos);
        pos++;
        }
      else
        {
        String_Concat_c (&output, SLASH_STR);
        Number2Char (Array_1d16_Get (&sling_pos, neighid), tbuf);
        String_Concat_c (&output, tbuf);
        }

      Stack_PushU16 (stack_p, neighid);
      }                      /* End if-neigh < MX_NEIGHBORS */
    else
      {
      Stack_Pop (stack_p);
      retrace++;
      }
    }                        /* End while-stack_size loop */

  sling = String2Sling (output);
  String_Destroy (output);
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&in_stack", "sling", &in_stack);
  mind_Array_Destroy ("&parity_ok", "sling", &parity_ok);
  mind_Array_Destroy ("&sling_pos", "sling", &sling_pos);
  mind_Array_Destroy ("&bond_mult", "sling", &bond_mult);
#else
  Array_Destroy (&in_stack);
  Array_Destroy (&parity_ok);
  Array_Destroy (&sling_pos);
  Array_Destroy (&bond_mult);
#endif
  Stack_Destroy (stack_p);
  Tsd_Destroy (tsd_tmp);

  DEBUG (R_TSD, DB_TSD2SLING, TL_PARAMS, (outbuf,
    "Leaving Tsd2Sling, sling not shown"));

  return sling;
}

/****************************************************************************
*
*  Function Name:                 Tsd2SlingX
*
*    This function converts a TSD molecule into a Sling format molecule, plus
*    it fills in a bunch of ancilliary information about the molecule.
*
*  Used to be:
*
*    tsdslnb:, somewhat tsdslnc:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Sling_t Tsd2SlingX
  (
  Tsd_t        *tsd_p,                     /* Molecule to convert */
  Array_t      *parityvec_p,               /* 1-d byte, parity vector */
  Array_t      *cebros_p,                  /* 1-d word, CE brothers */
  Array_t      *sebros_p,                  /* 1-d word, SE brothers */
  Array_t      *asymmetric_p,              /* 1-d bit, asymmetric flags */
  Array_t      *paritybits_p,              /* 1-d bit, parity flags (Out) */
  String_t     *ce_str_p,                  /* CE string format */
  String_t     *se_str_p,                  /* SE string format */
  U16_t         num_bros,                  /* Number of entries in C/SE bros */
  U16_t         num_carbons                /* Number of carbons found */
  )
{
  Tsd_t        *tsd_tmp;                   /* Copy of TSD to munge */
  Stack_t      *stack_p;                   /* For traversing the m'cule */
  U16_t         num_atoms;                 /* # atoms in molecule */
  U16_t         last_carbon;               /* Carbon index */
  U16_t         retrace;                   /* Number of atoms to retrace */
  U16_t         atom;                      /* Counter */
  U16_t         pos;                       /* Index into Sling */
  U16_t         perm;                      /* Counter */
  U16_t         atomid;                    /* For conversion */
  U16_t         neighid;                   /* Neighbor's atom index */
  U16_t          neigh;                     /* Counter */
  U16_t          k;                         /* Counter */
  char          tbuf[6];                   /* Convert word to string */
  String_t      output;                    /* Result of the operation */
  String_t      temp_str;                  /* For conversion */
  Sling_t       sling;                     /* For output */
  Array_t       bond_mult;                 /* 2-d byte, bond multiplicities */
  Array_t       parity_ok;                 /* 1-d bit, done with parity flag */
  Array_t       in_stack;                  /* 1-d bit, mark atom as in stack */
  Array_t       sling_pos;                 /* 1-d word, offsets into Sling */
  Array_t       parityvec_pos;             /* 1-d word, parity vector offset */
  Array_t       sorted_parity;             /* 2-d byte, sorted by Sling ord */
  Array_t       dontcare;                  /* 1-d bit, keep hydrogens? */

  FILL (output, 0);

  if (tsd_p == NULL || !Tsd_NumAtoms_Get (tsd_p))
    return String2Sling (output);

  DEBUG (R_TSD, DB_TSD2SLINGX, TL_PARAMS, (outbuf,
    "Entering Tsd2SlingX, tsd = %p, # brothers = %u, # carbons = %u",
    tsd_p, num_bros, num_carbons));

  TRACE_DO (DB_TSD2SLINGX, TL_MAJOR,
    {
    IO_Put_Trace (R_TSD, "Parity vector in Tsd2SlingX");
    Array_Dump (parityvec_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "CE Brothers");
    Array_Dump (cebros_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "SE Brothers");
    Array_Dump (sebros_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "Asymmetric flags");
    Array_Dump (asymmetric_p, &GTraceFile);
    });

  tsd_tmp = Tsd_Copy (tsd_p);
  num_atoms = Tsd_NumAtoms_Get (tsd_tmp);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&in_stack", "sling{7}", &in_stack, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&parity_ok", "sling{7}", &parity_ok, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&dontcare", "sling{7}", &dontcare, num_atoms, BITSIZE);
  mind_Array_1d_Create ("&sling_pos", "sling{7}", &sling_pos, num_atoms, WORDSIZE);
  mind_Array_2d_Create ("&bond_mult", "sling{7}", &bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#else
  Array_1d_Create (&in_stack, num_atoms, BITSIZE);
  Array_1d_Create (&parity_ok, num_atoms, BITSIZE);
  Array_1d_Create (&dontcare, num_atoms, BITSIZE);
  Array_1d_Create (&sling_pos, num_atoms, WORDSIZE);
  Array_2d_Create (&bond_mult, num_atoms, MX_NEIGHBORS, BYTESIZE);
#endif
  Array_Set (&in_stack, FALSE);
  Array_Set (&dontcare, FALSE);
  Array_Set (&parity_ok, TRUE);
  Array_Set (&bond_mult, 0);
  output = String_Create (NULL, num_atoms << 2);

  retrace = 0;
  Array_1d16_Put (&sling_pos, 0, 0);
  pos = 1;

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    {
    if (Tsd_Atomid_Get (tsd_tmp, atom) != CARBON)
      Array_1d1_Put (&dontcare, atom, TRUE);
    else
      {
      Array_1d1_Put (&dontcare, atom, Array_1d8_Get (parityvec_p,
        last_carbon) == PARITY_INIT);
      last_carbon++;
      }

    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      if (Tsd_Atomid_Get (tsd_tmp, atom) == CARBON && Array_1d1_Get (
            &dontcare, atom) == TRUE)
        if (Tsd_Atom_NeighborId_Get (tsd_tmp, atom, neigh) != TSD_INVALID &&
            Tsd_Atomid_Get (tsd_tmp, Tsd_Atom_NeighborId_Get (tsd_tmp, atom,
            neigh)) == HYDROGEN)
          Tsd_Atom_NeighborId_Put (tsd_tmp, atom, neigh, TSD_INVALID);

      if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_NONE)
        {
        Array_2d8_Put (&bond_mult, atom, neigh, BOND_NONE_SYM);
        }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_SINGLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_SINGLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_DOUBLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_DOUBLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_TRIPLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_TRIPLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_VARIABLE)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_VARIABLE_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == BOND_RESONANT)
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_RESONANT_SYM);
          }
      else
        if (Tsd_Atom_NeighborBond_Get (tsd_tmp, atom, neigh) == (BOND_SINGLE
            | BOND_RESONANT))
          {
          Array_2d8_Put (&bond_mult, atom, neigh, BOND_SINGLE_OR_RESONANT_SYM);
          }
      else
        IO_Exit_Error (R_TSD, X_SYNERR,
          "Illegal bond found in TSD in Tsd2SlingX");
      }
    }

  stack_p = Stack_Create (STACK_SCALAR);
  atomid = Tsd_Atomid_Get (tsd_tmp, 0);
  temp_str = SLetter (atomid);
  String_Concat (&output, temp_str);
  String_Destroy (temp_str);

  Stack_PushU16 (stack_p, 0);
  Array_1d1_Put (&in_stack, 0, TRUE);

  /* Continue while there remain bonds to be processed */

  while (Stack_Size_Get (stack_p) > 0)
    {
    atom = Stack_TopU16 (stack_p);

    DEBUG (R_TSD, DB_TSD2SLINGX, TL_MAJOR, (outbuf,
      "Sling = %s", String_Value_Get (output)));

    for (neigh = 0; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (tsd_tmp,
         atom, neigh) == TSD_INVALID; neigh++)
      /* Empty loop body */ ;

    /* Atom on top of stack has more neighbors */

    if (neigh < MX_NEIGHBORS)
      {
      if (retrace > 0)
        {
        String_Concat_c (&output, RETRACE_STR);
        Number2Char (retrace, tbuf);
        String_Concat_c (&output, tbuf);
        }

      retrace = 0;

      if (Array_1d1_Get (&parity_ok, atom) == FALSE)
        {
        k = neigh;
        neigh++;
        for (; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (tsd_tmp,\
             atom, neigh) == TSD_INVALID; neigh++)
          /* Empty loop body */ ;

        /* There is only one bond on which to exit from this atom, therefore
           since the parity is wrong . . .
        */

        if (neigh >= MX_NEIGHBORS)
          {
          String_Concat_c (&output, PARITY_SPACE_STR);
          neigh = k;
          }
        else
          Array_1d1_Put (&parity_ok, atom, TRUE);
        }

      neighid = Tsd_Atom_NeighborId_Get (tsd_tmp, atom, neigh);
      for (k = 0, perm = 0; k < MX_NEIGHBORS && Tsd_Atom_NeighborId_Get (
           tsd_tmp, neighid, k) != atom; k++)
        if (Tsd_Atom_NeighborId_Get (tsd_tmp, neighid, k) != TSD_INVALID)
          perm++;

      if ((perm % 2) == 1 && Array_1d1_Get (&dontcare, neighid) == FALSE)
        Array_1d1_Put (&parity_ok, neighid, FALSE);

      Tsd_Atom_NeighborId_Put (tsd_tmp, atom, neigh, TSD_INVALID);
      Tsd_Atom_NeighborId_Put (tsd_tmp, neighid, k, TSD_INVALID);
      tbuf[0] = Array_2d8_Get (&bond_mult, atom, neigh);
      tbuf[1] = '\0';
      String_Concat_c (&output, tbuf);

      if (Array_1d1_Get (&in_stack, neighid) == FALSE)
        {
        temp_str = SLetter (Tsd_Atomid_Get (tsd_tmp, neighid));
        String_Concat (&output, temp_str);
        String_Destroy (temp_str);
        Array_1d1_Put (&in_stack, neighid, TRUE);
        Array_1d16_Put (&sling_pos, neighid, pos);
        pos++;
        }
      else
        {
        String_Concat_c (&output, SLASH_STR);
        Number2Char (Array_1d16_Get (&sling_pos, neighid), tbuf);
        String_Concat_c (&output, tbuf);
        }

      Stack_PushU16 (stack_p, neighid);
      }                      /* End if-neigh < MX_NEIGHBORS */
    else
      {
      Stack_Pop (stack_p);
      retrace++;
      }
    }                        /* End while-stack_size loop */

  Tsd_Destroy (tsd_tmp);
  tsd_tmp = Tsd_Copy (tsd_p);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&parityvec_pos", "sling{7}", &parityvec_pos, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("paritybits_p", "sling{7}", paritybits_p, num_carbons, BITSIZE);
  mind_Array_2d_Create ("&sorted_parity", "sling{7}", &sorted_parity, num_atoms, 2, BYTESIZE);
#else
  Array_1d_Create (&parityvec_pos, num_atoms, WORDSIZE);
  Array_1d_Create (paritybits_p, num_carbons, BITSIZE);
  Array_2d_Create (&sorted_parity, num_atoms, 2, BYTESIZE);
#endif
  Array_Set (&parityvec_pos, 0);
  Array_Set (paritybits_p, FALSE);
  Array_Set (&sorted_parity, PARITY_INVALID1);

  /* Note the position in the parity vector for each carbon in the molecule */

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    if (Tsd_Atomid_Get (tsd_tmp, atom) == CARBON)
      {
      Array_1d16_Put (&parityvec_pos, atom, last_carbon);
      last_carbon++;
      }

  /* Sort the parities according to the order in which the carbons appear in
     the Sling.  Also change the parity of one end of a +2 olefin bond to 0.
     This is necessary due to the different ways in which cis and trans
     parities about olefin bonds are represented in the nomenclature algorithm
     and in the Sling.
  */

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    {
    if (Tsd_Atomid_Get (tsd_tmp, atom) == CARBON)
      {
      Array_2d8_Put (&sorted_parity, Array_1d16_Get (&sling_pos, atom), 0,
        Array_1d8_Get (parityvec_p, last_carbon));
      Array_2d8_Put (&sorted_parity, Array_1d16_Get (&sling_pos, atom), 1,
        Array_1d1_Get (asymmetric_p, last_carbon));
      last_carbon++;

      if (Array_1d8_Get (parityvec_p, atom) == PARITY_TRIODD)
        {
        for (neigh = 0; neigh < MX_NEIGHBORS && Tsd_Atom_NeighborBond_Get (
             tsd_tmp, atom, neigh) != BOND_DOUBLE; neigh++)
          /* Empty loop body */ ;

        Array_1d8_Put (parityvec_p, Array_1d16_Get (&parityvec_pos,
          Tsd_Atom_NeighborId_Get (tsd_tmp, atom, neigh)), PARITY_INVALID2);
        }
      }
    }                         /* End of for-atom loop */

  for (atom = 0, last_carbon = 0; atom < num_atoms; atom++)
    {
    /* The atom has a parity which should be added to the parity bit string */

    if (atom >= num_carbons)
      Array_1d1_Put (asymmetric_p, atom, FALSE);

    if (Array_2d8_Get (&sorted_parity, atom, 0) != PARITY_INVALID1)
      {
      if ((S8_t)Array_2d8_Get (&sorted_parity, atom, 0) <= 0)
        Array_1d1_Put (paritybits_p, last_carbon, TRUE);

      if (Array_2d8_Get (&sorted_parity, atom, 0) == PARITY_DONTCARE)
        Tsd_AtomFlags_DontCare_Put (tsd_p, atom, TRUE);

      last_carbon++;
      }
    }

  *ce_str_p = String_Create (NULL, 3 * num_atoms + 8);
  *se_str_p = String_Create (NULL, 3 * num_atoms + 8);

  SBrothersXlate (cebros_p, &sling_pos, ce_str_p, num_bros);
  if (!String_Compare_c (*ce_str_p, ""))
    String_Concat_c (ce_str_p, NOCEBS);

  SBrothersXlate (sebros_p, &sling_pos, se_str_p, num_bros);
  if (!String_Compare_c (*se_str_p, ""))
    String_Concat_c (se_str_p, NOSEBS);

  for (atom = 0; atom < num_carbons; atom++)
    if (Array_1d8_Get (parityvec_p, atom) == PARITY_UNSURE)
      Array_1d8_Put (parityvec_p, atom, PARITY_TRIODD);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&in_stack", "sling", &in_stack);
  mind_Array_Destroy ("&parity_ok", "sling", &parity_ok);
  mind_Array_Destroy ("&sling_pos", "sling", &sling_pos);
  mind_Array_Destroy ("&bond_mult", "sling", &bond_mult);
  mind_Array_Destroy ("&parityvec_pos", "sling", &parityvec_pos);
  mind_Array_Destroy ("&sorted_parity", "sling", &sorted_parity);
  mind_Array_Destroy ("&dontcare", "sling", &dontcare);
#else
  Array_Destroy (&in_stack);
  Array_Destroy (&parity_ok);
  Array_Destroy (&sling_pos);
  Array_Destroy (&bond_mult);
  Array_Destroy (&parityvec_pos);
  Array_Destroy (&sorted_parity);
  Array_Destroy (&dontcare);
#endif
  Stack_Destroy (stack_p);
  Tsd_Destroy (tsd_tmp);

  TRACE_DO (DB_TSD2SLINGX, TL_PARAMS,
    {
    IO_Put_Trace (R_TSD, "Output parameters from Tsd2SlingX");
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "Sling (as String) => %s\n", String_Value_Get (output));
    IO_Put_Trace (R_TSD, "Parity bits");
    Array_Dump (paritybits_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "Asymmetric flags");
    Array_Dump (asymmetric_p, &GTraceFile);
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "CE Brother String => %s\n", String_Value_Get (*ce_str_p));
    fprintf (IO_FileHandle_Get (&GTraceFile),
      "SE Brother String => %s\n", String_Value_Get (*se_str_p));
    });

  sling = String2Sling (output);
  String_Destroy (output);

  DEBUG (R_TSD, DB_TSD2SLINGX, TL_PARAMS, (outbuf,
    "Leaving Tsd2SlingX, sling not shown"));

  return sling;
}

/****************************************************************************
*
*  Function Name:                 SAsiDump
*
*    This routine prints a formatted dump of an ASI plus TSD index.
*
*  Used to be:
*
*    print_asi:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SAsiDump
  (
  Array_t      *asi_p,                     /* ASI to dump */
  Array_t      *tsdindex_p,                /* Associated index */
  Array_t      *unique_p                   /* Uniqueness bits */
  )
{
  FILE         *f;                         /* Temporary */
  char         *boolstr[2];                /* Cute hack */
  U32_t         short_atoms;               /* Temporary */
  U32_t         atom;                      /* Counter */

  boolstr[FALSE] = "False";
  boolstr[TRUE] = "True";

  f = IO_FileHandle_Get (&GTraceFile);
  short_atoms = Array_Columns_Get (asi_p);

  fprintf (f, "ASI's by node number, sorted by ASI#\n");
  fprintf (f, "node#\tASI\tunique\tASI\tnode#\n");

  for (atom = 0; atom < short_atoms; atom++)
    {
    fprintf (f, "  %u\t%u\t%s\t%u\t%u\n", atom, Array_1d16_Get (asi_p, atom),
      boolstr[Array_1d1_Get (unique_p, atom)], Array_1d16_Get (asi_p,
      Array_1d16_Get (tsdindex_p, atom)), Array_1d16_Get (tsdindex_p, atom));
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SAtomLocate
*
*    This function returns the Canonical Atom Index instead of the one from
*    the periodic table as in ATOMSYM.
*
*  Used to be:
*
*    locatmn:
*
*  Implicit Inputs:
*
*    SAtomsymbols
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Index into canonical atoms table
*
*  Side Effects:
*
*
*
******************************************************************************/
static U16_t SAtomLocate
  (
  const char   *sym_p                      /* Symbol */
  )
{
  U16_t         i;                         /* Counter */

  for (i = 0; i <= MX_CANON_ATOM; i++)
     if (strcmp (SAtomsymbols[i], sym_p) == 0)
       return i;

  return INFINITY;
}

/****************************************************************************
*
*  Function Name:                 SAtomRead
*
*    This function obtains from the Sling the atom identifier (atomic number)
*    of the atom at the current position in the Sling, and it returns the 
*    new offset into the Sling.
*
*  Used to be:
*
*    read_atom:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Address of character beyond atom just processed
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U8_t *SAtomRead
  (
  U8_t         *symstart_p,                /* Start of symbol to xlate */
  Tsd_t        *tsd_p,                     /* Molecule to update */
  U16_t         nextid                     /* Next atom index in molecule */
  )
{
  U8_t         *cp;                        /* For string munging */
  U8_t          tbuf[8];                   /* For conversion */
  U8_t          i;                         /* Counter */

  if (*symstart_p == GROUP_START_SYM)
    {
    for (cp = symstart_p + 1, i = 0; *cp != GROUP_END_SYM; cp++)
      tbuf[i++] = *cp;

    tbuf[i] = '\0';
    cp++;
    }
  else
    {
    tbuf[0] = *symstart_p;
    tbuf[1] = '\0';
    cp = symstart_p + 1;
    }

    Tsd_Atomid_Put (tsd_p, nextid, Atomsymbol2Id (tbuf));

    return cp;
} 

/****************************************************************************
*
*  Function Name:                 SBreak
*
*    This routine breaks a tie between atoms in a TSD in favor of a particular
*    atom.
*
*  Used to be:
*
*    break:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    May call IO_Exit_Error
*
******************************************************************************/
static void SBreak
  (
  Array_t     *asi_p,                       /* ASI to putz with */
  Array_t     *tsdindex_p,                  /* Associated TSD index perm. */
  U16_t        atom                         /* Which atom we are using */
  )
{
  U16_t        short_atoms;                 /* # atoms to consider */
  U16_t        index1, index2;              /* Temporaries */
  U16_t        index;                       /* Temporary */
  U16_t        i;                           /* Counter */
  U16_t        first;                       /* Which atom index is tied */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, 
    "Entering SBreak, asi = %p, tsdindex = %p, atom = %u", asi_p, tsdindex_p, 
    atom));

  short_atoms = (U16_t) Array_Columns_Get (asi_p);
  index2 = Array_1d16_Get (tsdindex_p, atom);
  index1 = Array_1d16_Get (asi_p, index2);

  for (i = short_atoms - 1, first = short_atoms + 1; (S16_t)i >= 0; i--)
    {
    index = Array_1d16_Get (tsdindex_p, i);
    if (index != index2)
      if (Array_1d16_Get (asi_p, index) == index1)
        {
        Array_1d16_Put (asi_p, index, index1 + 1);
        first = i;
        }
    }

  if (first > short_atoms)
    IO_Exit_Error (R_SLING, X_SYNERR, "SBreak called incorrectly");

  if (first < atom)
    {
    Array_1d16_Put (tsdindex_p, atom, Array_1d16_Get (tsdindex_p, first));
    Array_1d16_Put (tsdindex_p, first, index2);
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SBreak, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SBrothersXlate
*
*    This routine translates the equivilance classes, CE and SE, from array
*    format into String format.
*
*  Used to be:
*
*    trans_bros:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SBrothersXlate
  (
  Array_t      *class_p,                   /* Array of class equivilances */
  Array_t      *sling_pos_p,               /* Offset into Sling for atom */
  String_t     *class_str_p,               /* Resulting string format */
  U16_t         class_cnt                  /* Size of array ??? */
  )
{
  S16_t         equiv;                     /* Class to search for */
  U16_t         i, j;                      /* Counters */
  Boolean_t     found;                     /* Flag for each class number */
  char          tbuf[8];                   /* Conversion buffer */

  DEBUG (R_TSD, DB_TSDSTATIC, TL_PARAMS, (outbuf, "Entering SBrothersXlate,"
    " class = %p, sling pos = %p, class string = %p, class size = %u", 
    class_p, sling_pos_p, class_str_p, class_cnt));

  for (i = 0; i < class_cnt; i++)
    if ((S16_t)Array_1d16_Get (class_p, i) >= 0)
      {
      equiv = Array_1d16_Get (class_p, i);
      found = FALSE;

      for (j = i + 1; j < class_cnt; j++)
        if ((S16_t)Array_1d16_Get (class_p, j) == equiv)
          {
          Number2Char (Array_1d16_Get (sling_pos_p, j), tbuf);
          String_Concat_c (class_str_p, tbuf);
          String_Concat_c (class_str_p, CLASSMEMBER_SEP);
          found = TRUE;
          Array_1d16_Put (class_p, j, (-equiv - 1));
          }

      if (found)
        {
        Number2Char (Array_1d16_Get (sling_pos_p, i), tbuf);
        String_Concat_c (class_str_p, tbuf);
        String_Concat_c (class_str_p, EQUIVCLASS_SEP);
        }
      }                    /* End of if-equiv is negative */

  for (i = 0; i < class_cnt; i++)
    if ((S16_t)Array_1d16_Get (class_p, i) < 0)
      {
      equiv = Array_1d16_Get (class_p, i);
      Array_1d16_Put (class_p, i, -(equiv + 1));
      }

  DEBUG (R_TSD, DB_TSDSTATIC, TL_PARAMS, (outbuf, "Leaving SBrothersXlate"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SChiralityCheck
*
*    This routine checks to see if an atom is Chiral.  The chemistry is not
*    currently understood.
*
*  Used to be:
*
*    check_chirality:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SChiralityCheck
  (
  Boolean_t   *chiral_p,                    /* Flag for chirality */
  CodedTsd_t  *curcoded_p,                  /* Current coded TSD */
  Array_t     *newparity_p,                 /* 1-d byte, parity indicators */
  U16_t        num_carbons                  /* # carbon atoms in molecule */
  )
{
  AtomArray_t *parity_p;                    /* Coded TSD parity */
  U16_t        atom;                        /* Counter */

  for (parity_p = CodeTsd_Parity_Get (curcoded_p); parity_p != NULL;
       parity_p = AtmArr_Next_Get (parity_p))
    {
    for (atom = 0, *chiral_p = FALSE; atom < num_carbons && !(*chiral_p);
         atom++)
      {
      /* Tetravalent atom check */

      if (abs((S8_t)Array_1d8_Get (newparity_p, atom)) == 1)
        {
        if (Array_1d8_Get (newparity_p, atom) == Array_1d8_Get (
            AtmArr_Array_Get (parity_p), atom))
          *chiral_p = TRUE;
        }
      else
        /* Trivalent atom check */

        if (Array_1d8_Get (newparity_p, atom) != Array_1d8_Get (
            AtmArr_Array_Get (parity_p), atom))
          *chiral_p = TRUE;
      }

    if (!(*chiral_p))
      return;
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SCodedTsdEqual
*
*    This function checks to see if two Coded TSDs are equal.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    TRUE - base is equal to comp
*    FALSE - base is NOT equal to comp
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SCodedTsdEqual
  (
  CodedTsd_t   *base_p,                     /* Coded TSD to compare vs */
  CodedTsd_t   *comp_p                      /* Coded TSD to see if is lesser */
  )
{
  Tsd_t        *basetsd_p;                  /* TSD of base */
  Tsd_t        *comptsd_p;                  /* TSD of comparator */
  Array_t      *baseweight_p;               /* 1-d word, bond weights */
  Array_t      *compweight_p;               /* 1-d word, bond weights */
  U16_t         atom;                       /* Counter */
  U8_t          neigh;                      /* Counter */

  basetsd_p = CodeTsd_Tsd_Get (base_p);
  comptsd_p = CodeTsd_Tsd_Get (comp_p);
  baseweight_p = CodeTsd_BondWeight_Get (base_p);
  compweight_p = CodeTsd_BondWeight_Get (comp_p);

  if (Tsd_NumAtoms_Get (basetsd_p) != Tsd_NumAtoms_Get (comptsd_p) ||
      CodeTsd_LastNonH_Get (base_p) != CodeTsd_LastNonH_Get (comp_p))
    return FALSE;

  for (atom = 0; atom < CodeTsd_LastNonH_Get (base_p); atom++)
    {
    if (Tsd_Atomid_Get (basetsd_p, atom) != Tsd_Atomid_Get (comptsd_p, atom) ||
        Array_1d16_Get (baseweight_p, atom) != Array_1d16_Get (compweight_p,
        atom))
      return FALSE;

    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      if (Tsd_Atom_NeighborId_Get (basetsd_p, atom, neigh) !=
          Tsd_Atom_NeighborId_Get (comptsd_p, atom, neigh) ||
          Tsd_Atom_NeighborBond_Get (comptsd_p, atom, neigh) !=
          Tsd_Atom_NeighborBond_Get (comptsd_p, atom, neigh))
        return FALSE;
    }

  return TRUE;
}

/****************************************************************************
*
*  Function Name:                 SCodedTsdLessThan
*
*    This function checks to see how two Coded TSDs are ordered by the 
*    less-than function.
*
*  Used to be:
*
*    N/A:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    TRUE - base is less than or equal to comp
*    FALSE - base is strictly greater than comp
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SCodedTsdLessThan
  (
  CodedTsd_t   *base_p,                     /* Coded TSD to compare vs */
  CodedTsd_t   *comp_p                      /* Coded TSD to see if is lesser */
  )
{
  Tsd_t        *basetsd_p;                  /* TSD of base */
  Tsd_t        *comptsd_p;                  /* TSD of comparator */
  Array_t      *baseweight_p;               /* 1-d word, bond weights */
  Array_t      *compweight_p;               /* 1-d word, bond weights */
  U16_t         atom;                       /* Counter */
  U8_t          neigh;                      /* Counter */

  basetsd_p = CodeTsd_Tsd_Get (base_p);
  comptsd_p = CodeTsd_Tsd_Get (comp_p);
  baseweight_p = CodeTsd_BondWeight_Get (base_p);
  compweight_p = CodeTsd_BondWeight_Get (comp_p);

  if (Tsd_NumAtoms_Get (basetsd_p) > Tsd_NumAtoms_Get (comptsd_p) ||
      CodeTsd_LastNonH_Get (base_p) > CodeTsd_LastNonH_Get (comp_p))
    return FALSE;

  for (atom = 0; atom < CodeTsd_LastNonH_Get (base_p); atom++)
    {
    if (Tsd_Atomid_Get (basetsd_p, atom) > Tsd_Atomid_Get (comptsd_p, atom) ||
        Array_1d16_Get (baseweight_p, atom) > Array_1d16_Get (compweight_p,
        atom))
      return FALSE;

    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      if (Tsd_Atom_NeighborId_Get (basetsd_p, atom, neigh) >
          Tsd_Atom_NeighborId_Get (comptsd_p, atom, neigh) ||
          Tsd_Atom_NeighborBond_Get (comptsd_p, atom, neigh) >
          Tsd_Atom_NeighborBond_Get (comptsd_p, atom, neigh))
        return FALSE;
    }

  return TRUE;
}

/****************************************************************************
*
*  Function Name:                 SCompact
*
*    This routine compacts a TSD.  Since in the search for a canonical name
*    the only interesting atoms are those which can be reached in more than
*    one way, this routine eliminates singly connected atoms by changing 
*    their index in their neighbor to atomid + a constant.  This can now be
*    unfolded later.  A mapping array is generated that gives the long atom
*    index as the short atom or TSD_INVALID if it was compacted away.
*
*  Used to be:
*
*    compact:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Address of compacted TSD
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Tsd_t *SCompact
  (
  Tsd_t        *longtsd_p,                  /* TSD to compact */
  Array_t      *map_p,                      /* 1-d word, long to compact map */
  Boolean_t     compaction                  /* Compaction flag */
  )
{
  Tsd_t        *tsd_p;                      /* Temporary */
  U16_t         mult_atoms;                 /* Number of non-single connected
    atoms */
  U16_t         long_atoms;                 /* # atoms in TSD */
  U16_t         short_atoms;                /* Short atom index */
  U16_t         atom;                       /* Counter */
  U16_t         connection;                 /* Counter */
  U8_t          neigh;                      /* Counter */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, 
    "Entering SCompact, tsd = %p, map = %p, compaction flag = %u", 
    longtsd_p, map_p, compaction));

  long_atoms = Tsd_NumAtoms_Get (longtsd_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("map_p", "sling{8}", map_p, long_atoms, WORDSIZE);
#else
  Array_1d_Create (map_p, long_atoms, WORDSIZE);
#endif
  Array_Set (map_p, TSD_INVALID);

  /* Count the atoms with multiple connections */

  for (atom = 0, mult_atoms = 0; atom < long_atoms; atom++)
    {
    for (neigh = 0, connection = 0; neigh < MX_NEIGHBORS; neigh++)
      if (Tsd_Atom_NeighborId_Get (longtsd_p, atom, neigh) != TSD_INVALID)
        connection++;

    if (connection > 1)
      {
      Array_1d16_Put (map_p, atom, mult_atoms);
      mult_atoms++;
      }
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf,
    "In SCompact, # short atoms = %u", mult_atoms));

  if ((mult_atoms == long_atoms) || (mult_atoms == 0) || (compaction == FALSE))
    {
    tsd_p = Tsd_Copy (longtsd_p);

    for (atom = 0; atom < long_atoms; atom++)
      Array_1d16_Put (map_p, atom, atom);

    DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
      "Leaving SCompact, tsd = %p", tsd_p));

    return tsd_p;
    }

  tsd_p = Tsd_Create (mult_atoms);

  for (atom = 0, short_atoms = 0; atom < long_atoms; atom++)
    {
    if (Array_1d16_Get (map_p, atom) != TSD_INVALID)
      {
      Tsd_Atomid_Put (tsd_p, short_atoms, Tsd_Atomid_Get (longtsd_p, atom));
      Tsd_Atom_Flags_Put (tsd_p, short_atoms, Tsd_Atom_Flags_Get (longtsd_p,
        atom));

      for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
        {
        Tsd_Atom_NeighborBond_Put (tsd_p, short_atoms, neigh,
          Tsd_Atom_NeighborBond_Get (longtsd_p, atom, neigh));

        connection = Tsd_Atom_NeighborId_Get (longtsd_p, atom, neigh);
        if ((connection > long_atoms) || (connection == TSD_INVALID))
          {
          Tsd_Atom_NeighborId_Put (tsd_p, short_atoms, neigh, connection);
          }
        else
          {
          if (Array_1d16_Get (map_p, connection) != TSD_INVALID)
            {
            Tsd_Atom_NeighborId_Put (tsd_p, short_atoms, neigh,
              Array_1d16_Get (map_p, connection));
            }
          else
            Tsd_Atom_NeighborId_Put (tsd_p, short_atoms, neigh,
              Tsd_Atomid_Get (longtsd_p, connection) + CANON_SHORT_DIFF);
          }
        }

      short_atoms++;
      }
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SCompact, tsd = %p", tsd_p));

  return tsd_p;
}

/****************************************************************************
*
*  Function Name:                 SCompute
*
*    This routine computes a new TSD and bondweight array from the original
*    TSD and the current ASI, TSD index and TSD.
*
*  Used to be:
*
*    compute:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SCompute
  (
  Array_t      *asi_p,                      /* 1-d word, ASI */
  Array_t      *tsdindex_p,                 /* 1-d word, TSD mapping */
  Tsd_t        *shorttsd_p,                 /* Original compact TSD */
  Tsd_t        *newtsd_p,                   /* To be computed TSD */
  Array_t      *newparity_p,                /* 1-d byte, parity indicators */
  Tsd_t        *tsd_p,                      /* Current TSD */
  Array_t      *bondweight_p,               /* Current bond weights */
  U16_t         num_carbons                 /* # carbons in molecule */
  )
{
  U16_t         j, k;                       /* Counters */
  U16_t         atom;                       /* Counter */
  U16_t         atomid;                     /* Atomic number */
  U16_t         atomid2;                    /* Another atomic number */
  U16_t         neighid;                    /* Atomic number */
  U16_t         index;                      /* Temporary */
  U16_t         short_atoms;                /* # atoms in molecule */
  U16_t         exchanges;                  /* Number of swap ops - sort */
  U8_t          neigh;                      /* Counter */
  Boolean_t     done;                       /* Flag for sorting */
  U16_t         vector[MX_NEIGHBORS];       /* For sorting */
  Array_t       uniqlist;                   /* 2-d word, atom ids */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Entering SCompute,"
   " asi = %p, tsdindex = %p, input tsd = %p, output tsd = %p, output parity"
   " = %p, current tsd = %p, current bond weight = %p, # carbon atoms = %u",
   asi_p, tsdindex_p, shorttsd_p, newtsd_p, newparity_p, tsd_p, bondweight_p,
   num_carbons));

  short_atoms = Tsd_NumAtoms_Get (shorttsd_p);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&uniqlist", "sling{9}", &uniqlist, short_atoms, MX_NEIGHBORS, WORDSIZE);
#else
  Array_2d_Create (&uniqlist, short_atoms, MX_NEIGHBORS, WORDSIZE);
#endif
  Array_Set (&uniqlist, TSD_INVALID);

  TRACE_DO (DB_SLINGSTATIC, TL_PARAMS,
    {
/*    SAsiDump (asi_p, tsdindex_p, unique_p);  unique_p not available */
    });

  for (atom = 0; atom < short_atoms; atom++)
    {
    index = Array_1d16_Get (tsdindex_p, atom);
    if (atom != Array_1d16_Get (asi_p, index))
      IO_Put_Debug (R_SLING, "Error in SCompute, ASI not correctly formed");

    Tsd_Atomid_Put (newtsd_p, atom, Tsd_Atomid_Get (tsd_p, index));
    Tsd_AtomFlags_DontCare_Put (newtsd_p, atom, Tsd_AtomFlags_DontCare_Get (
      tsd_p, index));
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      if (Tsd_Atom_NeighborId_Get (shorttsd_p, index, neigh) != TSD_INVALID)
        {
        if (Tsd_Atom_NeighborId_Get (shorttsd_p, index, neigh) >
            CANON_SHORT_DIFF)
          {
          Array_2d16_Put (&uniqlist, atom, neigh, Tsd_Atom_NeighborId_Get (
            shorttsd_p, index, neigh));
          }
        else
          {
          for (k = 0; k < short_atoms; k++)
            if (Array_1d16_Get (tsdindex_p, k) == Tsd_Atom_NeighborId_Get (
                shorttsd_p, index, neigh))
              Array_2d16_Put (&uniqlist, atom, neigh, k);
          }
        }

      Tsd_Atom_NeighborBond_Put (newtsd_p, atom, neigh,
        Tsd_Atom_NeighborBond_Get (shorttsd_p, index, neigh));
      }

    for (j = 0; j < MX_NEIGHBORS - 1; j++)
      for (k = j + 1; k < MX_NEIGHBORS; k++)
        if ((S16_t)Array_2d16_Get (&uniqlist, atom, k) >
            (S16_t)Array_2d16_Get (&uniqlist, atom, j))
          {
          SSwap (Array_2d16_Addr (&uniqlist, atom, k), Array_2d16_Addr (
            &uniqlist, atom, j));

          if ((S16_t)Array_2d16_Get (&uniqlist, atom, k) >
              (S16_t)Array_2d16_Get (&uniqlist, atom, j))
            IO_Put_Debug (R_SLING, "SCompute: swap using dummy");

          STsdSwap (newtsd_p, atom, k, j);
          }

    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      Tsd_Atom_NeighborId_Put (newtsd_p, atom, neigh, Array_2d16_Get (
        &uniqlist, atom, neigh));
    }

  TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_SLING, "Munged TSD:");
    STsdDump (newtsd_p, &GTraceFile);
    });

  /* Assign the parities.  for a tetrahedral carbon, if sorting the nwv takes
     an even # of exchanges, the parity is +1, else it is -1.  for two
     trihedral, doubly bonded carbons, if sorting the two nwv's takes even
     exchanges, the parity is +2, else it is -2 for both carbons.
  */

  if (!num_carbons)
    {
    DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf,
      "No parity computed -- no carbon atoms"));

#ifdef _MIND_MEM_
    mind_Array_Destroy ("&uniqlist", "sling", &uniqlist);
#else
    Array_Destroy (&uniqlist);
#endif

    DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
      "Leaving SCompute, status = <void>"));

    return;
    }

  Array_Set (newparity_p, PARITY_INIT);

  for (atom = 0; atom < short_atoms; atom++)
    {
    index = Array_1d16_Get (tsdindex_p, atom);
    atomid = Tsd_Atomid_Get (tsd_p, index) / CANON_ATOMID_PERM;
    if (atomid == CANON_CARBON)
      {
      if (Array_1d16_Get (bondweight_p, index) == CANON_TETRAVALENT)
        {
        if (Tsd_Atom_NeighborId_Get (tsd_p, index, BOND_DIR_RIGHT) != 
            TSD_INVALID && Tsd_Atom_NeighborId_Get (tsd_p, index,
            BOND_DIR_IN) == TSD_INVALID)
          {
          if (Tsd_AtomFlags_DontCare_Get (shorttsd_p, index))
            {
            Array_1d8_Put (newparity_p, atom, PARITY_DONTCARE);
            }
          else
            {
            /* Copy tsd row into vector for sorting */

            memset (vector, 0, sizeof (vector));

            SRowCopy (0, 3, vector, index, 0, tsd_p, asi_p);
            done = FALSE;
            exchanges = SExchangesCount (0, 2, 3, vector, &done);

            if (!done)
              {
              if ((exchanges % 2) == 0)
                {
                Array_1d8_Put (newparity_p, atom, PARITY_TETRAEVEN);
                }
              else
                Array_1d8_Put (newparity_p, atom, PARITY_TETRAODD);
              }
            }
          }
        }            /* End If-Tetravalent */

      if (Array_1d16_Get (bondweight_p, index) == CANON_TRIVALENT &&
          Array_1d8_Get (newparity_p, atom) == PARITY_INIT)
        {
        if (Tsd_Atom_NeighborId_Get (tsd_p, index, BOND_DIR_LEFT) !=
            TSD_INVALID && Tsd_Atom_NeighborId_Get (tsd_p, index,
            BOND_DIR_RIGHT) == TSD_INVALID)
          {
          memset (vector, 0, sizeof (vector));

          SRowCopy (0, 2, vector, index, 0, tsd_p, asi_p);
          neighid = Tsd_Atom_NeighborId_Get (tsd_p, index, BOND_DIR_LEFT);
          for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
            if (Tsd_Atom_NeighborBond_Get (shorttsd_p, index, neigh) ==
                BOND_DOUBLE)
              neighid = Tsd_Atom_NeighborId_Get (shorttsd_p, index, neigh);

          if (neighid != TSD_INVALID && neighid < short_atoms)
            {
            atomid2 = Tsd_Atomid_Get (tsd_p, neighid) / CANON_ATOMID_PERM;
            if (atomid2 == CANON_CARBON)
              if (Tsd_Atom_NeighborId_Get (tsd_p, neighid, BOND_DIR_LEFT)
                  != TSD_INVALID && Tsd_Atom_NeighborId_Get (tsd_p, neighid,
                  BOND_DIR_RIGHT) == TSD_INVALID)
                {
                if (Tsd_AtomFlags_DontCare_Get (shorttsd_p, index))
                  {
                  Array_1d8_Put (newparity_p, atom, PARITY_DONTCARE);
                  Array_1d8_Put (newparity_p, Array_1d16_Get (asi_p,
                    neighid), PARITY_DONTCARE);
                  }
                else
                  {
                  SRowCopy (0, 2, vector, neighid, 3, tsd_p, asi_p);
                  done = FALSE;
                  exchanges = SExchangesCount (0, 1, 2, vector, &done) +
                    SExchangesCount (3, 4, 5, vector, &done);
                  if (!done)
                    {
                    if ((exchanges % 2) == 0)
                      {
                      Array_1d8_Put (newparity_p, atom, PARITY_TRIEVEN);
                      Array_1d8_Put (newparity_p, Array_1d16_Get (asi_p, 
                        neighid), PARITY_TRIEVEN);
                      }
                    else
                      {
                      Array_1d8_Put (newparity_p, atom, PARITY_TRIODD);
                      Array_1d8_Put (newparity_p, Array_1d16_Get (asi_p, 
                        neighid), PARITY_TRIODD);
                      }
                    }
                  }          /* End Else- don't care */
                }            /* End If-bond[left] && bond[right] (neighid) */
            }                /* End If neighid < short_atoms */
          }                  /* End If-bond[left] && bond[right] (index) */
        }                    /* End If-Trivalent */
      }                      /* End If-atomid == Carbon */
    }                        /* End for-atom loop */

  TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_SLING, "New parity is:");
    Array_Dump (newparity_p, &GTraceFile);
    });

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&uniqlist", "sling", &uniqlist);
#else
  Array_Destroy (&uniqlist);
#endif

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SCompute, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SDifferentiator
*
*    This routine differentiates among several possible ways to try for the
*    next "smallest" molecule.  It can tell if the molecule is unique, ie
*    the smallest when it is done.
*
*    - asi contains the asi vector obtained by neighborhood tie breaking
*      based on the asi passed as input
*
*  Used to be:
*
*    differentiator:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SDifferentiator
  (
  Array_t      *asi_p,                      /* 1-d word, ASI */
  Array_t      *tsdindex_p,                 /* 1-d word, TSD index */
  Boolean_t    *all_unique_p,               /* Are all atoms unique */
  Array_t      *unique_p,                   /* 1-d bit, uniqueness flags */
  Tsd_t        *shorttsd_p,                 /* Original compact TSD */
  Array_t      *oldasi_p,                   /* 1-d word, old / original ASI */
  Array_t      *neighweight_p               /* 2-d word, neighbor weights */
  )
{
  U16_t         short_atoms;                /* # atoms in TSD */
  U16_t         atom;                       /* Counter */
  U16_t         index;                      /* Temporary */
  U16_t         tie_broken;                 /* Atom index where tie is broken
    */
  U16_t         neighid;                    /* Atom index of neighbor */
  U16_t         weight;                     /* Weight of neighbor */
  U16_t         index1, index2;             /* Temporary */
  S16_t         i, j, k;                    /* Counters */
  U16_t         sum;                        /* For calculating all unique */
  U8_t          neigh;                      /* Counter */
  Boolean_t     done;                       /* Flag for doneness */
  Boolean_t     asi_changed;                /* Flag to tell if we changed */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Entering\
 SDifferentiator, asi = %p, tsdindex = %p, unique = %p, tsd = %p, oldasi = %p,\
 neigh weight = %p", asi_p, tsdindex_p, unique_p, shorttsd_p, oldasi_p,
 neighweight_p));

  SUniqueCompare (asi_p, tsdindex_p, unique_p);
  short_atoms = Tsd_NumAtoms_Get (shorttsd_p);

  for (atom = 0, sum = 0; atom < short_atoms; atom++)
    sum += Array_1d1_Get (unique_p, atom) ? 1 : 0;

  if (sum < short_atoms)
    *all_unique_p = FALSE;
  else
    {
    *all_unique_p = TRUE;

    DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Leaving\
 SDifferentiator, all nodes are known to be unique, no work done,\
 status = <void>"));

    return;
    }

  asi_changed = TRUE;
  while (asi_changed)
    {
    Array_CopyContents (asi_p, oldasi_p);

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING, "Main SDifferentiator loop, ASI:");
      SAsiDump (asi_p, tsdindex_p, unique_p);
      });

    Array_Set (neighweight_p, CANON_NEIGHWEIGHT_INIT);

    for (atom = 0; atom < short_atoms; atom++)
      {
      if (!Array_1d1_Get (unique_p, atom))
        {
        for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
          {
          neighid = Tsd_Atom_NeighborId_Get (shorttsd_p, atom, neigh);
          if (neighid != TSD_INVALID)
            {
            if (neighid > CANON_SHORT_DIFF)
              weight = neighid;
            else
              weight = Array_1d16_Get (asi_p, neighid);

            for (k = 0, done = FALSE; k < MX_NEIGHBORS && !done; k++)
              if (Array_2d16_Get (neighweight_p, atom, k) > weight)
                {
                /* Used to be a loop here, but looks like it was only executed
                   once through.  It might need to be added back to execute
                   bond multiplicity times
                */

                for (i = MX_NEIGHBORS - 2; i >= k; i--)
                  Array_2d16_Put (neighweight_p, atom, i + 1,
                    Array_2d16_Get (neighweight_p, atom, i));

                Array_2d16_Put (neighweight_p, atom, k, weight);

                /* End of loop was here */

                done = TRUE;
                }
            }             /* End if-neighid != TSD_INVALID */
          }               /* End for-neigh loop */
        }                 /* End if-!unique */
      }                   /* End for-atom loop */

    /* Sort the neighweight vector's and assign asi's.  Set 'unique' bits
       and compute 'all_unique'.
    */

    for (atom = 1; atom < short_atoms; atom++)
      {
      index = Array_1d16_Get (tsdindex_p, atom);
      if (!Array_1d1_Get (unique_p, index))
        {
        j = atom - 1;
        if (Array_1d16_Get (asi_p, index) == Array_1d16_Get (asi_p,
            Array_1d16_Get (tsdindex_p, j)))
          {
          /* Find first on list below atom with a different asi */

          for ( ; j > 0 && Array_1d16_Get (asi_p, index) == Array_1d16_Get (
               asi_p, Array_1d16_Get (tsdindex_p, j - 1)); j--)
            /* Empty loop body */ ;

          for (k = 0; atom > (U16_t)j &&
               Array_2d16_Get (neighweight_p, index, k) >=
               Array_2d16_Get (neighweight_p, Array_1d16_Get (tsdindex_p, j),
               k); k++)
            if (k == MX_NEIGHBORS - 1 || Array_2d16_Get (neighweight_p, index,
                k) > Array_2d16_Get (neighweight_p, Array_1d16_Get (tsdindex_p,
                j), k))
              {
              j++;
              k = -1;
              }

          if ((U16_t)j < atom && Array_2d16_Get (neighweight_p, index, k) <
              Array_2d16_Get (neighweight_p, Array_1d16_Get (tsdindex_p, j),
              k))
            {
            /* Shift everything from i to j up by one and put i into j */

            for (k = atom - 1; k >= j; k--)
              Array_1d16_Put (tsdindex_p, k + 1, Array_1d16_Get (tsdindex_p,
                k));

            Array_1d16_Put (tsdindex_p, j, index);

            TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
              {
              IO_Put_Trace (R_SLING, "TSD index after sort:");
              Array_Dump (tsdindex_p, &GTraceFile);
              });

            }
          }                 /* End if-asi[index] == asi[tsdindex[j]] */
        }                   /* End if-!unique */
      }                     /* End for-atom loop */

    /* Assign the asi's and form all_unique */

    for (atom = 1; atom < short_atoms; atom++)
      {
      index1 = Array_1d16_Get (tsdindex_p, atom);
      index2 = Array_1d16_Get (tsdindex_p, atom - 1);
      if (Array_1d16_Get (oldasi_p, index1) == Array_1d16_Get (oldasi_p,
          index2))
        {
        for (neigh = 0, tie_broken = FALSE; neigh < MX_NEIGHBORS &&
             !tie_broken; neigh++)
          if (Array_2d16_Get (neighweight_p, index1, neigh) > Array_2d16_Get (
              neighweight_p, index2, neigh))
            {
            Array_1d16_Put (asi_p, index1, atom);
            tie_broken = TRUE;
            }

        if (!tie_broken)
          Array_1d16_Put (asi_p, index1, Array_1d16_Get (asi_p, index2));
        }
      }

    SUniqueCompare (asi_p, tsdindex_p, unique_p);

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING, "After call to SUniqueCompare (SDifferentiator):");
      SAsiDump (asi_p, tsdindex_p, unique_p);
      });

    for (atom = 0, sum = 0; atom < short_atoms; atom++)
      sum += Array_1d1_Get (unique_p, atom) ? 1 : 0;

    if (sum < short_atoms)
      *all_unique_p = FALSE;
    else
      {
      *all_unique_p = TRUE;

      TRACE_DO (DB_SLINGSTATIC, TL_PARAMS, 
        {
        SAsiDump (asi_p, tsdindex_p, unique_p);
        IO_Put_Trace (R_SLING, "Leaving SDifferentiator, all nodes have unique\
 ASIs, status = <void>");
        });

      return;
      }

    for (atom = 0, asi_changed = FALSE; atom < short_atoms && !asi_changed;
         atom++)
      {
      index = Array_1d16_Get (tsdindex_p, atom);
      asi_changed = (Array_1d16_Get (oldasi_p, index) != Array_1d16_Get (asi_p,
        index));
      }
    }                 /* End while-!asi_changed */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SDifferentiator, new ASI is same as last one"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SExchangesCount
*
*    This function counts how many times the array needs to swap elements
*    during a sort.
*
*  Used to be:
*
*    counexchanges_t:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    # times a swap occured
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U16_t SExchangesCount
  (
  U16_t         start,                      /* Start index */
  U16_t         end,                        /* End index */
  U16_t         max,                        /* One beyond end */
  U16_t        *vector_p,                   /* Vector to "exchange" */
  Boolean_t    *done_p                      /* Done flag */
  )
{
  U16_t         num_xchanges;               /* Counter */
  U16_t         i, j;                       /* Counter */

  if (*done_p)
    return 0;

  for (i = start, num_xchanges = 0; i <= end; i++)
    {
    for (j = i + 1; j <= max; j++)
      {
      if (vector_p[i] == vector_p[j])
        {
        *done_p = TRUE;
        return num_xchanges;
        }

      if (vector_p[i] > vector_p[j])
        {
        SSwap (&vector_p[i], &vector_p[j]);
        if (vector_p[i] > vector_p[j])
          IO_Put_Debug (R_SLING, "SExchangesCount, swap using dummy");

        num_xchanges++;
        }
      }
    }

  return num_xchanges;
}

/****************************************************************************
*
*  Function Name:                 SExpand
*
*    This function reverses SCompact.  It can tell which atoms were singly
*    connected assuming the constant was big enough and so it can simply
*    link them into the TSD after expanding it.
*
*  Used to be:
*
*    expand:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Address of expanded TSD
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Tsd_t *SExpand
  (
  Tsd_t        *shorttsd_p                  /* Short TSD */
  )
{
  Tsd_t        *tsd_p;                      /* Temporary */
  U16_t         atom;                       /* Counter */
  U16_t         short_atoms;                /* # atoms in short TSD */
  U16_t         long_atoms;                 /* # atoms in expanded TSD */
  U16_t         extra_atoms;                /* Counter */
  U8_t          neigh;                      /* Counter */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Entering SExpand, tsd = %p", shorttsd_p));

  short_atoms = Tsd_NumAtoms_Get (shorttsd_p);

  /* Count the singly connected atoms that should be replaced by a separate row
     in the expanded tsd
  */

  for (atom = 0, extra_atoms = 0; atom < short_atoms; atom++)
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      if (Tsd_Atom_NeighborId_Get (shorttsd_p, atom, neigh) > CANON_SHORT_DIFF
          && Tsd_Atom_NeighborId_Get (shorttsd_p, atom, neigh) != TSD_INVALID)
        extra_atoms++;

  long_atoms = short_atoms + extra_atoms;
  tsd_p = Tsd_Copy (shorttsd_p);

  if (!extra_atoms)
    {
    DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
      "Leaving SExpand, expanded tsd = %p", tsd_p));

    return tsd_p;
    }

  Tsd_Expand (tsd_p, long_atoms);

  for (atom = 0, extra_atoms = short_atoms; atom < short_atoms; atom++)
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      if (Tsd_Atom_NeighborId_Get (tsd_p, atom, neigh) > CANON_SHORT_DIFF &&
          Tsd_Atom_NeighborId_Get (tsd_p, atom, neigh) != TSD_INVALID)
        {
        Tsd_Atomid_Put (tsd_p, extra_atoms, Tsd_Atom_NeighborId_Get (tsd_p,
          atom, neigh) - CANON_SHORT_DIFF);
        Tsd_Atom_NeighborId_Put (tsd_p, atom, neigh, extra_atoms);
        Tsd_Atom_NeighborId_Put (tsd_p, extra_atoms, 0, atom);
        Tsd_Atom_NeighborBond_Put (tsd_p, extra_atoms, 0,
          Tsd_Atom_NeighborBond_Get (tsd_p, atom, neigh));
        extra_atoms++;
        }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SExpand, expanded tsd = %p", tsd_p));

  return tsd_p;
}

/****************************************************************************
*
*  Function Name:                 SFirstIndexProcess
*
*    This routine processes a given atom as the first one in the TSD to see
*    if a "smaller" canonical name / parity occurs.
*
*  Used to be:
*
*    process_ifirst:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SFirstIndexProcess
  (
  Stack_t     *first_st_p,                  /* Stack of TSD indices */
  Stack_t     *asi_st_p,                    /* 1-d word, ASI to process */
  Stack_t     *tsdindex_st_p,               /* 1-d word, TSD indexes */
  Boolean_t   *all_unique_p,                /* Flag for all atoms unique */
  Array_t     *unique_p,                    /* 1-d bit, unique flags */
  Tsd_t       *shorttsd_p,                  /* Original compact TSD */
  Array_t     *bondweight_p,                /* 1-d word, bond weights */
  Array_t     *oldasi_p,                    /* 1-d word, old ASI */
  Array_t     *neighweight_p                /* 2-d word, neighbor weights */
  )
{
  U16_t        atom;                        /* Counter */
  U16_t        short_atoms;                 /* Number atoms in compact TSD */
  Array_t     *asi_p;                       /* Array from stack */
  Array_t     *tsdindex_p;                  /* Array from stack */

  short_atoms = Tsd_NumAtoms_Get (shorttsd_p);

  SPushDown (asi_st_p, tsdindex_st_p, first_st_p);
  asi_p = AtmArr_Array_Get ((AtomArray_t *) Stack_TopAdd (asi_st_p));
  tsdindex_p = AtmArr_Array_Get ((AtomArray_t *) Stack_TopAdd (tsdindex_st_p));
  SBreak (asi_p, tsdindex_p, Stack_TopU16 (first_st_p));
  SDifferentiator (asi_p, tsdindex_p, all_unique_p, unique_p, shorttsd_p,
    oldasi_p, neighweight_p);

  Stack_Pop (first_st_p);
  Stack_PushU16 (first_st_p, short_atoms + 1);
  for (atom = 0; atom < short_atoms - 1; atom++)
    {
    if (Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, atom)) ==
        Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, atom + 1)))
      {
      Stack_Pop (first_st_p);
      Stack_PushU16 (first_st_p, atom);
      return;
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SIdCheck
*
*    This function converts an atom id into a canonical search atom id.
*
*  Used to be:
*
*    check_id:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Canonical atom id
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U16_t SIdCheck
  (
  U16_t         id                         /* Canonical atom identifier */
  )
{
  if (id > 0 && id <= MX_CANON_ATOM)
    return SAtomLocate (Atomid2Symbol (id));

  if (id > MX_CANON_ATOM && id <= SPECIAL_END)
    return id;

  if (id >= VARIABLE_START && id <= VARIABLE_END)     
    return MX_CANON_ID + id;

  IO_Exit_Error (R_SLING, X_SYNERR, 
    "Sling_Canonical_Generate :: SIdCheck, invalid atom id found");

  return id;                     /* To satisfy Lint about IO_Exit_Error */
}

/****************************************************************************
*
*  Function Name:                 SIdRestore
*
*    This routine converts a canonical search atom id into a normal atom id.
*
*  Used to be:
*
*    restore_id:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Normal atom id
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U16_t SIdRestore
  (
  U16_t         id                         /* Canonical atom identifier */
  )
{
  if (id > 0 && id <= MX_CANON_ATOM)
    return Atomsymbol2Id ((U8_t *)SAtomsymbols [id]);

  if (id > MX_CANON_ID)
/*
    return MX_CANON_ID - id;
*/
    return id - MX_CANON_ID; /* don't screw up variables for ANYCEBS!!! */

  if (id > MX_CANON_ATOM && id <= SPECIAL_END)
    return id;

  IO_Exit_Error (R_SLING, X_SYNERR, "Sling_Canonical_Generate :: SIdRestore, atom id out of bounds");

  return id;                     /* To satisfy Lint about IO_Exit_Error */
}

/****************************************************************************
*
*  Function Name:                 SLetter
*
*    This function converts an atom, superatom or variable atom id into the
*    corresponding symbol.
*
*  Used to be:
*
*    letter:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static String_t SLetter
  (
  S16_t         atom                       /* Atom id to convert */ 
  )
{
  char          tbuf[7];                   /* Pointer to C-string of atom */
  String_t      result;                    /* Result of conversion */

  tbuf[0] = '(';

  strcpy (&tbuf[1], Atomid2Symbol (atom));

  atom = strlen (&tbuf[1]);
  if (atom > 1)
    {
    tbuf[++atom] = ')';
    tbuf[++atom] = '\0';
    result = String_Create (tbuf, sizeof (tbuf));
    }
  else
    result = String_Create (&tbuf[1], sizeof (tbuf));

  return result;
}

/****************************************************************************
*
*  Function Name:                 SListAdd
*
*    This routine adds a Coded TSD to the list of them.  It checks to make
*    sure that it isn't already in the list and that it is inserted into
*    the correct place.
*
*  Used to be:
*
*    add_to_coded_tsd_list:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
static void SListAdd
  (
  CodedTsd_t **tsdlisthead_pp,              /* Head of coded TSD list */
  CodedTsd_t **newplace_pp,                 /* Where to insert this new one */
  U16_t       *index_p,                     /* Where the insertion ends up */
  Tsd_t       *tsd_p,                       /* TSD to add */
  Array_t     *bondweight_p,                /* 1-d word, bond weights */
  Array_t     *tsdindex_p,                  /* 1-d word, mapping */
  Array_t     *tsdcoded_p                   /* 1-d word, which atoms encoded */
  )
{
  CodedTsd_t  *codedtsd_p;                  /* Temporary for new one */
  CodedTsd_t  *before_p;                    /* For list manipulation */
  CodedTsd_t  *after_p;                     /* For list manipulation */
  U16_t        atom;                        /* Counter */
  U16_t        last_nonh;                   /* Index of last non-hydrogen */

#ifdef _MIND_MEM_
  mind_malloc ("codedtsd_p", "sling{10}", &codedtsd_p, CODEDTSDSIZE);
#else
  Mem_Alloc (CodedTsd_t *, codedtsd_p, CODEDTSDSIZE, GLOBAL);
#endif

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_MEMORY, (outbuf,
    "Allocated memory for a Coded Tsd in SListAdd at %p", codedtsd_p));

  if (codedtsd_p == NULL)
    IO_Exit_Error (R_SLING, X_LIBCALL, "No memory for Coded Tsd in SListAdd");

  for (last_nonh = Tsd_NumAtoms_Get (tsd_p) - 1; last_nonh != 0 &&
       Tsd_Atomid_Get (tsd_p, last_nonh) / CANON_ATOMID_PERM == CANON_HYDROGEN;
       last_nonh--)
    /* Empty loop body */ ;

  CodeTsd_Next_Put (codedtsd_p, NULL);
  CodeTsd_Parity_Put (codedtsd_p, NULL);
  CodeTsd_Tsd_Put (codedtsd_p, tsd_p);
#ifdef _MIND_MEM_
  mind_Array_Copy ("CodeTsd_Map_Get(codedtsd_p)", "sling{10}", tsdindex_p, CodeTsd_Map_Get (codedtsd_p));
  mind_Array_Copy ("CodeTsd_BondWeight_Get(codedtsd_p)", "sling{10}", bondweight_p, CodeTsd_BondWeight_Get (codedtsd_p));
#else
  Array_Copy (tsdindex_p, CodeTsd_Map_Get (codedtsd_p));
  Array_Copy (bondweight_p, CodeTsd_BondWeight_Get (codedtsd_p));
#endif
  CodeTsd_LastNonH_Put (codedtsd_p, last_nonh);

  *newplace_pp = SListSearch (*tsdlisthead_pp, codedtsd_p, index_p);

  if (*newplace_pp != NULL)
    {
    Tsd_Destroy (CodeTsd_Tsd_Get (codedtsd_p));
#ifdef _MIND_MEM_
    mind_Array_Destroy ("CodeTsd_Map_Get(codedtsd_p)", "sling", CodeTsd_Map_Get (codedtsd_p));
    mind_Array_Destroy ("CodeTsd_BondWeight_Get(codedtsd_p)", "sling", CodeTsd_BondWeight_Get (codedtsd_p));

    mind_free ("codedtsd_p", "sling", codedtsd_p);
#else
    Array_Destroy (CodeTsd_Map_Get (codedtsd_p));
    Array_Destroy (CodeTsd_BondWeight_Get (codedtsd_p));

    Mem_Dealloc (codedtsd_p, CODEDTSDSIZE, GLOBAL);
#endif

    DEBUG (R_SLING, DB_SLINGSTATIC, TL_MEMORY, (outbuf,
      "Deallocated memory for a Coded Tsd in SListAdd at %p", codedtsd_p));

    return;
    }

  if (*tsdlisthead_pp == NULL)
    {
    *tsdlisthead_pp = codedtsd_p;
    *index_p = 1;
    }
  else
    {
    SListPlaceFind (*tsdlisthead_pp, codedtsd_p, &before_p, &after_p, index_p);

    /* Insert the new Coded Tsd in its proper place */

    if (before_p == NULL)
      *tsdlisthead_pp = codedtsd_p;
    else
      CodeTsd_Next_Put (before_p, codedtsd_p);

    CodeTsd_Next_Put (codedtsd_p, after_p);
    }

  for (atom = 0; atom < Array_Columns_Get (tsdcoded_p); atom++)
    if (Array_1d16_Get (tsdcoded_p, atom) >= *index_p)
      Array_1d16_Put (tsdcoded_p, atom, Array_1d16_Get (tsdcoded_p, atom) + 1);

  *newplace_pp = codedtsd_p;

  return;
}

/****************************************************************************
*
*  Function Name:                 SListPlaceFind
*
*    This routine finds the place to insert into the list for the Coded
*    TSD.  The ordering function is lexical less than, so that the smallest
*    entry lexically appears at the head of the list.
*
*  Used to be:
*
*    find_place:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SListPlaceFind
  (
  CodedTsd_t  *tsdlisthead_p,               /* Head of list of coded TSDs */
  CodedTsd_t  *curcoded_p,                  /* Current one to find */
  CodedTsd_t **before_pp,                   /* Prior entry address */
  CodedTsd_t **after_pp,                    /* Next entry address */
  U16_t       *list_index_p                 /* Which place in list is this */
  )
{
  for (*before_pp = NULL, *after_pp = tsdlisthead_p, *list_index_p = 1;
       *after_pp != NULL; *before_pp = *after_pp,
       *after_pp = CodeTsd_Next_Get (*after_pp), (*list_index_p)++)
    if (SCodedTsdLessThan (*after_pp, curcoded_p))
      return;

  return;
}

/****************************************************************************
*
*  Function Name:                 SListSearch
*
*    This function finds the place in the list where the TSD exists.  If it
*    does not yet exist, then NULL is returned.
*
*  Used to be:
*
*    search_coded_tsd_list:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Address of TSD in coded form in list
*    NULL - TSD not found in list
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static CodedTsd_t *SListSearch
  (
  CodedTsd_t  *tsdlisthead_p,               /* Head of list to search */
  CodedTsd_t  *curcoded_p,                  /* Which one we are looking for */
  U16_t       *list_index_p                 /* Where in list we end up */
  )
{
  CodedTsd_t  *found_p;                     /* Output address of found */

  for (*list_index_p = 1, found_p = tsdlisthead_p ; found_p != NULL;
       found_p = CodeTsd_Next_Get (found_p), (*list_index_p)++)
    if (SCodedTsdEqual (found_p, curcoded_p))
      {
      DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf,
        "Coded Tsd %p, found at position %u", curcoded_p, *list_index_p));

      return found_p;
      }

  if (tsdlisthead_p == NULL)
    *list_index_p = 0;

  return NULL;
}

/****************************************************************************
*
*  Function Name:                 SListSmallestFind
*
*    This routine checks to see if a given parity is the smallest yet
*    seen and updates the parity vector of the Coded TSD appropriately.
*
*  Used to be:
*
*    find_smallest:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SListSmallestFind
  (
  CodedTsd_t  *curcoded_p,                  /* Current coded TSD */
  Array_t     *newparity_p,                 /* 1-d byte, parity indicators */
  Boolean_t   *found_p,                     /* Flag for successful search */
  Boolean_t   *smallest_p,                  /* Current is smallest */
  U16_t        num_carbons                  /* # carbon atoms in molecule */
  )
{
  AtomArray_t *parity_p;                    /* For list traversal */
  U16_t        atom;                        /* Counter */
  Boolean_t    equal;                       /* Flag for equal parity */
  Boolean_t    smaller;                     /* Flag for smaller parity */

  for (parity_p = CodeTsd_Parity_Get (curcoded_p), *found_p = FALSE,
       *smallest_p = TRUE; parity_p != NULL; parity_p = AtmArr_Next_Get (
       parity_p))
    {
    for (atom = 0, smaller = FALSE, equal = TRUE; atom < num_carbons && equal;
         atom++)
      {
      if ((S8_t)Array_1d8_Get (newparity_p, atom) < (S8_t) Array_1d8_Get (
          AtmArr_Array_Get (parity_p), atom))
        smaller = TRUE;

      equal = (Array_1d8_Get (newparity_p, atom) == Array_1d8_Get (
        AtmArr_Array_Get (parity_p), atom));
      }

    if (equal)
      {
      *found_p = TRUE;
      *smallest_p = FALSE;
      return;
      }

    if (!smaller)
      *smallest_p = FALSE;
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SNumberSkip
*
*    This routine simply skips over numbers in the Sling.  For the 1st pass
*    across it.
*
*  Used to be:
*
*    skip_numbers:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SNumberSkip
  (
  Slng2TsdCB_t *stcb_p                     /* Control block */
  )
{
  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Entering SNumberSkip, control block ignored"));

  while (isdigit (S2TCB_NextChar_Get (stcb_p)))
    {
    S2TCB_NextChar_Set (stcb_p);
    if (S2TCB_AtSlingEnd (stcb_p))
      {
      S2TCB_NextChar_Put (stcb_p, ' ');
      return;
      }
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SNumberSkip, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SNumberRead
*
*    This routine reads a number in the Sling and returns its value.
*
*  Used to be:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U8_t SNumberRead
  (
  Slng2TsdCB_t *stcb_p                     /* Control block */
  )
{
  U8_t value, *c;

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Entering SNumberRead, control block ignored"));

  value = 0;

  c = S2TCB_Offset_Get (stcb_p);
#ifdef _DEBUG_
printf("first position: %c\n",*c);
#endif
  while (isdigit (*c))
    {
#ifdef _DEBUG_
printf("isdigit: %c\n",*c);
#endif
    value = 10 * value + *c - '0';
    c++;
    if (c > S2TCB_End_Get (stcb_p))
      return (value);
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SNumberRead, status = <void>"));

  return (value);
}

/****************************************************************************
*
*  Function Name:                 SOlefinAtomFix
*
*    This routine fixes up an atom with one olefin bond.  The other bonds
*    (and neighbors) should be in the "up" and "down" columns to reflect
*    proper parity.
*
*  Used to be:
*
*    move_lower_bonds:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SOlefinAtomFix
  (
  Tsd_t        *tsd_p,                     /* Molecule to fix up */
  Array_t      *olefin_done_p,             /* 1-d bit, olefin proc. flags */
  U16_t         atom,                      /* Atom to fix up */
  U8_t          bondslot                   /* Which bond is the olefin */
  )
{
  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Entering\
 SOlefinAtomFix, tsd = %p, olefin flags = %p, atom = %u, bond = %u", tsd_p,
 olefin_done_p, atom, bondslot));

  Array_1d1_Put (olefin_done_p, atom, TRUE);
  if (bondslot == BOND_DIR_UP)
    {
    Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_UP,
      Tsd_Atom_NeighborId_Get (tsd_p, atom, BOND_DIR_DOWN));
    Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_DOWN,
      Tsd_Atom_NeighborId_Get (tsd_p, atom, BOND_DIR_LEFT));
    Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_UP,
      Tsd_Atom_NeighborBond_Get (tsd_p, atom, BOND_DIR_DOWN));
    Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_DOWN,
      Tsd_Atom_NeighborBond_Get (tsd_p, atom, BOND_DIR_LEFT));
    }
  else
    if (bondslot == BOND_DIR_DOWN)
      {
      Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_DOWN,
        Tsd_Atom_NeighborId_Get (tsd_p, atom, BOND_DIR_UP));
      Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_UP,
        Tsd_Atom_NeighborId_Get (tsd_p, atom, BOND_DIR_LEFT));
      Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_DOWN,
        Tsd_Atom_NeighborBond_Get (tsd_p, atom, BOND_DIR_UP));
      Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_UP,
        Tsd_Atom_NeighborBond_Get (tsd_p, atom, BOND_DIR_LEFT));
      }

  Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_LEFT, TSD_INVALID);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_LEFT, BOND_NONE);

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SOlefinAtomFix, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SParityCheck
*
*    This routine marks the TSD with the parity information from the Sling.
*    It marks an atom as either don't care, or one whose parity is opposite
*    that indicated by the order of its bonds in the Sling.
*
*  Used to be:
*
*    check_for_parity:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SParityCheck
  (
  Tsd_t        *tsd_p,                     /* Molecule to parity in */
  Array_t      *parity_wrong_p,            /* Array to record parity in */
  U16_t         atom,                      /* Which atom in TSD to look at */
  Slng2TsdCB_t *stcb_p                     /* Control block */
  )
{
  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, 
    "Entering SParityCheck, tsd = %p, parity wrong = %p, atom = %u", 
    tsd_p, parity_wrong_p, atom));

  if (S2TCB_NextChar_Get (stcb_p) == DONTCARE_SYM)
    {
    Tsd_AtomFlags_DontCare_Put (tsd_p, atom, TRUE);
    S2TCB_NextChar_Set (stcb_p);
    }

  if (S2TCB_NextChar_Get (stcb_p) == PARITY_SPACE_SYM)
    {
    Array_1d1_Put (parity_wrong_p, atom, TRUE);
    S2TCB_NextChar_Set (stcb_p);
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SParityCheck, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SParityPushDown
*
*    This routine saves the parity and mapping information in the Coded TSD
*    if this is an improvement over what is currently in the list.
*
*  Used to be:
*
*    push_down_parity:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SParityPushDown
  (
  Boolean_t   *chiral_p,                    /* Chiral molecule flag */
  CodedTsd_t  *curcoded_p,                  /* Current coded TSD */
  Array_t     *newparity_p,                 /* 1-d byte, parity indicators */
  Array_t     *tsdindex_p,                  /* 1-d word, mapping */
  U16_t        num_carbons                  /* # carbon atoms in molecule */
  )
{
  AtomArray_t *parity_p;                    /* New parity vector */
  U16_t        atom;                        /* Counter */
  Boolean_t    found;                       /* Flag from search */
  Boolean_t    smallest;                    /* Flag from search */

  if (*chiral_p)
    SChiralityCheck (chiral_p, curcoded_p, newparity_p, num_carbons);

  SListSmallestFind (curcoded_p, newparity_p, &found, &smallest, num_carbons);
  if (!found)
    {
    /* Copy in TSD index and parity */

    parity_p = AtmArr_Create (num_carbons, ATMARR_NOBONDS_BYTE, PARITY_INIT);
    AtmArr_Next_Put (parity_p, CodeTsd_Parity_Get (curcoded_p));
    CodeTsd_Parity_Put (curcoded_p, parity_p);

    if (smallest)
      Array_CopyContents (tsdindex_p, CodeTsd_Map_Get (curcoded_p));

    for (atom = 0; atom < num_carbons; atom++)
      Array_1d8_Put (AtmArr_Array_Get (CodeTsd_Parity_Get (curcoded_p)), atom,
        Array_1d8_Get (newparity_p, atom));
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SParitySkip
*
*    This routine simply skips over parity information in the Sling.  For the
*    1st pass across it.
*
*  Used to be:
*
*    skip_parity:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SParitySkip
  (
  Slng2TsdCB_t *stcb_p                     /* Control block */
  )
{
  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Entering SParitySkip, control block ignored"));

  if (S2TCB_NextChar_Get (stcb_p) == PARITY_SPACE_SYM || S2TCB_NextChar_Get (
      stcb_p) == DONTCARE_SYM)
    {
    if (S2TCB_Offset_Get (stcb_p) < S2TCB_End_Get (stcb_p)) 
      {
      S2TCB_NextChar_Set (stcb_p);
      }
    else
      S2TCB_Offset_Incr (stcb_p);
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SParitySkip, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SParityUpdate
*
*    This routine controls the updating of parity in a newly generated TSD.
*
*  Used to be:
*
*    update_parity:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SParityUpdate
  (
  Array_t     *asi_p,                       /* 1-d word, ASI */
  Array_t     *tsdindex_p,                  /* 1-d word, TSD index */
  Array_t     *newparity_p,                 /* 1-d byte, parity values */
  Array_t     *asymmetry_vec_p,             /* 1-d bit, asymmetrical atoms */
  Array_t     *minasi_p,                    /* 1-d word, minimum ASI */
  Array_t     *tsdcoded_p,                  /* 1-d word,which TSDs are coded */
  Array_t     *parity_vecs_p,               /* 2-d byte, parity values */
  U16_t        list_index                   /* Which TSD on list */
  )
{
  U16_t        num_carbons;                 /* # carbon atoms in molecule */
  U16_t        short_atoms;                 /* # atoms in compacted molecule */
  U16_t        atom;                        /* Counter */
  U16_t        index;                       /* Temporary */
  U8_t         parity;                      /* Parity value */

  short_atoms = (U16_t) Array_Columns_Get (asi_p);
  num_carbons = (U16_t) Array_Columns_Get (parity_vecs_p);
  for (atom = 0, parity = PARITY_INIT; atom < short_atoms;
       atom++)
    {
    index = Array_1d16_Get (tsdindex_p, atom);
    if (Array_1d16_Get (asi_p, index) < num_carbons)
      parity = Array_1d8_Get (newparity_p, Array_1d16_Get (asi_p, index));

    if (parity == PARITY_INIT)
      Array_1d1_Put (asymmetry_vec_p, index, FALSE);

    if (Array_1d16_Get (minasi_p, index) == Array_1d16_Get (asi_p, index))
      {
      if (Array_1d16_Get (tsdcoded_p, index) == list_index)
        {
        if (Array_1d16_Get (asi_p, index) < num_carbons)
          SParityUpdate1 (index, parity, asi_p, parity_vecs_p, newparity_p,
            asymmetry_vec_p);

        SParityUpdate2 (index, parity_vecs_p, newparity_p);
        }
      else
        if (Array_1d16_Get (tsdcoded_p, index) > list_index)
          {
          Array_1d16_Put (tsdcoded_p, index, list_index);
          SParityUpdate3 (index, parity_vecs_p, newparity_p);
          }
      }

    if (Array_1d16_Get (minasi_p, index) > Array_1d16_Get (asi_p, index))
      {
      Array_1d16_Put (tsdcoded_p, index, list_index);
      Array_1d16_Put (minasi_p, index, Array_1d16_Get (asi_p, index));
      SParityUpdate3 (index, parity_vecs_p, newparity_p);
      }
    }                      /* End for-atoms loop */

  return;
}

/****************************************************************************
*
*  Function Name:                 SParityUpdate1
*
*    This routine updates the asymmetry flag for a given atom.
*
*  Used to be:
*
*    upar1:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SParityUpdate1
  (
  U16_t        index,                       /* Current TSD index value */
  S8_t         parity,                      /* New parity value */
  Array_t     *asi_p,                       /* 1-d word, ASI to putz with */
  Array_t     *parity_vecs_p,               /* 2-d word, parity of neighbors */
  Array_t     *newparity_p,                 /* 1-d byte, parity indicators */
  Array_t     *asymmetry_vec_p              /* 1-d bit, asymmetric atoms */
  )
{
  U16_t        num_carbons;                 /* # carbon atoms */
  U16_t        atom;                        /* Counter */
  U16_t        parity1_cnt;                 /* # carbons with 1 parity */
  U16_t        parity2_cnt;                 /* # carbons with 2 parity */
  S8_t         old_parity;                  /* Temporary */
  S8_t         cur_parity;                  /* Temporary */

  num_carbons = (U16_t) Array_Columns_Get (parity_vecs_p);

  if (parity == Array_2d8_Get (parity_vecs_p, index, Array_1d16_Get (asi_p,
      index)))
    return;

  for (atom = 0, parity1_cnt = 0, parity2_cnt = 0; atom < num_carbons; atom++)
    {
    old_parity = Array_2d8_Get (parity_vecs_p, index, atom);
    cur_parity = Array_1d8_Get (newparity_p, atom);
    if (old_parity + cur_parity == 0)
      {
      if (abs (old_parity) == 1)      /* Tetravalent carbon */
        parity1_cnt++;
      else
        if (abs (old_parity) == 2)    /* Trivalent carbon */
          parity2_cnt++;
      }
    }

  if ((abs (parity) == 1 && parity2_cnt == 0 && parity1_cnt == 1) ||
      (abs (parity) == 2 && parity2_cnt == 2 && parity1_cnt == 0))
    Array_1d1_Put (asymmetry_vec_p, index, FALSE);

  return;
}

/****************************************************************************
*
*  Function Name:                 SParityUpdate2
*
*    This routine updates the parity vector, but checks to see if the parity
*    is getting smaller ?
*
*  Used to be:
*
*    upar2:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SParityUpdate2
  (
  U16_t        index,                       /* Current TSD index value */
  Array_t     *parity_vecs_p,               /* 2-d byte, parity by carbons */
  Array_t     *newparity_p                  /* 1-d byte, parity indicators */
  )
{
  U16_t        atom;                        /* Counter */
  U16_t        num_carbons;                 /* # carbon atoms in molecule */
  Boolean_t    flag1;                       /* Flag for larger parity */
  Boolean_t    flag2;                       /* Flag for updating */

  num_carbons = (U16_t) Array_Columns_Get (parity_vecs_p);

  for (atom = 0, flag2 = FALSE, flag1 = FALSE; atom < num_carbons &&
       flag1 == FALSE; atom++)
    {
    if (flag2 == TRUE)
      {
      Array_2d8_Put (parity_vecs_p, index, atom, Array_1d8_Get (newparity_p,
        atom));
      }
    else
      {
      if ((S8_t)Array_1d8_Get (newparity_p, atom) < (S8_t)Array_2d8_Get (
          parity_vecs_p, index, atom))
        {
        Array_2d8_Put (parity_vecs_p, index, atom, Array_1d8_Get (newparity_p,
          atom));
        flag2 = TRUE;
        }

      if ((S8_t)Array_1d8_Get (newparity_p, atom) > (S8_t)Array_2d8_Get (
          parity_vecs_p, index, atom))
        flag1 = TRUE;
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SParityUpdate3
*
*    This routine simply updates the parity vector with the parity for this
*    version of the TSD.
*
*  Used to be:
*
*    upar3:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SParityUpdate3
  (
  U16_t        index,                       /* Current TSD index value */
  Array_t     *parity_vecs_p,               /* 2-d byte, parity by carbons */
  Array_t     *newparity_p                  /* 1-d byte, parity indicators */
  )
{
  U16_t        atom;                        /* Counter */
  U16_t        num_carbons;                 /* # carbons in molecule */

  num_carbons = (U16_t) Array_Columns_Get (parity_vecs_p);

  for (atom = 0; atom < num_carbons; atom++)
    Array_2d8_Put (parity_vecs_p, index, atom, Array_1d8_Get (newparity_p,
      atom));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPointerMove
*
*    This routine checks for a retrace or forward trace in the Sling, and if
*    it finds one, then it updates the stack of atoms to reflect where the
*    next atom to come from is.
*
*  Used to be:
*
*    move_pointer:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SPointerMove
  (
  Stack_t      *stack_p,                   /* What is in TSD slot stack */
  Slng2TsdCB_t *stcb_p                     /* Control block */
  )
{
  U8_t         *pointer;                   /* For string munging */
  S16_t         num_moves;                 /* Number of atoms to skip */
  U8_t          next;                      /* For char processing */
  Boolean_t     negative;                  /* Negative number found */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Entering SPointerMove,\
 stack = %p", stack_p));

  next = S2TCB_NextChar_Get (stcb_p);
  if ((next == FORTRACE_SYM) || (next == RETRACE_SYM) || (isdigit (next)))
    {
    if (!isdigit (next))
      {
      negative = (next == RETRACE_SYM);
      pointer = S2TCB_Offset_Get (stcb_p) + 1;
      next = *pointer;
      }
    else
      {
      negative = FALSE;
      pointer = S2TCB_Offset_Get (stcb_p);
      }

    for (num_moves = 0; pointer <= S2TCB_End_Get (stcb_p) && isdigit (next);
         next = *pointer)
      {
      num_moves = num_moves * 10 + (next - '0');
      pointer++;
      }

    if (negative)
      num_moves *= -1;

    S2TCB_Offset_Put (stcb_p, pointer);
    if (num_moves >= 0)
      {
      Stack_PushS16 (stack_p, num_moves);
      }
    else
      for (num_moves = -num_moves; num_moves > 0; num_moves--)
        {
        Stack_Pop (stack_p);
        }
    }              /* End if-next_char ... */

  S2TCB_NextChar_Put (stcb_p, next);

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SPointerMove, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPushDown
*
*    This routine creates new TSD index and associated ASI arrays.  Used with
*    SFirstIndexProcess to try for "smaller" molecules.
*
*  Used to be:
*
*    push_down:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SPushDown
  (
  Stack_t     *asi_st_p,                    /* 1-d word, ASI to process */
  Stack_t     *tsdindex_st_p,               /* 1-d word, TSD indexes */
  Stack_t     *first_st_p                   /* Stack of indices */
  )
{
  AtomArray_t *atmarr_p;                    /* Temporary */

  atmarr_p = AtmArr_Copy ((AtomArray_t *)Stack_TopAdd (asi_st_p));
  Stack_PushAdd (asi_st_p, atmarr_p);

  atmarr_p = AtmArr_Copy ((AtomArray_t *)Stack_TopAdd (tsdindex_st_p));
  Stack_PushAdd (tsdindex_st_p, atmarr_p);

  Stack_PushU16 (first_st_p, Stack_TopU16 (first_st_p));

  return;
}

/****************************************************************************
*
*  Function Name:                 SRowCopy
*
*    This routine copies the neighbor atom indexes into an array for sorting.
*
*  Used to be:
*
*    copy_row:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SRowCopy
  (
  U16_t         start,                      /* Starting neighbor */
  U16_t         end,                        /* Ending neighbor */
  U16_t        *vector_p,                   /* What to copy into */
  U16_t         atom,                       /* Which atom to read */
  U16_t         offset,                     /* Offset to add for compare */
  Tsd_t        *tsd_p,                      /* Molecule to copy from */
  Array_t      *asi_p                       /* 1-d word, ASI of m'cule */
  )
{
  U16_t         neighid;                    /* Temporary */
  U16_t         i;                          /* Counter */

  for (i = start; i <= end; i++)
    {
    neighid = Tsd_Atom_NeighborId_Get (tsd_p, atom, i);
    if (neighid > CANON_SHORT_DIFF)
      vector_p[i + offset] = neighid;
    else
      vector_p[i + offset] = Array_1d16_Get (asi_p, neighid);
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SSwap
*
*    This routine swaps to numbers (usually in an array).
*
*  Used to be:
*
*    swap:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SSwap
  (
  U16_t        *x_p,                       /* One to swap */
  U16_t        *y_p                        /* Other to swap */
  )
{
  U16_t         i;                         /* Temporary */

  i = *x_p;
  *x_p = *y_p;
  *y_p = i;

  return;
}

/****************************************************************************
*
*  Function Name:                 STieBreaker
*
*    This routine makes sure that everything is set correctly when a tie
*    breaker is needed - TSD list, parity vectors, etc.
*
*  Used to be:
*
*    tie_breaker:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void STieBreaker
  (
  Array_t     *asi_p,                       /* 1-d word, ASI to putz with */
  Array_t     *tsdindex_p,                  /* 1-d word, TSD index to putz */
  U16_t        first_idx,                   /* Index of tie */
  Boolean_t   *chiral_p,                    /* Molecule chirality flag */
  Boolean_t   *all_unique_p,                /* Result of SDifferentiator */
  Array_t     *unique_p,                    /* 1-d bit, uniqueness of atoms */
  Tsd_t       *shorttsd_p,                  /* Compacted TSD */
  Array_t     *oldasi_p,                    /* 1-d word, previous ASI */
  Array_t     *neighweight_p,               /* 2-d word, neighbor weights */
  Array_t     *nextparity_p,                /* 1-d byte, parity indicators */
  Array_t     *minasi_p,                    /* 1-d word, minimum ASI */
  Array_t     *asymmetry_vec_p,             /* 1-d bit, asymmetry flags */
  Array_t     *parity_vecs_p,               /* 2-d word, parity by neighbors */
  CodedTsd_t **tsdlisthead_pp,              /* Head of coded TSD list */
  Array_t     *tsdcoded_p,                  /* 1-d word, which atoms coded */
  Tsd_t       *curtsd_p,                    /* Current TSD */
  Array_t     *curbondweight_p              /* 1-d word, bond weights */
  )
{
  CodedTsd_t  *place_p;                     /* Place in linked list */
  AtomArray_t *atmarr_p;                    /* Temporary for stack crud */
  Stack_t     *first_st_p;                  /* Stack of firsts */
  Stack_t     *asi_st_p;                    /* Stack of ASIs */
  Stack_t     *tsdindex_st_p;               /* Stack of TSD indices */
  Tsd_t       *nexttsd_p;                   /* Computed TSD for encoding */
  U16_t        place_cnt;                   /* How far down the list */
  U16_t        num_carbons;                 /* # carbon atoms in molecule */
  U16_t        short_atoms;                 /* # atoms in compact TSD */
  Boolean_t    more_ties;                   /* Flag for when done */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Entering STieBreaker,"
    " asi = %p, tsd index = %p, first index = %u, chiral = %u, unique = %p,"
    " short tsd = %p, old asi = %p, neighbor weights = %p, next parity = %p,"
    " min asi = %p, asymmetry = %p, parity vectors = %p, tsd list head = %p,"
    " tsd coded = %p, current tsd = %p, current bond weights = %p", asi_p,
    tsdindex_p, first_idx, *chiral_p, unique_p, shorttsd_p, oldasi_p,
    neighweight_p, nextparity_p, minasi_p, asymmetry_vec_p, parity_vecs_p,
    *tsdlisthead_pp, tsdcoded_p, curtsd_p, curbondweight_p));

  num_carbons = (U16_t) Array_Columns_Get (parity_vecs_p);
  short_atoms = Tsd_NumAtoms_Get (shorttsd_p);
  first_st_p = Stack_Create (STACK_SCALAR);
  Stack_PushU16 (first_st_p, first_idx);
  asi_st_p = Stack_Create (STACK_ATMARR);
  atmarr_p = AtmArr_Create (short_atoms, ATMARR_NOBONDS_WORD, 0);
  Array_CopyContents (asi_p, AtmArr_Array_Get (atmarr_p));
  Stack_PushAdd (asi_st_p, atmarr_p);
  tsdindex_st_p = Stack_Create (STACK_ATMARR);
  atmarr_p = AtmArr_Create (short_atoms, ATMARR_NOBONDS_WORD, 0);
  Array_CopyContents (tsdindex_p, AtmArr_Array_Get (atmarr_p));
  Stack_PushAdd (tsdindex_st_p, atmarr_p);
  
  more_ties = TRUE;
  while (more_ties)
    {
    while (first_idx < short_atoms)
      {
      SFirstIndexProcess (first_st_p, asi_st_p, tsdindex_st_p, all_unique_p,
        unique_p, shorttsd_p, curbondweight_p, oldasi_p, neighweight_p);
      first_idx = Stack_TopU16 (first_st_p);
      }

    asi_p = AtmArr_Array_Get ((AtomArray_t *) Stack_TopAdd (asi_st_p));
    tsdindex_p = AtmArr_Array_Get ((AtomArray_t *) Stack_TopAdd (
      tsdindex_st_p));

    /* No ties found */

    if (first_idx >= short_atoms)
      {
      nexttsd_p = Tsd_Create (short_atoms);
      SCompute (asi_p, tsdindex_p, shorttsd_p, nexttsd_p, nextparity_p,
        curtsd_p, curbondweight_p, num_carbons);

      SListAdd (tsdlisthead_pp, &place_p, &place_cnt, nexttsd_p,
        curbondweight_p, tsdindex_p, tsdcoded_p);

      if (num_carbons > 0)
        SParityPushDown (chiral_p, place_p, nextparity_p, tsdindex_p,
          num_carbons);

      SParityUpdate (asi_p, tsdindex_p, nextparity_p, asymmetry_vec_p,
        minasi_p, tsdcoded_p, parity_vecs_p, place_cnt);
      }

    more_ties = FALSE;
    while (!more_ties)
      {
      Stack_Pop (asi_st_p);
      Stack_Pop (tsdindex_st_p);
      Stack_Pop (first_st_p);
      if (Stack_Size_Get (first_st_p) == 0)
        {
        Stack_Destroy (first_st_p);
        Stack_Destroy (asi_st_p);
        Stack_Destroy (tsdindex_st_p);

        DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
          "Leaving STieBreaker, status = <void>"));

        return;
        }

      first_idx = Stack_TopU16 (first_st_p) + 1;
      Stack_Pop (first_st_p);
      Stack_PushU16 (first_st_p, first_idx);
      asi_p = AtmArr_Array_Get ((AtomArray_t *) Stack_TopAdd (asi_st_p));
      tsdindex_p = AtmArr_Array_Get ((AtomArray_t *) Stack_TopAdd (
        tsdindex_st_p));

      TRACE_DO (DB_SLINGSTATIC, TL_PARAMS,
        {
        TRACE (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf, 
          "Emptying stack of nodes to process and associated ASIs and"
          " TSD index arrays, first index = %u", first_idx));

        Array_Dump (asi_p, &GTraceFile);
        Array_Dump (tsdindex_p, &GTraceFile);
        });

      if (first_idx < short_atoms)
        if (Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, first_idx)) ==
            Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, first_idx - 1)))
          {
          more_ties = TRUE;

          DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf,
            "More ties to be broken in STieBreaker"));
          }
      }                   /* End while !more_ties */
    }                     /* End while more_ties */

  Stack_Destroy (first_st_p);
  Stack_Destroy (asi_st_p);
  Stack_Destroy (tsdindex_st_p);

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving STieBreaker, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 STsdDump
*
*    This routine prints a formatted dump of a TSD.
*
*  Used to be:
*
*    N/A
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void STsdDump
  (
  Tsd_t         *tsd_p,                      /* Address of TSD to format */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */
  U16_t          i, j;                       /* Counters */
  U16_t          neighid;                    /* Temporary */
  U8_t           bondsize, type;             /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (tsd_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL compact TSD\n");
    return;
    }

  fprintf (f, "Atom\tId\tFlags\tUp\tDown\tLeft\tRight\tIn\tOut\n");
  for (i = 0; i < Tsd_NumAtoms_Get (tsd_p); i++)
    {
    fprintf (f, "#%4u\t%s\t%u", i, SAtomsymbols [Tsd_Atomid_Get (tsd_p, i) /
      CANON_ATOMID_PERM], Tsd_Atom_Flags_Get (tsd_p, i));
    for (j = 0; j < MX_NEIGHBORS; j++)
      {
      bondsize = Tsd_Atom_NeighborBond_Get (tsd_p, i, j);
      neighid = Tsd_Atom_NeighborId_Get (tsd_p, i, j);
      switch (bondsize)
        {
        case 1:
        case 2:
        case 3:
        case 4:

          type = '0' + bondsize;
          break;

        case BOND_VARIABLE:

          type = 'v';
          break;

        case BOND_RESONANT:

          type = 'r';
          break;

        default:

          type = '?';
          break;
          }

      if (bondsize)
        {
        if (neighid > CANON_SHORT_DIFF)
          {
          neighid -= CANON_SHORT_DIFF;
          neighid /= CANON_ATOMID_PERM;
          fprintf (f, "\t%s:%c", SAtomsymbols [neighid], type);
          }
        else
          fprintf (f, "\t%u:%c", neighid, type);
        }

      else
        fprintf (f, "\t");
      }

    fprintf (f, "\n");
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 STsdSwap
*
*    This routine swaps two neighbors in a TSD.
*
*  Used to be:
*
*    swap:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void STsdSwap
  (
  Tsd_t        *tsd_p,                     /* TSD to munge */
  U16_t         atom,                      /* Atom to munge */
  U8_t          neigh1,                    /* One to swap */
  U8_t          neigh2                     /* Other to swap */
  )
{
  U8_t          temp;                      /* Temporary */

  temp = Tsd_Atom_NeighborBond_Get (tsd_p, atom, neigh1);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom, neigh1, Tsd_Atom_NeighborBond_Get (
    tsd_p, atom, neigh2));
  Tsd_Atom_NeighborBond_Put (tsd_p, atom, neigh2, temp);

  return;
}

/****************************************************************************
*
*  Function Name:                 STsdUnique
*
*    This function generates a single unique TSD.  The uniqueness is
*    determined by the ordering of the atoms in the TSD.  The canonical order
*    is likely described in the documentation.
*
*  Used to be:
*
*    unique_tsd:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Address of unique TSD
*
*  Side Effects:
*
*    May call IO_Exit_Error
*
******************************************************************************/
static Tsd_t *STsdUnique
  (
  Tsd_t        *shorttsd_p,                 /* Compacted TSD */
  AtomArray_t **parityvector_pp,            /* Parity linked list */
  Array_t      *cebs_p,                     /* 1-d word, CE brothers */
  Array_t      *sebs_p,                     /* 1-d word, SE brothers */
  Array_t      *uniqparity_p,               /* 1-d byte, parity indicators */
  Array_t      *uniqasymmetric_p,           /* 1-d bit, asymmetric flags */
  Array_t      *uniqmap_p,                  /* 1-d word, orig -> uniq map */
  Boolean_t    *chiral_p,                   /* Molecule chirality flag */
  U16_t         long_atoms,                 /* Number atoms overall */
  U16_t        *num_carbons_p               /* Number carbon atoms */
  )
{
  Tsd_t        *tsd_p;                      /* Local copy, compressed */
  Tsd_t        *tsd_unique_p;               /* Output unique TSD */
  Tsd_t        *tsd_final_p;                /* "Final" TSD */
  CodedTsd_t   *tsdlisthead_p;              /* Head of coded TSD list */
  CodedTsd_t   *codedtsd_tmp;               /* Current coded TSD */
  CodedTsd_t   *codedtsd_p;                 /* For list traversal */
  AtomArray_t  *atmarr_p;                   /* For debugging only */
  U16_t         atom;                       /* Counter */
  U16_t         short_atoms;                /* # atoms in short TSD */
  U16_t         num_single;                 /* # single bonds */
  U16_t         num_double;                 /* # double bonds */
  U16_t         num_triple;                 /* # triple bonds */
  U16_t         num_resonant;               /* # resonant bonds */
  U16_t         bond_weight;                /* Temporary */
  U16_t         neighid;                    /* Temporary */
  U16_t         atomid;                     /* Temporary */
  U16_t         first_tie;                  /* Atom index of first tie */
  U16_t         first_idx;                  /* Atom index related to tie */
  U16_t         i, j, k;                    /* Counter */
  U16_t         asival;                     /* ASI value */
  U16_t         index1, index2;             /* Temporary */
  S16_t         index3, index4;             /* Temporary */
  U8_t          neigh;                      /* Counter */
  U8_t          tsdneigh;                      /* Counter */
  Boolean_t     all_unique;                 /* Flag for when all done */
  Boolean_t     sorted;                     /* Flag for sorting ASI */
  Array_t       asymmetry_vec;              /* 1-d bit, asymmetric flags */
  Array_t       new_asymmetry_vec;          /* 1-d bit, new asymm. flags */
  Array_t       finalparity;                /* 1-d byte, parity indicators */
  Array_t       parity_vecs;                /* 2-d byte, parity vectors */
  Array_t       old_asi;                    /* 1-d word, original ASI */
  Array_t       min_asi;                    /* 1-d word, minimum ASI */
  Array_t       unique;                     /* 1-d bit, is ASI unique flags */
  Array_t       neighweight;                /* 2-d word, neigh. bond weight */
  Array_t       bondweight;                 /* 1-d word, was TSD[2] */
  Array_t       init_asi;                   /* 1-d word, initial asi */
  Array_t       init_tsdindex;              /* 1-d word, initial TSD index */
  Array_t       latest_tsdindex;            /* 1-d word, most recnt tsdindex */
  Array_t       latest_asi;                 /* 1-d word, most recent asi */
  Array_t       tsdcoded;                   /* 1-d word, tsd list
    description */

#ifdef _DEBUG_
printf("STsdUnique entered\n");
#endif
  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Entering STsdUnique,"
    " tsd = %p, cebs = %p, sebs = %p, parity = %p, asymmetry = %p, map = %p,"
    " chiral = %u, atoms = %u", shorttsd_p, cebs_p, sebs_p, uniqparity_p,
    uniqasymmetric_p, uniqmap_p, *chiral_p, long_atoms));

  *parityvector_pp = NULL;
  *chiral_p = TRUE;

  tsdlisthead_p = NULL;
  short_atoms = Tsd_NumAtoms_Get (shorttsd_p);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&asymmetry_vec", "sling{11}", &asymmetry_vec, short_atoms, BITSIZE);
#else
  Array_1d_Create (&asymmetry_vec, short_atoms, BITSIZE);
#endif
  Array_Set (&asymmetry_vec, FALSE);
#ifdef _DEBUG_
printf("STU1\n");
#endif

/*
  tsd_p = Tsd_Copy (shorttsd_p);
*/

  tsd_p = Tsd_Create (short_atoms);

  
  /* Compute the bond weight for each atom */

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&bondweight", "sling{11}", &bondweight, short_atoms, WORDSIZE);
#else
  Array_1d_Create (&bondweight, short_atoms, WORDSIZE);
#endif
#ifdef _DEBUG_
printf("STU2\n");
#endif
  for (atom = 0; atom < short_atoms; atom++)
    {

    Tsd_Atomid_Put (tsd_p, atom, Tsd_Atomid_Get (shorttsd_p, atom));

    for (tsdneigh = 0,neigh = 0, num_single = 0, num_double = 0, num_triple = 0,
         num_resonant = 0, neighid = TSD_INVALID; neigh < MX_NEIGHBORS;
         neigh++)
      {
      switch (Tsd_Atom_NeighborBond_Get (/*tsd_p */shorttsd_p, atom, neigh))
        {
        case BOND_SINGLE:

          num_single++;
/*
          if (neighid != TSD_INVALID)
            {
            STsdSwap (tsd_p, atom, neighid, neigh);
            neighid = TSD_INVALID;
            }
*/
          break;

        case BOND_DOUBLE:

          num_double++;
/*
          if (neighid != TSD_INVALID)
            {
            STsdSwap (tsd_p, atom, neighid, neigh);
            neighid = TSD_INVALID;
            }
*/
          break;

        case BOND_TRIPLE:

          num_triple++;
/*
          if (neighid != TSD_INVALID)
            {
            STsdSwap (tsd_p, atom, neighid, neigh);
            neighid = TSD_INVALID;
            }
*/
          break;

        case BOND_RESONANT:

          num_resonant++;
/*
          if (neighid != TSD_INVALID)
            {
            STsdSwap (tsd_p, atom, neighid, neigh);
            neighid = TSD_INVALID;
            }
*/
          break;

        default:
/*
          if (neighid == TSD_INVALID)
            neighid = neigh;
*/
          break;
        }
        if (Tsd_Atom_NeighborId_Get (shorttsd_p, atom, neigh) != TSD_INVALID)
          {
          Tsd_Atom_NeighborId_Put (tsd_p, atom, tsdneigh,  
            Tsd_Atom_NeighborId_Get (shorttsd_p, atom, neigh));
          ++tsdneigh;
          }
      }

    Array_1d16_Put (&bondweight, atom, num_single + num_double * 10 +
      num_triple * 100 + num_resonant * 1000);
    }
#ifdef _DEBUG_
printf("STU3\n");
#endif

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&init_tsdindex", "sling{11}", &init_tsdindex, short_atoms, WORDSIZE);
  mind_Array_1d_Create ("&init_asi", "sling{11}", &init_asi, short_atoms, WORDSIZE);
  mind_Array_1d_Create ("&old_asi", "sling{11}", &old_asi, short_atoms, WORDSIZE);
  mind_Array_1d_Create ("&unique", "sling{11}", &unique, short_atoms, BITSIZE);
  mind_Array_1d_Create ("&finalparity", "sling{11}", &finalparity, short_atoms, BYTESIZE);
  mind_Array_2d_Create ("&neighweight", "sling{11}", &neighweight, short_atoms, MX_NEIGHBORS, WORDSIZE);
#else
  Array_1d_Create (&init_tsdindex, short_atoms, WORDSIZE);
  Array_1d_Create (&init_asi, short_atoms, WORDSIZE);
  Array_1d_Create (&old_asi, short_atoms, WORDSIZE);
  Array_1d_Create (&unique, short_atoms, BITSIZE);
  Array_1d_Create (&finalparity, short_atoms, BYTESIZE);
  Array_2d_Create (&neighweight, short_atoms, MX_NEIGHBORS, WORDSIZE);
#endif

  Array_Set (&init_asi, TSD_INVALID);
  Array_Set (&old_asi, TSD_INVALID);
  Array_Set (&finalparity, PARITY_INIT);

#ifdef _DEBUG_
printf("STU4\n");
#endif
  /* init_tsdindex: holds the index into tsd where this atom appears
     finalparity:   holds the latest parity assignment
     init_asi:      holds the latest value of the asi of an atom
     old_asi:       holds the asi obtained earlier
     neighweight:   the sorted asi's of all neighbors of an atom
     unique:        indicates whether the asi of the atom is unique by a '1'b
  */

  /* Sort the atoms according to their bond weights */

  for (atom = 0; atom < short_atoms; atom++)
    Array_1d16_Put (&init_tsdindex, atom, atom);

  for (sorted = FALSE, j = short_atoms; !sorted; j--)
    for (atom = 1, sorted = TRUE; atom < j; atom++)
      if ((Tsd_Atomid_Get (tsd_p, Array_1d16_Get (&init_tsdindex, atom)) <
          Tsd_Atomid_Get (tsd_p, Array_1d16_Get (&init_tsdindex, atom - 1))) ||
          (Tsd_Atomid_Get (tsd_p, Array_1d16_Get (&init_tsdindex, atom)) ==
          Tsd_Atomid_Get (tsd_p, Array_1d16_Get (&init_tsdindex, atom - 1)) &&
          Array_1d16_Get (&bondweight, Array_1d16_Get (&init_tsdindex, atom)) <
          Array_1d16_Get (&bondweight, Array_1d16_Get (&init_tsdindex,
          atom - 1))))
        {
        SSwap (Array_1d16_Addr (&init_tsdindex, atom),
          Array_1d16_Addr (&init_tsdindex, atom - 1));
        sorted = FALSE;
        }

  /* Assign the initial asi's according to atoms and their bond weights. */

#ifdef _DEBUG_
printf("STU5\n");
#endif
  Array_1d16_Put (&init_asi, Array_1d16_Get (&init_tsdindex, 0), 0);
  for (atom = 1, asival = 0; atom < short_atoms; atom++)
    {
    index1 = Array_1d16_Get (&init_tsdindex, atom);
    index2 = Array_1d16_Get (&init_tsdindex, atom - 1);
    if ((Array_1d16_Get (&bondweight, index1) > Array_1d16_Get (&bondweight,
        index2)) || (Tsd_Atomid_Get (tsd_p, index1) > Tsd_Atomid_Get (tsd_p,
        index2)))
      asival = atom;

    Array_1d16_Put (&init_asi, index1, asival);
    }

#ifdef _DEBUG_
printf("STU6\n");
#endif
  SUniqueCompare (&init_asi, &init_tsdindex, &unique);
#ifdef _DEBUG_
printf("STU7\n");
#endif

  TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_SLING, "Initial ASI assignment:");
    SAsiDump (&init_asi, &init_tsdindex, &unique);
    });

  /* Count carbons and record all potentially asymmetric ones */

  for (atom = 0, *num_carbons_p = 0; atom < short_atoms; atom++)
    {
    atomid = Tsd_Atomid_Get (tsd_p, atom) / CANON_ATOMID_PERM;
    if (atomid == CANON_CARBON)
      {
      (*num_carbons_p)++;
      bond_weight = Array_1d16_Get (&bondweight, atom);
      if (bond_weight == CANON_TRIVALENT || bond_weight == CANON_TETRAVALENT)
        Array_1d1_Put (&asymmetry_vec, atom, TRUE);
      }
    }

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&new_asymmetry_vec", "sling{11}", &new_asymmetry_vec, short_atoms, BITSIZE);
#else
  Array_1d_Create (&new_asymmetry_vec, short_atoms, BITSIZE);
#endif
#ifdef _DEBUG_
printf("STU8\n");
#endif

  SDifferentiator (&init_asi, &init_tsdindex, &all_unique, &unique, shorttsd_p, 
    &old_asi, &neighweight);
#ifdef _DEBUG_
printf("STU9\n");
#endif

  TIMETRACE (R_SLING, DB_SLINGSTATIC, TL_TRACE, (outbuf,
    "Check point #10, in Sling_Canonical_Generate"));

  /* If all ties broken, form tsd, cebs, sebs */

  if (all_unique)
    {
    DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf,
      "TSD has no CEBs or SEBs (all nodes are unique)"));
 
    tsd_final_p = Tsd_Create (short_atoms);
    SCompute (&init_asi, &init_tsdindex, shorttsd_p, tsd_final_p,
      &finalparity, tsd_p, &bondweight, *num_carbons_p);
#ifdef _DEBUG_
printf("STU9.1\n");
#endif

    Array_Set (&new_asymmetry_vec, TRUE);

    for (atom = 0; atom < *num_carbons_p; atom++)
      if (Array_1d8_Get (&finalparity, atom) == PARITY_INIT)
        Array_1d1_Put (&new_asymmetry_vec, atom, FALSE);
#ifdef _DEBUG_
printf("STU9.2\n");
#endif

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING, "The final TSD:");
      STsdDump (tsd_final_p, &GTraceFile);
      });

#ifdef _MIND_MEM_
    mind_Array_1d_Create ("cebs_p", "sling{11}", cebs_p, short_atoms, WORDSIZE);
    mind_Array_1d_Create ("sebs_p", "sling{11}", sebs_p, short_atoms, WORDSIZE);
    mind_Array_1d_Create ("uniqmap_p", "sling{11}", uniqmap_p, short_atoms, WORDSIZE);
#else
    Array_1d_Create (cebs_p, short_atoms, WORDSIZE);
    Array_1d_Create (sebs_p, short_atoms, WORDSIZE);
    Array_1d_Create (uniqmap_p, short_atoms, WORDSIZE);
#endif
    Array_CopyContents (&init_asi, uniqmap_p);
#ifdef _DEBUG_
printf("STU10\n");
#endif

    if (*num_carbons_p != 0)
      {
      atmarr_p = AtmArr_Create (*num_carbons_p, ATMARR_NOBONDS_BYTE, FALSE);
      for (atom = 0; atom < *num_carbons_p; atom++)
        Array_1d8_Put (AtmArr_Array_Get (atmarr_p), atom, Array_1d8_Get (
          &finalparity, atom));

      *parityvector_pp = atmarr_p;
      }

    for (atom = 0; atom < short_atoms; atom++)
      {
      Array_1d16_Put (cebs_p, atom, atom);
      Array_1d16_Put (sebs_p, atom, atom);
      }
    }                      /* End if-all_unique */
  else                     /* not all nodes are unique */
    {
    DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf,
      "Not all nodes are unique at this point"));

#ifdef _MIND_MEM_
    mind_Array_1d_Create ("&tsdcoded", "sling{11}", &tsdcoded, short_atoms, WORDSIZE);
    Array_Set (&tsdcoded, 0);

    if (*num_carbons_p != 0)
      {
      mind_Array_2d_Create ("&parity_vecs", "sling{11}", &parity_vecs, short_atoms, *num_carbons_p, BYTESIZE);
      Array_Set (&parity_vecs, 10240 /*PARITY_INIT*/);
      }
    else
      FILL (parity_vecs, 0);

    mind_Array_1d_Create ("&latest_asi", "sling{11}", &latest_asi, short_atoms, WORDSIZE);
    mind_Array_1d_Create ("&latest_tsdindex", "sling{11}", &latest_tsdindex, short_atoms, WORDSIZE);
    mind_Array_1d_Create ("&min_asi", "sling{11}", &min_asi, short_atoms, WORDSIZE);
#else
    Array_1d_Create (&tsdcoded, short_atoms, WORDSIZE);
    Array_Set (&tsdcoded, 0);

    if (*num_carbons_p != 0)
      {
      Array_2d_Create (&parity_vecs, short_atoms, *num_carbons_p, BYTESIZE);
      Array_Set (&parity_vecs, 10240 /*PARITY_INIT*/);
      }
    else
      FILL (parity_vecs, 0);

    Array_1d_Create (&latest_asi, short_atoms, WORDSIZE);
    Array_1d_Create (&latest_tsdindex, short_atoms, WORDSIZE);
    Array_1d_Create (&min_asi, short_atoms, WORDSIZE);
#endif

    Array_Set (&min_asi, CANON_MINASI_INIT);
#ifdef _DEBUG_
printf("STU11\n");
#endif

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING, "The starting ASI assignment");
      SAsiDump (&init_asi, &init_tsdindex, &unique);
      });

    for (atom = 0, first_tie = TSD_INVALID; atom < short_atoms; atom++)
      {
      if (atom)
        index1 = Array_1d16_Get (&init_tsdindex, atom - 1);
      else
        index1 = TSD_INVALID;

      index2 = Array_1d16_Get (&init_tsdindex, atom);
      if (atom != short_atoms - 1)
        index3 = Array_1d16_Get (&init_tsdindex, atom + 1);
      else
        index3 = TSD_INVALID;

      DEBUG (R_SLING, DB_SLINGSTATIC, TL_LOOP, (outbuf, 
        "Atom loop : atom = %u, index1 = %u, index2 = %u, index3 = %d,"
        " first tie = %u", atom, index1, index2, index3, first_tie));

      if (((index1 != TSD_INVALID) && (Array_1d16_Get (&init_asi, index1) ==
          Array_1d16_Get (&init_asi, index2))) || ((index3 !=
          (S16_t)TSD_INVALID) && (Array_1d16_Get (&init_asi, index3) ==
          Array_1d16_Get (&init_asi, index2))))
        {
        if (first_tie == TSD_INVALID)
          first_tie = Array_1d16_Get (&init_asi, index2);

        if (Array_1d16_Get (&init_asi, index2) == first_tie)
          {
          Array_CopyContents (&init_asi, &latest_asi);
          Array_CopyContents (&init_tsdindex, &latest_tsdindex);

          SBreak (&latest_asi, &latest_tsdindex, atom);
          SDifferentiator (&latest_asi, &latest_tsdindex, &all_unique,
            &unique, shorttsd_p, &old_asi, &neighweight);

          for (i = 0, first_idx = short_atoms + 1; i < short_atoms &&
               first_idx == short_atoms + 1; i++)
            if (i != (short_atoms - 1) && short_atoms != 1)
              if (Array_1d16_Get (&latest_asi, Array_1d16_Get (
                  &latest_tsdindex, i)) == Array_1d16_Get (&latest_asi,
                  Array_1d16_Get (&latest_tsdindex, i + 1)))
                first_idx = i;

          STieBreaker (&latest_asi, &latest_tsdindex, first_idx, chiral_p,
            &all_unique, &unique, shorttsd_p, &old_asi, &neighweight,
            &finalparity, &min_asi, &asymmetry_vec, &parity_vecs,
            &tsdlisthead_p, &tsdcoded, tsd_p, &bondweight);
          }
        }
      }                                /* End for-atom loop */

    TIMETRACE (R_SLING, DB_SLINGSTATIC, TL_TRACE, (outbuf,
      "Check point #13, in Sling_Canonical_Generate"));
#ifdef _DEBUG_
printf("STU12\n");
#endif

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING, "Coded TSD Index:");
      Array_Dump (&tsdcoded, &GTraceFile);
      IO_Put_Trace (R_SLING, "Parity vectors:");
      Array_Dump (&parity_vecs, &GTraceFile);
      IO_Put_Trace (R_SLING, "Minimum ASI:");
      Array_Dump (&min_asi, &GTraceFile);
      IO_Put_Trace (R_SLING, "Initial TSD index:");
      Array_Dump (&init_tsdindex, &GTraceFile);
      });

    /* Sort the Coded Tsd's */

    for (atom = 0; atom < short_atoms - 1; atom++)
      {
      for (j = atom + 1, i = atom; j < short_atoms; j++)
        {
        index1 = Array_1d16_Get (&init_tsdindex, j);
        index2 = Array_1d16_Get (&init_tsdindex, i);
        if (Array_1d16_Get (&min_asi, index1) <= Array_1d16_Get (&min_asi,
            index2) && (Array_1d16_Get (&min_asi, index1) < Array_1d16_Get (
            &min_asi, index2) || Array_1d16_Get (&tsdcoded, index1) <
            Array_1d16_Get (&tsdcoded, index2)))
          i = j;
        }

      if (i != atom)
        SSwap (Array_1d16_Addr (&init_tsdindex, atom), Array_1d16_Addr (
          &init_tsdindex, i));
      }
#ifdef _DEBUG_
printf("STU13\n");
#endif

    /* Assign the ceb's */

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING, "Initial TSD index, after sorting:");
      Array_Dump (&init_tsdindex, &GTraceFile);
      });

#ifdef _MIND_MEM_
    mind_Array_1d_Create ("cebs_p", "sling{11a}", cebs_p, short_atoms, WORDSIZE);
#else
    Array_1d_Create (cebs_p, short_atoms, WORDSIZE);
#endif

    Array_1d16_Put (cebs_p, Array_1d16_Get (&init_tsdindex, 0), 0);
    for (atom = 0; atom < short_atoms - 1; atom++)
      {
      index1 = Array_1d16_Get (&init_tsdindex, atom);
      index2 = Array_1d16_Get (&init_tsdindex, atom + 1);
      if (Array_1d16_Get (&min_asi, index2) == Array_1d16_Get (&min_asi,
          index1) && Array_1d16_Get (&tsdcoded, index1) == Array_1d16_Get (
          &tsdcoded, index2))
        {
        Array_1d16_Put (cebs_p, index2, Array_1d16_Get (cebs_p, index1));
        }
      else
        Array_1d16_Put (cebs_p, index2, atom + 1);
      }
#ifdef _DEBUG_
printf("STU14\n");
#endif

    /* Sort the parity vectors among the ceb's */

    for (atom = 0; atom < short_atoms - 1; atom++)
      {
      for (j = atom + 1, index3 = 0, i = atom; j < short_atoms &&
           index3 <= 0; j++)
        {
        index1 = Array_1d16_Get (&init_tsdindex, i);
        index2 = Array_1d16_Get (&init_tsdindex, j);
        index3 = (S16_t)Array_1d16_Get (cebs_p, index2) -
          (S16_t)Array_1d16_Get (cebs_p, index1);

        if (index3 < 0)
          i = j;
        else
          {
          for (k = 0, index4 = 0; k < *num_carbons_p && index4 == 0 /*&&
               index3 == 0*/; k++)
            {
            index4 = (S8_t)Array_2d8_Get (&parity_vecs, index2, k) -
              (S8_t)Array_2d8_Get (&parity_vecs, index1, k);

            if (index4 < 0)
              i = j;
            }
          }
        }

      if (i != atom)
        SSwap (Array_1d16_Addr (&init_tsdindex, atom), Array_1d16_Addr (
          &init_tsdindex, i));
      }
#ifdef _DEBUG_
printf("STU15\n");
#endif

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING,
        "Initial TSD index, after adjusting parity vectors");
      Array_Dump (&init_tsdindex, &GTraceFile);
      });

    TIMETRACE (R_SLING, DB_SLINGSTATIC, TL_TRACE, (outbuf,
      "Check point #14, in Sling_Canonical_Generate"));

    /* Assign the seb's */

#ifdef _MIND_MEM_
    mind_Array_1d_Create ("sebs_p", "sling{11a}", sebs_p, short_atoms, WORDSIZE);
#else
    Array_1d_Create (sebs_p, short_atoms, WORDSIZE);
#endif

    Array_1d16_Put (sebs_p, Array_1d16_Get (&init_tsdindex, 0), 0);
    for (atom = 0; atom < short_atoms - 1; atom++)
      {
      index1 = Array_1d16_Get (&init_tsdindex, atom);
      index2 = Array_1d16_Get (&init_tsdindex, atom + 1);
      if (Array_1d16_Get (cebs_p, index2) == Array_1d16_Get (cebs_p,
          index1))
        {
        for (k = 0, index3 = (S16_t)TSD_INVALID; k < *num_carbons_p &&
             index3 == (S16_t)TSD_INVALID; k++)
          if (Array_2d8_Get (&parity_vecs, index2, k) != Array_2d8_Get (
              &parity_vecs, index1, k))
            {
            Array_1d16_Put (sebs_p, index2, atom + 1);
            index3 = atom;
            }

        if (index3 == (S16_t)TSD_INVALID)
          Array_1d16_Put (sebs_p, index2, Array_1d16_Get (sebs_p, index1));
        }
      else
        Array_1d16_Put (sebs_p, index2, atom + 1);
      }
#ifdef _DEBUG_
printf("STU16\n");
#endif

    for (atom = 0, index3 = -1; atom < short_atoms && index3 == -1; atom++)
      if (Array_1d16_Get (&tsdcoded, Array_1d16_Get (&init_tsdindex, atom)) !=
          0)
        index3 = atom;

    if (index3 == -1)
      IO_Exit_Error (R_SLING, X_SYNERR, 
        "Error in STsdUnique, all tsdcoded elements are zero.");

    for (i = 1, codedtsd_tmp = tsdlisthead_p; i < Array_1d16_Get (&tsdcoded,
         Array_1d16_Get (&init_tsdindex, index3)); codedtsd_tmp =
         CodeTsd_Next_Get (codedtsd_tmp))
      /* Empty loop body */ ;

    tsd_final_p = CodeTsd_Tsd_Get (codedtsd_tmp);
    CodeTsd_Tsd_Put (codedtsd_tmp, NULL);
#ifdef _DEBUG_
printf("STU17\n");
#endif

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      TRACE (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf, "Final TSD and"
        " parity vector, index = %d, # carbons = %u", index3, *num_carbons_p));

      STsdDump (tsd_final_p, &GTraceFile);
      for (i = 0; i < Array_Columns_Get (&parity_vecs); i++)
        fprintf (IO_FileHandle_Get (&GTraceFile), "%5d",
          (S8_t)Array_2d8_Get (&parity_vecs,
          Array_1d16_Get (&init_tsdindex, index3), i));

      fprintf (IO_FileHandle_Get (&GTraceFile), "\n");
      });

    /* Use Coded TSD map to get ceb, seb vectors, need to use latest*
       arrays as temporary spots for rearranging the C/SE brothers
    */

#ifdef _MIND_MEM_
    mind_Array_1d_Create ("uniqmap_p", "sling{11a}", uniqmap_p, short_atoms, WORDSIZE);
#else
    Array_1d_Create (uniqmap_p, short_atoms, WORDSIZE);
#endif
    Array_Set (uniqmap_p, TSD_INVALID);
    Array_Set (&finalparity, PARITY_INIT);
    Array_CopyContents (cebs_p, &latest_asi);
    Array_CopyContents (sebs_p, &latest_tsdindex);
#ifdef _DEBUG_
printf("STU18\n");
#endif

    for (atom = 0; atom < short_atoms; atom++)
      {
      j = Array_1d16_Get (CodeTsd_Map_Get (codedtsd_tmp), atom);
      Array_1d16_Put (uniqmap_p, j, atom);
      Array_1d1_Put (&new_asymmetry_vec, atom, Array_1d1_Get (&asymmetry_vec,
        j));
      Array_1d16_Put (cebs_p, atom, Array_1d16_Get (&latest_asi, j));
      Array_1d16_Put (sebs_p, atom, Array_1d16_Get (&latest_tsdindex, j));
      if (atom < *num_carbons_p)
        Array_1d8_Put (&finalparity, atom, Array_2d8_Get (&parity_vecs,
          Array_1d16_Get (&init_tsdindex, index3), atom));
      }

    TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
      {
      IO_Put_Trace (R_SLING, "Final CEB class, SEB class and TSD map");
      Array_Dump (cebs_p, &GTraceFile);
      Array_Dump (sebs_p, &GTraceFile);
      Array_Dump (uniqmap_p, &GTraceFile);
      });

#ifdef _MIND_MEM_
    mind_Array_Destroy ("&tsdcoded", "sling", &tsdcoded);
    mind_Array_Destroy ("&latest_asi", "sling", &latest_asi);
    mind_Array_Destroy ("&latest_tsdindex", "sling", &latest_tsdindex);
    mind_Array_Destroy ("&min_asi", "sling", &min_asi);

    if (*num_carbons_p)
      mind_Array_Destroy ("&parity_vecs", "sling", &parity_vecs);
#else
    Array_Destroy (&tsdcoded);
    Array_Destroy (&latest_asi);
    Array_Destroy (&latest_tsdindex);
    Array_Destroy (&min_asi);

    if (*num_carbons_p)
      Array_Destroy (&parity_vecs);
#endif
#ifdef _DEBUG_
printf("STU19\n");
#endif

    DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf, 
      "The list of Coded TSDs, their mapping and their associated parity"
      " vectors, # carbon atoms = %u", *num_carbons_p));

    for (codedtsd_tmp = tsdlisthead_p; codedtsd_tmp != NULL; codedtsd_tmp =
         codedtsd_p)
      {
      TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
        {
        if (CodeTsd_Tsd_Get (codedtsd_tmp) == NULL)
          STsdDump (tsd_final_p, &GTraceFile);
        else
          STsdDump (CodeTsd_Tsd_Get (codedtsd_tmp), &GTraceFile);
        Array_Dump (CodeTsd_Map_Get (codedtsd_tmp), &GTraceFile);
        });

      for (atmarr_p = CodeTsd_Parity_Get (codedtsd_tmp); atmarr_p != NULL; )
        {
        TRACE_DO (DB_SLINGSTATIC, TL_MAJOR,
          {
          Array_Dump (AtmArr_Array_Get (atmarr_p), &GTraceFile);
          });

        if (codedtsd_tmp != tsdlisthead_p)
          {
          CodeTsd_Parity_Put (codedtsd_tmp, AtmArr_Next_Get (atmarr_p));
          AtmArr_Destroy (atmarr_p);
          atmarr_p = CodeTsd_Parity_Get (codedtsd_tmp);
          }
        else
          {
          *parityvector_pp = CodeTsd_Parity_Get (codedtsd_tmp);
          atmarr_p = AtmArr_Next_Get (atmarr_p);
          }
        }

      codedtsd_p = CodeTsd_Next_Get (codedtsd_tmp);
      Tsd_Destroy (CodeTsd_Tsd_Get (codedtsd_tmp));
#ifdef _MIND_MEM_
      mind_Array_Destroy ("CodeTsd_Map_Get(codedtsd_tmp)", "sling", CodeTsd_Map_Get (codedtsd_tmp));
      mind_Array_Destroy ("CodeTsd_BondWeight_Get(codedtsd_tmp)", "sling", CodeTsd_BondWeight_Get (codedtsd_tmp));
      mind_free ("codedtsd_tmp", "sling", codedtsd_tmp);
#else
      Array_Destroy (CodeTsd_Map_Get (codedtsd_tmp));
      Array_Destroy (CodeTsd_BondWeight_Get (codedtsd_tmp));
      Mem_Dealloc (codedtsd_tmp, CODEDTSDSIZE, GLOBAL);
#endif
#ifdef _DEBUG_
printf("STU20\n");
#endif

      DEBUG (R_SLING, DB_SLINGSTATIC, TL_MEMORY, (outbuf,
        "Deallocated memory for a Coded TSD in STsdUnique at %p",
        codedtsd_tmp));
      }

    tsdlisthead_p = NULL;
    }                                   /* End of else-!all_unique */

  TIMETRACE (R_SLING, DB_SLINGSTATIC, TL_TRACE, (outbuf,
    "Check point #15, in Sling_Canonical_Generate"));

  tsd_unique_p = SExpand (tsd_final_p);
#ifdef _DEBUG_
printf("STU21\n");
#endif

  TIMETRACE (R_SLING, DB_SLINGSTATIC, TL_TRACE, (outbuf,
    "Check point #16, in Sling_Canonical_Generate"));

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_MAJOR, (outbuf, "Number of TSDs generated\
 during canonical search = not calculated anymore"));

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("uniqasymmetric_p", "sling{11}", uniqasymmetric_p, long_atoms, BITSIZE);
  mind_Array_1d_Create ("uniqparity_p", "sling{11}", uniqparity_p, *num_carbons_p, BYTESIZE);
#else
  Array_1d_Create (uniqasymmetric_p, long_atoms, BITSIZE);
  Array_1d_Create (uniqparity_p, *num_carbons_p, BYTESIZE);
#endif
#ifdef _DEBUG_
printf("STU22\n");
#endif

  for (atom = 0; atom < Array_Columns_Get (&new_asymmetry_vec); atom++)
    Array_1d1_Put (uniqasymmetric_p, atom, Array_1d1_Get (&new_asymmetry_vec,
      atom));

  for (atom = 0; atom < *num_carbons_p; atom++)
    Array_1d8_Put (uniqparity_p, atom, Array_1d8_Get (&finalparity, atom));

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bondweight", "sling", &bondweight);
  mind_Array_Destroy ("&finalparity", "sling", &finalparity);
  mind_Array_Destroy ("&asymmetry_vec", "sling", &asymmetry_vec);
  mind_Array_Destroy ("&new_asymmetry_vec", "sling", &new_asymmetry_vec);
  mind_Array_Destroy ("&init_asi", "sling", &init_asi);
  mind_Array_Destroy ("&init_tsdindex", "sling", &init_tsdindex);
  mind_Array_Destroy ("&old_asi", "sling", &old_asi);
  mind_Array_Destroy ("&unique", "sling", &unique);
  mind_Array_Destroy ("&neighweight", "sling", &neighweight);
#else
  Array_Destroy (&bondweight);
  Array_Destroy (&finalparity);
  Array_Destroy (&asymmetry_vec);
  Array_Destroy (&new_asymmetry_vec);
  Array_Destroy (&init_asi);
  Array_Destroy (&init_tsdindex);
  Array_Destroy (&old_asi);
  Array_Destroy (&unique);
  Array_Destroy (&neighweight);
#endif
  Tsd_Destroy (tsd_p);
  Tsd_Destroy (tsd_final_p);
#ifdef _DEBUG_
printf("STU23\n");
#endif

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving STsdUnique, unique TSD = %p", tsd_unique_p));

  return tsd_unique_p;
}

/****************************************************************************
*
*  Function Name:                 SUniqueCompare
*
*    This routine checks to see if the atoms are unique, by comparing them
*    against their neighbors, if they are different than both their neighbors,
*    they get marked as unique.
*
*  Used to be:
*
*    unicomp:
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static void SUniqueCompare
  (
  Array_t      *asi_p,                      /* 1-d word, ASI */
  Array_t      *tsdindex_p,                 /* 1-d word, sorted -> compact*/
  Array_t      *unique_p                    /* 1-d bit, unique flags */
  )
{
  U16_t         short_atoms;                /* # atoms in compact m'cule */
  U16_t         atom;                       /* Counter */
  U16_t         index;                      /* Temporary */

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf, "Entering\
 SUniqueCompare, asi = %p, tsdindex = %p, unique = %p", asi_p, tsdindex_p,
 unique_p));

  Array_Set (unique_p, FALSE);

  short_atoms = (U16_t) Array_Columns_Get (asi_p);
  if (short_atoms < 2)
    {
    Array_1d1_Put (unique_p, 0, TRUE);

    DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
      "Leaving SUniqueCompare, status = <void>"));

    return;
    }

  if (Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, 0)) < Array_1d16_Get (
      asi_p, Array_1d16_Get (tsdindex_p, 1)))
    Array_1d1_Put (unique_p, Array_1d16_Get (tsdindex_p, 0), TRUE);

  if (Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, short_atoms - 1)) >
      Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, short_atoms - 2)))
    Array_1d1_Put (unique_p, Array_1d16_Get (tsdindex_p, short_atoms - 1),
      TRUE);

  if (short_atoms < 3)
    {
    DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
      "Leaving SUniqueCompare, status = <void>"));

    return;
    }

  for (atom = 1; atom < short_atoms - 1; atom++)
    {
    index = Array_1d16_Get (tsdindex_p, atom);
    if (Array_1d16_Get (asi_p, index) != Array_1d16_Get (asi_p,
        Array_1d16_Get (tsdindex_p, atom - 1)) && Array_1d16_Get (asi_p, index)
        != Array_1d16_Get (asi_p, Array_1d16_Get (tsdindex_p, atom + 1)))
      Array_1d1_Put (unique_p, index, TRUE);
    }

  DEBUG (R_SLING, DB_SLINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SUniqueCompare, status = <void>"));

  return;
}
/* End of SLING.C */
