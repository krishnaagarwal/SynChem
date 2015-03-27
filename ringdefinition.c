/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RINGDEFINITION.C
*
*    This module is the abstraction for the ring definition.
*
*  Routines:
*
*    Ringdef_Carbocyclic_Find
*    Ringdef_CommonRing_Find
*    Ringdef_Copy
*    Ringdef_Create
*    Ringdef_CycleMember_Set
*    Ringdef_CycleSize_Find
*    Ringdef_Destroy
*    Ringdef_Dump
*    Ringdef_MinPrimaryRing_Find
*    Ringdef_Min4PrimaryRing_Find
*    Ringdef_NumPrimaryCycles_Find
*    Ringdef_PrimaryRingNodes_Get
*    Ringdef_PrimeCycle_Find
*    SAdd_Node
*    SBfstree_Build
*    SCommon_Ancestor
*    SCycle_Form
*    SCycles_Collect
*    SFundamental_Cycle
*    SGenerate
*    SInsert
*    SPath_Combine
*    SPathCopy
*    SPath_Destroy
*    SPath_Get
*    SPath_Initialize
*    SPrimeCycles_Find
*    SValid_Mark
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
* 10-Nov-96  Krebsbach  Vector should be based on the original Xtr, not the 
*                       subset xtr.  Moved vector set code to Xtr_Ringdef_Set 
*                       from Ringdef_PrimeCycles_Find.
* 22-May-95  Cheung     In Ringdef_NumPrimaryCycles_Find, added  - if ringdef_p
*			== NULL, call Xtr_Ringdef_Set
*
******************************************************************************/

#include <string.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_RINGDEFINITION_
#include "ringdefinition.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/* Static Routine Prototypes */

static void          SAdd_Node          (LinkedData_t *, U32_t, Boolean_t);
static void          SBfstree_Build     (LinkedData_t **, PrmCycleCB_t *);
static U16_t         SCommon_Ancestor   (U16_t, U16_t, Boolean_t, Array_t *);
static LinkedData_t *SCycle_Form        (LinkedData_t *, LinkedData_t *);
static void          SCycles_Collect    (LinkedData_t *, LinkedData_t **,
  PrmCycleCB_t *);
static LinkedData_t *SFundamental_Cycle (U16_t, U16_t, U16_t, Boolean_t *,
  Boolean_t *, Boolean_t *, Boolean_t *, Boolean_t *, Array_t *, Array_t *,
  PrmCycleCB_t *);
static void          SGenerate          (U16_t, U16_t, U16_t, LinkedData_t **,
  Boolean_t, PrmCycleCB_t *);
static void          SInsert            (LinkedData_t *, LinkedData_t **,
  LinkedData_t **);
static void          SPath_Combine      (LinkedData_t *, LinkedData_t *,
  LinkedData_t **);
static LinkedData_t *SPath_Copy         (LinkedData_t *);
static void          SPath_Destroy      (LinkedData_t *);
static void          SPath_Get          (LinkedData_t **, Boolean_t,
  LinkedData_t **, LinkedData_t **, LinkedData_t **, LinkedData_t **, U16_t,
  PrmCycleCB_t *);
static LinkedData_t *SPath_Initialize   (U16_t);
static void          SPrimeCycles_Find  (U16_t, PrmCycleCB_t *,
  LinkedData_t **, U16_t *);
static void          SValid_Mark        (LinkedData_t *, U16_t,
  PrmCycleCB_t *);


/****************************************************************************
*
*  Function Name:                 Ringdef_Carbocyclic_Find
*
*    This function looks up the specified ring in the specified ring
*    system and returns a flag indicating whether it is a carbocyclic ring
*    or a heterocyclic ring.
*
*  Used to be:
*
*    carbcyc:
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
*    Boolean_t from prime cycle vector
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Ringdef_Carbocyclic_Find
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx,           /* Which RingSystem to look in */
  U16_t            ring_idx               /* Which Ring to look up */
  )
{
  Ringdef_t       *ringdef_p;             /* Ring Defn. in question */
  AtomArray_t     *pcycle_tmp;            /* Prime cycle to look at */

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
  pcycle_tmp = Ringdef_PrimeCycle_Get (ringdef_p, ring_idx);

  return Ringdef_PrimeCycle_Carbocyclic_Get (pcycle_tmp);
}

/****************************************************************************
*
*  Function Name:                 Ringdef_CommonRing_Find
*
*    This routine fills in an array of flags which indicate which of the
*    many rings two atoms have in common.
*
*  Used to be:
*
*    copring:
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
void Ringdef_CommonRing_Find
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx,           /* Which RingSystem to look in */
  U16_t            atom1,                 /* One node in match */
  U16_t            atom2,                 /* Other node in match */
  Array_t         *flags_p                /* Results, (1-d bit array) */
  )
{
  Ringdef_t       *ringdef_p;             /* Ring Defn. for m'cule */
  Array_t         *vector_p;              /* Cycle vector for m'cule */
  U16_t            cycle;                 /* Counter */

  DEBUG (R_XTR, DB_RINGDEFCOMFIND, TL_PARAMS, (outbuf,
    "Entering Ringdef_CommonRing_Find, Xtr = %p, Ring System = %u,"
    " atom1 = %u, atom2 = %u, flag array = %p", xtr_p, ringsys_idx, atom1, 
    atom2, flags_p));

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
  vector_p = Ringdef_CycleVector_Get (ringdef_p);

  for (cycle = 0; cycle < Ringdef_NumCycles_Get (ringdef_p); cycle++)
    if (Ringdef_CycleVector_Node_Get (vector_p, atom1, cycle)
        && Ringdef_CycleVector_Node_Get (vector_p, atom2, cycle))
      Array_1d1_Put (flags_p, cycle, TRUE);
    else
      Array_1d1_Put (flags_p, cycle, FALSE);

  DEBUG (R_XTR, DB_RINGDEFCOMFIND, TL_PARAMS, (outbuf,
    "Leaving Ringdef_CommonRing_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_Copy
*
*    This function makes a copy of a Ring Definition.
*
*  Used to be:
*
*    nrdefn:
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
*    Address of copy of Ring Definition
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Ringdef_t *Ringdef_Copy
  (
  Ringdef_t    *ringdef_p                 /* Instance to copy */
  )
{
  Ringdef_t    *ringdef_tmp;              /* Temporary */
  U16_t         i;                        /* Counter */

  if (ringdef_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_RINGDEFCOPY, TL_PARAMS, (outbuf,
    "Entering Ringdef_Copy, Ring Def = %p", ringdef_p));

  ringdef_tmp = Ringdef_Create (Ringdef_NumCycles_Get (ringdef_p));

  Ringdef_CycleVector_Copy (Ringdef_CycleVector_Get (ringdef_p),
    Ringdef_CycleVector_Get (ringdef_tmp));

  for (i = 0; i < Ringdef_NumCycles_Get (ringdef_p); i++)
    {
    Ringdef_PrimeCycle_Put (ringdef_tmp, i, AtmArr_Copy (
      Ringdef_PrimeCycle_Get (ringdef_p, i)));
    }

  DEBUG (R_XTR, DB_RINGDEFCOPY, TL_PARAMS, (outbuf,
    "Leaving Ringdef_Copy, Ring Def = %p", ringdef_tmp));

  return ringdef_tmp;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_Create
*
*    This function creates a new XtrRingDefinition.
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
*    Address of newly created Ring Definition
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Ringdef_t *Ringdef_Create
  (
  U16_t          num_cycles                 /* Number of cycles */
  )
{
  Ringdef_t     *ringdef_tmp;               /* Temporary */
  AtomArray_t  **pcycle_tmp;                /* Temporary */

  DEBUG (R_XTR, DB_RINGDEFCREATE, TL_PARAMS, (outbuf,
    "Entering Ringdef_Create, # cycles = %u", num_cycles));

#ifdef _MIND_MEM_
  mind_malloc ("ringdef_tmp", "ringdefinition{1}", &ringdef_tmp, RINGDEFSIZE);
#else
  Mem_Alloc (Ringdef_t *, ringdef_tmp, RINGDEFSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGDEFCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Ring Definition in Ringdef_Create at %p",
    ringdef_tmp));

  if (ringdef_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for Ring Definition in Ringdef_Create");

#ifdef _MIND_MEM_
  mind_malloc ("pcycle_tmp", "ringdefinition{1}", &pcycle_tmp, num_cycles * sizeof (Address));
#else
  Mem_Alloc (AtomArray_t **, pcycle_tmp, num_cycles * sizeof (Address), GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGDEFCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for primary cycle address array in Ringdef_Create at %p",
    pcycle_tmp));

  if (pcycle_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for primary cycle array in Ringdef_Create");

  DEBUG_DO (DB_RINGDEFCREATE, TL_MEMORY,
    {
    memset (ringdef_tmp, 0, RINGDEFSIZE);
    memset (pcycle_tmp, 0, num_cycles * sizeof (Address));
    });

  Ringdef_NumCycles_Put (ringdef_tmp, num_cycles);
  Ringdef_PrimeCycleHandle_Put (ringdef_tmp, pcycle_tmp);

  DEBUG (R_XTR, DB_RINGDEFCREATE, TL_PARAMS, (outbuf,
    "Leaving Ringdef_Create, Ring Def. = %p", ringdef_tmp));

  return ringdef_tmp;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_CycleMember_Set
*
*    This routine fills in an array of flags indicating which primary rings
*    contain the specified atom.
*
*  Used to be:
*
*    prngset:
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
void Ringdef_CycleMember_Set
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx,           /* Which RingSystem to look in */
  U16_t            node,                  /* Which node number to search for */
  Array_t         *flags_p                /* 1-d bit, array for results */
  )
{
  Ringdef_t       *ringdef_p;             /* Ring Defn. to look in */
  Array_t         *vector_p;              /* Cycle vector to look in */
  U16_t            cycle;                 /* Counter */

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
  vector_p = Ringdef_CycleVector_Get (ringdef_p);

  for (cycle = 0; cycle < Ringdef_NumCycles_Get (ringdef_p); cycle++)
    Array_1d1_Put (flags_p, cycle, Ringdef_CycleVector_Node_Get (vector_p,
      node, cycle));

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_CycleSize_Find
*
*    This function returns the number of nodes in the specified ring of
*    the specified ring definition.
*
*  Used to be:
*
*    pringsz:
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
U16_t Ringdef_CycleSize_Find
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx,           /* Which RingSystem to look in */
  U16_t            ring_idx               /* Which Ring to look in */
  )
{
  Ringdef_t       *ringdef_p;             /* Temporary */
  AtomArray_t     *cycle_p;               /* Temporary */

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
  cycle_p = Ringdef_PrimeCycle_Get (ringdef_p, ring_idx);

  return Ringdef_PrimeCycle_NumNodes_Get (cycle_p);
}

/****************************************************************************
*
*  Function Name:                 Ringdef_Destroy
*
*    This function deallocates all the memory for an XtrRingDefinition.
*
*  Used to be:
*
*    zrdefn:
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
void Ringdef_Destroy
  (
  Ringdef_t     *ringdef_p                  /* Address of instance, destroy */
  )
{
  U16_t          i;                         /* Counter */

  if (ringdef_p == NULL)
    return;

  DEBUG (R_XTR, DB_RINGDEFDESTROY, TL_PARAMS, (outbuf,
    "Entering Ringdef_Destroy, Ring Def. = %p", ringdef_p));

  for (i = 0; i < Ringdef_NumCycles_Get (ringdef_p); i++)
    {
    AtmArr_Destroy (Ringdef_PrimeCycle_Get (ringdef_p, i));

    DEBUG_DO (DB_RINGDEFDESTROY, TL_MEMORY,
      {
      Ringdef_PrimeCycle_Put (ringdef_p, i, NULL);
      });
    }

#ifdef _MIND_MEM_
  mind_free ("Ringdef_PrimeCycleHandle_Get(ringdef_p)", "ringdefinition{2}", Ringdef_PrimeCycleHandle_Get (ringdef_p));

  DEBUG (R_XTR, DB_RINGDEFDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for primary cycle array in Ringdef_Destroy at %p",
    Ringdef_PrimeCycleHandle_Get (ringdef_p)));

  mind_Array_Destroy ("Ringdef_CycleVector_Get(ringdef_p)", "ringdefinition", Ringdef_CycleVector_Get (ringdef_p));

  DEBUG_DO (DB_RINGDEFDESTROY, TL_MEMORY,
    {
    memset (ringdef_p, 0, RINGDEFSIZE);
    });

  mind_free ("ringdef_p", "rindefinition", ringdef_p);
#else
  Mem_Dealloc (Ringdef_PrimeCycleHandle_Get (ringdef_p), sizeof (Address)
    * Ringdef_NumCycles_Get (ringdef_p), GLOBAL);

  DEBUG (R_XTR, DB_RINGDEFDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for primary cycle array in Ringdef_Destroy at %p",
    Ringdef_PrimeCycleHandle_Get (ringdef_p)));

  Array_Destroy (Ringdef_CycleVector_Get (ringdef_p));

  DEBUG_DO (DB_RINGDEFDESTROY, TL_MEMORY,
    {
    memset (ringdef_p, 0, RINGDEFSIZE);
    });

  Mem_Dealloc (ringdef_p, RINGDEFSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGDEFDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Ring Definition in Ringdef_Destroy at %p",
    ringdef_p));

  DEBUG (R_XTR, DB_RINGDEFDESTROY, TL_PARAMS, (outbuf,
    "Leaving Ringdef_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_Dump
*
*    This function prints a formatted dump of a ring definition.
*
*  Used to be:
*
*    prdefn:
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
void Ringdef_Dump
  (
  Ringdef_t     *ringdef_p,                  /* Instance to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */
  AtomArray_t   *pcycle_tmp;                 /* Temporary */
  U16_t          i, j;                       /* Counters */

  f = IO_FileHandle_Get (filed_p);
  if (ringdef_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Ring Definition\n\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_RINGDEFCREATE, ringdef_p);
  fprintf (f, "Ring Definition\nCycle #\tsize\t\t\tnodes\n");
  fprintf (f, "-------\t----\t----------------------------------------\n");
  for (i = 0; i < Ringdef_NumCycles_Get (ringdef_p); i++)
    {
    pcycle_tmp = Ringdef_PrimeCycle_Get (ringdef_p, i);
    fprintf (f, "#%4u\t%3u\t", i, Ringdef_PrimeCycle_NumNodes_Get (
      pcycle_tmp));
    for (j = 0; j < Ringdef_PrimeCycle_NumNodes_Get (pcycle_tmp); j++)
      fprintf (f, "%4u", Ringdef_PrimeCycle_Node_Get (pcycle_tmp, j));

    fprintf (f, "\n");
    }

  fprintf (f, "\n");

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_MinPrimaryRing_Find
*
*    This routine finds the ring number and size of the smallest "primary"
*    cycle, which contains both nodes.  NOTE: this does NOT find the smallest
*    ring which contains both nodes.  If no such ring is found, then
*    SYN_GENFAILURE is returned for the ring index and 0 for the ring size.
*
*  Used to be:
*
*    minring:
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
void Ringdef_MinPrimaryRing_Find
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx,           /* Which RingSystem to look in */
  U16_t            atom1,                 /* One node in match */
  U16_t            atom2,                 /* Other node in match */
  U16_t           *ring_idx_p,            /* Address to store ring index */
  U16_t           *ring_size_p            /* Address to store ring size */
  )
{
  Ringdef_t       *ringdef_p;             /* Ring Defn. for m'cule */
  AtomArray_t     *cycle_p;               /* Cycle array element for m'cule */
  Array_t         *vector_p;              /* Cycle vector for m'cule */
  U8_t             flag;                  /* Flag */

  DEBUG (R_XTR, DB_RINGDEFMINRINGFIND, TL_PARAMS, (outbuf, "Entering\
 Ringdef_MinPrimaryRing_Find, Xtr = %p, ringsys # = %u, atom1 = %u,\
 atom2 = %u", xtr_p, ringsys_idx, atom1, atom2));

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
  vector_p = Ringdef_CycleVector_Get (ringdef_p);

  for (*ring_idx_p = 0, flag = FALSE; *ring_idx_p < Ringdef_NumCycles_Get (
       ringdef_p) && !flag; (*ring_idx_p)++)
    {
    if (Ringdef_CycleVector_Node_Get (vector_p, atom1, *ring_idx_p)
        && Ringdef_CycleVector_Node_Get (vector_p, atom2, *ring_idx_p))
      {
      cycle_p = Ringdef_PrimeCycle_Get (ringdef_p, *ring_idx_p);
      *ring_size_p = Ringdef_PrimeCycle_NumNodes_Get (cycle_p);
      flag = TRUE;
      }
    }

  if (!flag)
    {
    *ring_idx_p = (U16_t) SYN_GENFAILURE;
    *ring_size_p = 0;
    }
  else
    (*ring_idx_p)--;

  DEBUG (R_XTR, DB_RINGDEFMINRINGFIND, TL_PARAMS, (outbuf, 
    "Leaving Ringdef_MinPrimaryRing_Find, ring index = %u, ring size = %u", 
    *ring_idx_p, *ring_size_p));

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_Min4PrimaryRing_Find
*
*    This routine finds the ring number and size of the smallest "primary"
*    cycle, which contains all(4) nodes.  NOTE: this does NOT find the smallest
*    ring which contains both nodes.  If no such ring is found, then
*    SYN_GENFAILURE is returned for both the size and ring index.
*
*  Used to be:
*
*    min4ring:
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
void Ringdef_Min4PrimaryRing_Find
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx,           /* Which RingSystem to look in */
  U16_t            atom1,                 /* One node in match */
  U16_t            atom2,                 /* Other node in match */
  U16_t            atom3,                 /* Third node in match */
  U16_t            atom4,                 /* Fourth node in match */
  U16_t           *ring_idx_p,            /* Address to store ring index */
  U16_t           *ring_size_p            /* Address to store ring size */
  )
{
  Ringdef_t       *ringdef_p;             /* Ring Defn. for m'cule */
  AtomArray_t     *cycle_p;               /* Cycle array element for m'cule */
  Array_t         *vector_p;              /* Cycle vector for m'cule */
  U8_t             flag;                  /* Flag */

  DEBUG (R_XTR, DB_RINGDEFMIN4RINGFIND, TL_PARAMS, (outbuf, 
    "Entering Ringdef_Min4PrimaryRing_Find, Xtr = %p, Ring System = %u,"
    " atom1 = %u, atom2 = %u, atom3 = %u, atom4 = %u", xtr_p, ringsys_idx, 
    atom1, atom2, atom3, atom4));

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
  vector_p = Ringdef_CycleVector_Get (ringdef_p);

  for (*ring_idx_p = 0, flag = FALSE; *ring_idx_p < Ringdef_NumCycles_Get (
       ringdef_p) && !flag; (*ring_idx_p)++)
    {
    if (Ringdef_CycleVector_Node_Get (vector_p, atom1, *ring_idx_p)
        && Ringdef_CycleVector_Node_Get (vector_p, atom2, *ring_idx_p)
        && Ringdef_CycleVector_Node_Get (vector_p, atom3, *ring_idx_p)
        && Ringdef_CycleVector_Node_Get (vector_p, atom4, *ring_idx_p))
      {
      cycle_p = Ringdef_PrimeCycle_Get (ringdef_p, *ring_idx_p);
      *ring_size_p = Ringdef_PrimeCycle_NumNodes_Get (cycle_p);
      flag = TRUE;
      }
    }

  if (!flag)
    {
    *ring_idx_p = (U16_t) SYN_GENFAILURE;
    *ring_size_p = (U16_t) SYN_GENFAILURE;
    }
  else
    (*ring_idx_p)--;

  DEBUG (R_XTR, DB_RINGDEFMIN4RINGFIND, TL_PARAMS, (outbuf, 
    "Leaving Ringdef_Min4PrimaryRing_Find, ring index = %u, ring size = %u", 
    *ring_idx_p, *ring_size_p));

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_NumPrimaryCycles_Find
*
*    This function returns the number of primary cycles in the specified
*    ring definition.
*
*  Used to be:
*
*    nprings:
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
U16_t Ringdef_NumPrimaryCycles_Find
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx            /* Which RingSystem to look in */
  )
{
  Ringdef_t       *ringdef_p;             /* Temporary */

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);

  if (ringdef_p == NULL)
     {
     Xtr_Ringdef_Set (xtr_p, ringsys_idx); 
     ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
     }

  return Ringdef_NumCycles_Get (ringdef_p);
}

/****************************************************************************
*
*  Function Name:                 Ringdef_PrimaryRingNodes_Get
*
*    This routine fills in an array with the atom indices that are
*    part of the specified ring in the specified ring system.
*
*  Used to be:
*
*    gtpring:
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
void Ringdef_PrimaryRingNodes_Get
  (
  Xtr_t           *xtr_p,                 /* Xtr to look in */
  U16_t            ringsys_idx,           /* Which RingSystem to look in */
  U16_t            ring_idx,              /* Which Ring to get atoms of */
  Array_t         *atoms_p                /* Array for results (1-d word) */
  )
{
  Ringdef_t       *ringdef_p;             /* Ring Defn. for m'cule */
  AtomArray_t     *cycle_p;               /* Cycle array for m'cule */
  U16_t            i;                     /* Counter */

  DEBUG (R_XTR, DB_RINGDEFPRMRNGNDSGET, TL_PARAMS, (outbuf, 
    "Entering Ringdef_PrimaryRingNodes_Get, Xtr = %p, Ring System = %u,"
    " Ring = %u, output = %p", xtr_p, ringsys_idx, ring_idx, atoms_p));

  ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);

  if (ringdef_p == NULL)
    {
    Xtr_Ringdef_Set (xtr_p, ringsys_idx);
    ringdef_p = Ringsys_Ringdef_Find (xtr_p, ringsys_idx);
    }

  cycle_p = Ringdef_PrimeCycle_Get (ringdef_p, ring_idx);

  for (i = 0; i < Ringdef_PrimeCycle_NumNodes_Get (cycle_p); i++)
    Array_1d16_Put (atoms_p, i, Ringdef_PrimeCycle_Node_Get (cycle_p, i));

  DEBUG (R_XTR, DB_RINGDEFPRMRNGNDSGET, TL_PARAMS, (outbuf,
    "Leaving Ringdef_PrimaryRingNodes_Get, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringdef_PrimeCycle_Find
*
*    This function sets up part of the BFS search environment.  The BFS search
*    is used to detect cycles in the graph and then to determine some of their
*    characteristics.  This is one of the two main external entrypoints.
*    The resulting cycles that are found are set up into a Ring Definition
*    structure which is returned to the caller.
*
*  Used to be:
*
*    prmcycl:
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
*    Address of Ring Definition
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Ringdef_t *Ringdef_PrimeCycles_Find
  (
  Xtr_t           *xtr_p                  /* Molecule to look at */
  )
{
  LinkedData_t    *cycle_node_p;          /* For list traversal */
  LinkedData_t    *primepaths_p;          /* Temporary */
  LinkedData_t    *next_cycle_p;          /* For rearranging primary paths */
  AtomArray_t     *cycle_tmp;             /* For creating the atom array */
  Ringdef_t       *ringdef_p;             /* For output */
  U32_t            nodeid;                /* Node in path during conversion */
  U16_t            num_nodes;             /* For Atom Array creation */
  U16_t            first_root;            /* Got to start somewhere */
  U16_t            cycle, node;           /* Counters */
  U16_t            element;               /* Counter */
  U16_t            num_atoms;             /* Number of atoms in molecule */
  U16_t            num_cycles;            /* Number of cycles found */
  U8_t             element_degree;        /* Degree of element node */
  U8_t             root_degree;           /* Degree of root node */
  PrmCycleCB_t     primecyc_cb;           /* Control block of arrays */

  DEBUG (R_XTR, DB_RINGDEFPRMCYCFIND, TL_PARAMS, (outbuf,
    "Entering Ringdef_PrimeCycles_Find, Xtr = %p", xtr_p));

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
  primecyc_cb.xtr = xtr_p;

  /* Note first time call.  Search for root with largest degree in order to
     get the search done quickly.
  */

  primecyc_cb.firsttime = TRUE;
  first_root = 0;
  root_degree = Xtr_Attr_NumNeighbors_Get (xtr_p, first_root);
  for (element = 1; element < num_atoms; element++)
    {
    element_degree = Xtr_Attr_NumNeighbors_Get (xtr_p, element);
    if (element_degree > root_degree)
      {
      first_root = element;
      root_degree = element_degree;
      }
    }

  SPrimeCycles_Find (first_root, &primecyc_cb, &primepaths_p, &num_cycles);

  TRACE_DO (DB_RINGDEFPRMCYCFIND, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "Paths from SPrimeCycles_Find");
    LnkData_Dump (primepaths_p, LNKDT_PATH, &GTraceFile);
    });

  ringdef_p = Ringdef_Create (num_cycles);

  /* Format the paths into a Ring Definition structure. */

  for (cycle = 0, next_cycle_p = primepaths_p; cycle < Ringdef_NumCycles_Get (
       ringdef_p); cycle++, next_cycle_p = LnkData_Next_Get (next_cycle_p))
    {
    num_nodes = (U16_t) Path_Length_Get (next_cycle_p);
    cycle_tmp = AtmArr_Create (num_nodes, ATMARR_NOBONDS_WORD, FALSE);
    Ringdef_PrimeCycle_Carbocyclic_Put (cycle_tmp, TRUE);
    Ringdef_PrimeCycle_Put (ringdef_p, cycle, cycle_tmp);

    for (node = 0, cycle_node_p = Path_First_Get (next_cycle_p); node <
         num_nodes; node++, cycle_node_p = LnkData_Next_Get (cycle_node_p))
      {
      nodeid = Node_Id_Get (cycle_node_p);
      Ringdef_PrimeCycle_Node_Put (cycle_tmp, node, nodeid);
      if (Xtr_Attr_Atomid_Get (xtr_p, nodeid) != CARBON)
        Ringdef_PrimeCycle_Carbocyclic_Put (cycle_tmp, FALSE);
      }
    }

  SPath_Destroy (primepaths_p);

  DEBUG (R_XTR, DB_RINGDEFPRMCYCFIND, TL_PARAMS, (outbuf,
    "Leaving Ringdef_PrimeCycle_Find, Ring Def. = %p", ringdef_p));

  return ringdef_p;
}

/****************************************************************************
*
*  Function Name:                 SAdd_Node
*
*    This routine adds a node to a path at the specified end.
*
*  Used to be:
*
*    add_node:
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
static void SAdd_Node
  (
  LinkedData_t    *which_path_p,          /* Path to add node to */
  U32_t            nodeid,                /* Node identifier */
  Boolean_t        add_to_end             /* Flag for where to add */
  )
{
  LinkedData_t    *node_p;                /* Temporary */
  LinkedData_t    *where_p;               /* For list traversal */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf, 
    "Entering SAdd_Node, path = %p, node id = %u, end flag = %hu", 
    which_path_p, nodeid, add_to_end));

  node_p = LnkData_Create ();
  Node_Id_Put (node_p, nodeid);
  LnkData_Next_Put (node_p, NULL);

  if (add_to_end)
    {
    where_p = Path_Last_Get (which_path_p);
    if (where_p != NULL)
      {
      LnkData_Next_Put (where_p, node_p);
      }
    else
      Path_First_Put (which_path_p, node_p);

    Path_Last_Put (which_path_p, node_p);
    }
  else
    {
    where_p = Path_First_Get (which_path_p);
    LnkData_Next_Put (node_p, where_p);
    if (where_p == NULL)
      Path_Last_Put (which_path_p, node_p);

    Path_First_Put (which_path_p, node_p);
    }

  Path_Length_Put (which_path_p, Path_Length_Get (which_path_p) + 1);

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SAdd_Node, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SBfstree_Build
*
*    This routine walks the atom graph in BFS fashion, it fills in all the
*    arrays with the information it finds during the graph walk.  This ends
*    up determining the characteristics of the cycles which are interpreted
*    in Ringdef_PrimeCycles_Find.
*
*  Used to be:
*
*    build_bfs_tree:
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
static void SBfstree_Build
  (
  LinkedData_t   **first_chord_pp,        /* Head of chord list */
  PrmCycleCB_t    *pccb_p                 /* Control block of arrays */
  )
{
  LinkedData_t    *chord_p;               /* Temporary */
  LinkedData_t    *last_chord_p;          /* End of list pointer */
  Xtr_t           *xtr_p;                 /* Temporary */
  U16_t            last_vertex;           /* For walking the graph */
  U16_t            cur_vertex;            /* For walking the graph */
  U16_t            loop;                  /* Counter */
  U16_t            son;                   /* Atom index of neighbor */
  U8_t             neigh;                 /* Counter */
  U8_t             num_neighbors;         /* Compiler bug */
  Array_t          visited;               /* Array of flags */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Entering SBfstree_Build"));

  xtr_p = PrmCycCB_Xtr_Get (pccb_p);
  *first_chord_pp = NULL;
  last_chord_p = NULL;
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&visited", "ringdefinition{3}", &visited, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
#else
  Array_1d_Create (&visited, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
#endif
  Array_Set (&visited, FALSE);

  Array_1d16_Put (PrmCycCB_Father_Get (pccb_p), PrmCycCB_Rootid_Get (pccb_p),
    PrmCycCB_Rootid_Get (pccb_p));
  Array_1d16_Put (PrmCycCB_Level_Get (pccb_p), PrmCycCB_Rootid_Get (pccb_p),
    0);

  cur_vertex = PrmCycCB_Rootid_Get (pccb_p);
  last_vertex = PrmCycCB_Rootid_Get (pccb_p);
  for (loop = 0; loop < Xtr_NumAtoms_Get (xtr_p); loop++)
    {
    Array_1d1_Put (&visited, cur_vertex, TRUE);
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, cur_vertex);
    for (neigh = 0; neigh < num_neighbors; neigh++)
      {
      son = Xtr_Attr_NeighborId_Get (xtr_p, cur_vertex, neigh);
      if (!Array_1d1_Get (&visited, son))
        { 
        if (Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), son) == XTR_INVALID)
          {
          Array_1d16_Put (PrmCycCB_Father_Get (pccb_p), son, cur_vertex);
          Array_1d16_Put (PrmCycCB_Level_Get (pccb_p), son, Array_1d16_Get (
            PrmCycCB_Level_Get (pccb_p), cur_vertex) + 1);
          Array_1d16_Put (PrmCycCB_Bfsorder_Get (pccb_p), last_vertex, son);
          last_vertex = son;
          }
        else
          {
          DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MAJOR, (outbuf,
            "Chord found: %u %u", cur_vertex, son));

          /* If a chord in the graph can be found, record the details as it
             will affect whether a cycle is primary or composite.
          */

          chord_p = LnkData_Create ();
          Chord_Fundamental_Put (chord_p, cur_vertex);
          Chord_End_Put (chord_p, son);
          if (Array_1dAddr_Get (PrmCycCB_Fundamental_Get (pccb_p), cur_vertex)
              == (U32_t) NULL)
	    {
            Array_1dAddr_Put (PrmCycCB_Fundamental_Get (pccb_p), cur_vertex,
              chord_p);
	    }
          else
            Array_1d1_Put (PrmCycCB_Multifundamental_Get (pccb_p), cur_vertex,
              TRUE);

          if (Array_1d16_Get (PrmCycCB_Level_Get (pccb_p), son) ==
              Array_1d16_Get (PrmCycCB_Level_Get (pccb_p), cur_vertex) + 1)
            {
            Chord_Isback_Put (chord_p, TRUE);
            }
          else
            {
            Chord_Isback_Put (chord_p, FALSE);
            }

          Array_1d16_Put (PrmCycCB_ChordEndpoint_Get (pccb_p), son,
            Array_1d16_Get (PrmCycCB_ChordEndpoint_Get (pccb_p), son) + 1);
          Array_1d16_Put (PrmCycCB_ChordEndpoint_Get (pccb_p), cur_vertex,
            Array_1d16_Get (PrmCycCB_ChordEndpoint_Get (pccb_p),
            cur_vertex) + 1);

          if (*first_chord_pp == NULL)
            {
            *first_chord_pp = chord_p;
            last_chord_p = chord_p;
            }
          else
            {
            LnkData_Next_Put (last_chord_p, chord_p);
            last_chord_p = chord_p;
            }
          }           /* End of else-Father[son] == XTR_INVALID */
        }             /* End of if-!visited[son] */
      }               /* End for-neigh loop */

      cur_vertex = Array_1d16_Get (PrmCycCB_Bfsorder_Get (pccb_p),
        cur_vertex);
    }                 /* End for-loop loop */

  TRACE_DO (DB_RINGDEFSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "\tVertex\tFather\tLevel\tBFS order");
    for (loop = 0; loop < Xtr_NumAtoms_Get (xtr_p); loop++)
      TRACE (R_XTR, DB_RINGDEFSTATIC, TL_TRACE, (outbuf, "\t%u\t%u\t%u\t%u",
        loop, Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), loop),
        Array_1d16_Get (PrmCycCB_Level_Get (pccb_p), loop),
        Array_1d16_Get (PrmCycCB_Bfsorder_Get (pccb_p), loop)));
    });

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&visited", "ringdefinition", &visited);
#else
  Array_Destroy (&visited);
#endif

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SBfstree_Build, first chord = %p", *first_chord_pp));

  return;
}

/****************************************************************************
*
*  Function Name:                 SCommon_Ancestor
*
*    This function looks for the common ancestor in the BFS forest of two
*    nodes (atoms) and returns it.
*
*  Used to be:
*
*    common_ancestor:
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
*    Atom index of common ancestor
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U16_t SCommon_Ancestor
  (
  U16_t            delta,                 /* One node of common ancestor */
  U16_t            epsilon,               /* The other such node */
  Boolean_t        back_chord,            /* Flag for chord */
  Array_t         *father_p               /* Array of father nodes in search */
  )
{
  U16_t            ancestor;              /* Result */
  U16_t            temp;                  /* Temporary */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf, 
    "Entering SCommon_Ancestor, delta = %u, epsilon = %u, back flag = %hu,"
    " father = %p", delta, epsilon, back_chord, father_p));

  ancestor = Array_1d16_Get (father_p, epsilon);

  if (back_chord)
    temp = delta;
  else
    temp = Array_1d16_Get (father_p, delta);

  while (ancestor != temp)
    {
    ancestor = Array_1d16_Get (father_p, ancestor);
    temp = Array_1d16_Get (father_p, temp);
    }

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SCommon_Ancestor, ancestor = %u", ancestor));

  return ancestor;
}

/****************************************************************************
*
*  Function Name:                 SCycle_Form
*
*    This function puts two paths together to form a cycle.
*
*  Used to be:
*
*    form_cycle:
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
*    Address of new cycle in Path form
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static LinkedData_t *SCycle_Form
  (
  LinkedData_t    *first_half_p,          /* Front half of cycle */
  LinkedData_t    *last_half_p            /* Second half of cycle */
  )
{
  LinkedData_t    *new_cycle_p;           /* New cycle created */
  LinkedData_t    *which_node_p;          /* For list traversal */
  Boolean_t        to_end;                /* Flag */

  to_end = TRUE;
  new_cycle_p = SPath_Copy (first_half_p);
  which_node_p = Path_First_Get (last_half_p);
  while (which_node_p != NULL)
    {
    SAdd_Node (new_cycle_p, Node_Id_Get (which_node_p), to_end);
    which_node_p = LnkData_Next_Get (which_node_p);
    }

  return new_cycle_p;
}

/****************************************************************************
*
*  Function Name:                 SCycles_Collect
*
*    This routine collects a bunch of cycles together into a Paths structure.
*
*  Used to be:
*
*    collectcycles:
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
static void SCycles_Collect
  (
  LinkedData_t    *pqchord_p,             /* Chord across the molecule */
  LinkedData_t   **cycleset_pp,           /* Address of paths head */
  PrmCycleCB_t    *pccb_p                 /* Control block of arrays */
  )
{
  LinkedData_t    *cycle_path_p;          /* Head of path for cycle */
  LinkedData_t    *this_node_p;           /* For traversing a path */
  Xtr_t           *xtr_p;                 /* Temporary */
  U32_t            nodeid1;               /* Atom index */
  U32_t            nodeid2;               /* Atom index */
  U16_t            result;                /* Computed atom index */
  U16_t            fundament;             /* Fundamental point of input */
  U16_t            end;                   /* End point of input */
  U16_t            cyclesize;             /* Counter */
  Boolean_t        cprimary;              /* Flag - cycle is primary */
  Boolean_t        prpath_valid;          /* Flag - fund. to result */
  Boolean_t        qrpath_valid;          /* Flag - end to result */
  Boolean_t        detour_valid;          /* Flag */
  Boolean_t        prprimary;             /* Flag - fund. to result */
  Boolean_t        qrprimary;             /* Flag - end to result */
  Boolean_t        incident_to_chord;     /* Flag */
  Boolean_t        forbidden_edge;        /* Flag */
  Boolean_t        backlevel;             /* Flag for chord type */
  Array_t         *prpath_p;              /* Nodes on fund. to result path */
  Array_t         *qrpath_p;              /* Nodes on end to result path */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Entering SCycles_Collect, pqchord = %p", pqchord_p));

  /* Create arrays to collect the cycles in, initialize all the flags */

  xtr_p = PrmCycCB_Xtr_Get (pccb_p);
  prpath_p = PrmCycCB_Scratch1_Get (pccb_p);
  qrpath_p = PrmCycCB_Scratch2_Get (pccb_p);
  Array_Set (PrmCycCB_Prime_Get (pccb_p), FALSE);
  Array_Set (prpath_p, FALSE);
  Array_Set (qrpath_p, FALSE);

  cyclesize = 0;
  cprimary = TRUE;
  prpath_valid = TRUE;
  qrpath_valid = TRUE;
  detour_valid = FALSE;
  incident_to_chord = FALSE;
  forbidden_edge = FALSE;
  prprimary = FALSE;
  qrprimary = FALSE;

  fundament = (U16_t) Chord_Fundamental_Get (pqchord_p);
  end = (U16_t) Chord_End_Get (pqchord_p);
  backlevel = Chord_Isback_Get (pqchord_p);

  TRACE_DO (DB_RINGDEFSTATIC, TL_MAJOR,
    {
    if (backlevel)
      {
      TRACE (R_XTR, DB_RINGDEFSTATIC, TL_TRACE, (outbuf, 
        "fundamental point = %u, end point = %u, is a back chord", 
        fundament, end));
      }
    else
      TRACE (R_XTR, DB_RINGDEFSTATIC, TL_TRACE, (outbuf, 
        "fundamental point = %u, end point = %u, is a parallel chord", 
        fundament, end));
    });

  result = SCommon_Ancestor (fundament, end, backlevel,
    PrmCycCB_Father_Get (pccb_p));
  Array_1d1_Put (prpath_p, result, TRUE);
  Array_1d1_Put (qrpath_p, result, TRUE);

  if (backlevel) 	       /* End is below & to the left of fundament */
    cycle_path_p = SFundamental_Cycle (end, fundament, result, &detour_valid,
      &incident_to_chord, &qrpath_valid, &prpath_valid, &forbidden_edge,
      qrpath_p, prpath_p, pccb_p);
  else		               /* Fundament is parallel & to the left of end */
    cycle_path_p = SFundamental_Cycle (fundament, end, result, &detour_valid,
      &incident_to_chord, &prpath_valid, &qrpath_valid, &forbidden_edge,
      prpath_p, qrpath_p, pccb_p);

  *cycleset_pp = NULL;
  if (result != PrmCycCB_Rootid_Get (pccb_p))
    {
    if (incident_to_chord)
      cprimary = FALSE;
    else
      {
      if (PrmCycCB_First_Get (pccb_p) && !forbidden_edge)
        *cycleset_pp = cycle_path_p;
      else
        SPath_Destroy (cycle_path_p);
      }
    }
  else
    {
    cyclesize = (U16_t) Path_Length_Get (cycle_path_p);

    DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
      "Cyclesize is %u", cyclesize));

    /* Take the two nodes next to the common ancestor, and note what size
       cycles they occur in.
    */

    this_node_p = Path_First_Get (cycle_path_p);
    this_node_p = LnkData_Next_Get (this_node_p);
    nodeid1 = Node_Id_Get (this_node_p);
    this_node_p = Path_Last_Get (cycle_path_p);
    nodeid2 = Node_Id_Get (this_node_p);

    if (Array_1d16_Get (PrmCycCB_Minsize_Get (pccb_p), nodeid1) ==
        (U16_t)INFINITY)
      Array_1d16_Put (PrmCycCB_Minsize_Get (pccb_p), nodeid1, cyclesize);

    if (Array_1d16_Get (PrmCycCB_Minsize_Get (pccb_p), nodeid2) ==
        (U16_t)INFINITY)
      Array_1d16_Put (PrmCycCB_Minsize_Get (pccb_p), nodeid2, cyclesize);

    if (cyclesize <= Array_1d16_Get (PrmCycCB_Minsize_Get (pccb_p), nodeid1))
      {
      qrprimary = TRUE;
      Array_1d16_Put (PrmCycCB_Minsize_Get (pccb_p), nodeid1, cyclesize);
      Array_1d1_Put (PrmCycCB_Prime_Get (pccb_p), nodeid1, TRUE);

      DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
        "%u -> %u is a primary edge", nodeid1, result));
      }

    if (cyclesize <= Array_1d16_Get (PrmCycCB_Minsize_Get (pccb_p), nodeid2))
      {
      prprimary = TRUE;
      Array_1d16_Put (PrmCycCB_Minsize_Get (pccb_p), nodeid2, cyclesize);
      Array_1d1_Put (PrmCycCB_Prime_Get (pccb_p), nodeid2, TRUE);

      DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
        "%u -> %u is a primary edge", nodeid2, result));
      }

    if (!Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), fundament, end))
      {
      if (PrmCycCB_First_Get (pccb_p) && ((Array_1d16_Get (
          PrmCycCB_ChordEndpoint_Get (pccb_p), fundament) > 1) ||
         (Array_1d16_Get (PrmCycCB_ChordEndpoint_Get (pccb_p), end) > 1)))
        cprimary = FALSE;
      else
        {
        if (prprimary || qrprimary)
          {
          if (detour_valid)
            {
            SPath_Destroy (cycle_path_p);
            cycle_path_p = NULL;
            SGenerate (fundament, end, result, cycleset_pp, backlevel, pccb_p);
            }
          else
            if ((prpath_valid || qrpath_valid) && !forbidden_edge)
              *cycleset_pp = cycle_path_p;
          }
        else
          cprimary = FALSE;
        }
      }

    if (forbidden_edge)
      {
      SPath_Destroy (cycle_path_p);
      cycle_path_p = NULL;
      }
    }                    /* End else-result != current root */

  if (!cprimary && cycle_path_p != NULL) 
    SPath_Destroy (cycle_path_p);

  /* If q is a detour point then mark it as such. */

  if (cprimary || !PrmCycCB_First_Get (pccb_p))
    {
    if (backlevel && (Xtr_Attr_NumNeighbors_Get (xtr_p, end) > 2))
      {
      Array_1d1_Put (PrmCycCB_Detour_Get (pccb_p), end, TRUE);

      DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MAJOR, (outbuf,
        "%u is a detour point", end));
      }
    }

  TRACE_DO (DB_RINGDEFSTATIC, TL_MAJOR,
    {
    if (cprimary)
      IO_Put_Trace (R_XTR, "Cycle path is primary");
    else
      IO_Put_Trace (R_XTR, "Cycle path is NOT primary");
    });

  TRACE_DO (DB_RINGDEFSTATIC, TL_MINOR,
    {
    if (prpath_valid)
      IO_Put_Trace (R_XTR, "Fundamental to result path is valid");
    else
      IO_Put_Trace (R_XTR, "Fundamental to result path is NOT valid");

    Array_Dump (prpath_p, &GTraceFile);

    if (qrpath_valid)
      IO_Put_Debug (R_XTR, "End to result path is valid");
    else
      IO_Put_Debug (R_XTR, "End to result path is NOT valid");

    Array_Dump (qrpath_p, &GTraceFile);
    });

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SCycles_Collect, cycle set = %p", *cycleset_pp));

  return;
}

/****************************************************************************
*
*  Function Name:                 SFundamental_Cycle
*
*    This function forms a cycle from a given chord and common ancestor.  It
*    builds up from one end, then the other.
*
*  Used to be:
*
*    fundamental_cycle:
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
*    Address of cycle in path form
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static LinkedData_t *SFundamental_Cycle
  (
  U16_t            alpha,                 /* Atom index of one end of chord */
  U16_t            beta,                  /* Other end of chord */
  U16_t            omega,                 /* Common ancestor */
  Boolean_t       *detour_present_p,      /* Detour flag */
  Boolean_t       *chord_present_p,       /* Chord flag */
  Boolean_t       *alphapath_valid_p,     /* Path from alpha flag */
  Boolean_t       *betapath_valid_p,      /* Path from beta flag */
  Boolean_t       *forbidden_present_p,   /* Forbidden edge flag */
  Array_t         *alpha_p,               /* Alpha node array */
  Array_t         *beta_p,                /* Beta node array */
  PrmCycleCB_t    *pccb_p                 /* Control block of arrays */
  )
{
  LinkedData_t    *fundamental_cycle_p;   /* Path for found cycle */
  U16_t            tau;                   /* Temp node */
  Boolean_t        first;                 /* Flag for first path */
  Boolean_t        direction;             /* Flag ??? */
  Boolean_t        forming_cycle;         /* Flag indicating cycle is formng */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf, 
    "Entering SFundamental_Cycle, alpha node = %u, beta node = %u,"
    " omega node = %u, detour = %hu, chord = %hu, alpha valid = %hu,"
    " beta valid = %hu, forbidden = %hu, alpha array = %p, beta array = %p", 
    alpha, beta, omega, *detour_present_p, *chord_present_p, 
    *alphapath_valid_p, *betapath_valid_p, *forbidden_present_p, alpha_p, 
    beta_p));

  if (Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), alpha, beta))
    *forbidden_present_p = TRUE;

  if (Array_1d16_Get (PrmCycCB_ChordEndpoint_Get (pccb_p), alpha) > 1)
    *chord_present_p = TRUE;

  if (Array_1d16_Get (PrmCycCB_ChordEndpoint_Get (pccb_p), beta) > 1)
    *chord_present_p = TRUE;

  fundamental_cycle_p = LnkData_Create ();
  Path_First_Put (fundamental_cycle_p, NULL);
  Path_Last_Put (fundamental_cycle_p, NULL);
  Path_Length_Put (fundamental_cycle_p, 0);

  /* Start building up a Path from one end of the chord to the common ancestor
     then switch direction to TRUE.  First gets switched to FALSE at the same
     time as we are now building up from the other end.  When we reach the
     common ancestor from the second end, then we are done forming the cycle.
     Pay attention to use of forbidden and valid edges, and chords.
  */

  tau = alpha;
  direction = FALSE;
  first = TRUE;
  forming_cycle = TRUE;
  while (forming_cycle)
    {
    if (Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), tau,
        Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), tau)))
      *forbidden_present_p = TRUE;

    SAdd_Node (fundamental_cycle_p, tau, direction);

    if (!Array_1d1_Get (PrmCycCB_Valid_Get (pccb_p), tau))
      {
      DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
        "node %u is not valid", tau));

      if (first)
        *alphapath_valid_p = FALSE;
      else
        *betapath_valid_p = FALSE;
      }

    if (Array_1d1_Get (PrmCycCB_Detour_Get (pccb_p), tau))
      *detour_present_p = TRUE;

    if ((tau != alpha) && (tau != beta) && (tau != omega) &&
       (Array_1d16_Get (PrmCycCB_ChordEndpoint_Get (pccb_p), tau) > 0))
      *chord_present_p = TRUE;

    if (first)
      {
      if (tau == omega)
        {
        tau = beta;
        first = FALSE;
        direction = TRUE;
        }
      else
        {
        Array_1d1_Put (alpha_p, tau, TRUE);
        tau = Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), tau);
        }
      }
    else
      {
      Array_1d1_Put (beta_p, tau, TRUE);
      if (Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), tau) == omega)
        forming_cycle = FALSE;
      else
        tau = Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), tau);
      }
    }

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf, 
    "Leaving SFundamental_Cycle, fundamental cycle = %p, detour = %hu,"
    " chord = %hu, alpha valid = %hu, beta valid = %hu, forbidden = %hu",
    fundamental_cycle_p, *detour_present_p, *chord_present_p, 
    *alphapath_valid_p, *betapath_valid_p, *forbidden_present_p));

  return fundamental_cycle_p;
}

/****************************************************************************
*
*  Function Name:                 SGenerate
*
*    This routine ???
*
*  Used to be:
*
*    generate:
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
static void SGenerate
  (
  U16_t            p_vertex,              /* One end of potential path */
  U16_t            q_vertex,              /* Other end */
  U16_t            pathroot,              /* Common ancestor of p & q */
  LinkedData_t   **new_cycles_pp,         /* Address of path head */
  Boolean_t        backlevel,             /* Chord type flag */
  PrmCycleCB_t    *pccb_p                 /* Control block of arrays */
  )
{
  LinkedData_t    *va_npr_fp_p;           /* Path head */
  LinkedData_t    *va_npr_fq_p;           /* Path head */
  LinkedData_t    *nva_npr_fp_p;          /* Path head */
  LinkedData_t    *nva_npr_fq_p;          /* Path head */
  LinkedData_t    *va_pr_fp_p;            /* Path head */
  LinkedData_t    *va_pr_fq_p;            /* Path head */
  LinkedData_t    *nva_pr_fp_p;           /* Path head */
  LinkedData_t    *nva_pr_fq_p;           /* Path head */
  LinkedData_t    *singleton_p;           /* ??? */
  Boolean_t        chord_forbidden;       /* Flag */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf, 
    "Entering SGenerate, p vertex = %u, q vertex = %u, path root = %u", 
    p_vertex, q_vertex, pathroot));

  if (Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), p_vertex, q_vertex))
    chord_forbidden = TRUE;
  else
    {
    Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), p_vertex, q_vertex, TRUE);
    Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), q_vertex, p_vertex, TRUE);
    chord_forbidden = FALSE;
    }

  *new_cycles_pp = NULL;
  va_npr_fp_p = NULL;
  va_npr_fq_p = NULL;
  nva_npr_fp_p = NULL;
  nva_npr_fq_p = NULL;
  va_pr_fp_p = NULL;
  va_pr_fq_p = NULL;
  nva_pr_fp_p = NULL;
  nva_pr_fq_p = NULL;

  singleton_p = SPath_Initialize (p_vertex);

  SPath_Get (&singleton_p, backlevel, &va_npr_fp_p, &va_pr_fp_p,
    &nva_npr_fp_p, &nva_pr_fp_p, pathroot, pccb_p);

  singleton_p = SPath_Initialize (q_vertex);

  SPath_Get (&singleton_p, !backlevel, &va_npr_fq_p, &va_pr_fq_p,
    &nva_npr_fq_p, &nva_pr_fq_p, pathroot, pccb_p);

  TRACE_DO (DB_RINGDEFSTATIC, TL_MAJOR,
    {
    TRACE (R_XTR, DB_RINGDEFSTATIC, TL_TRACE, (outbuf,
      "Paths in SGenerate, p = %u q = %u\n", p_vertex, q_vertex));
    IO_Put_Trace (R_XTR, "Valid and prime paths from p:");
    LnkData_Dump (va_pr_fp_p, LNKDT_PATH, &GTraceFile);
    IO_Put_Trace (R_XTR, "Valid and non-prime paths from p:");
    LnkData_Dump (va_npr_fp_p, LNKDT_PATH, &GTraceFile);
    IO_Put_Trace (R_XTR, "Non-valid and prime paths from p:");
    LnkData_Dump (nva_pr_fp_p, LNKDT_PATH, &GTraceFile);
    IO_Put_Trace (R_XTR, "Non-valid and non-prime paths from p:");
    LnkData_Dump (nva_npr_fp_p, LNKDT_PATH, &GTraceFile);
    IO_Put_Trace (R_XTR, "Valid and prime paths from q:");
    LnkData_Dump (va_pr_fq_p, LNKDT_PATH, &GTraceFile);
    IO_Put_Trace (R_XTR, "Valid and non-prime paths from q:");
    LnkData_Dump (va_npr_fq_p, LNKDT_PATH, &GTraceFile);
    IO_Put_Trace (R_XTR, "Non-valid and prime paths from q:");
    LnkData_Dump (nva_pr_fq_p, LNKDT_PATH, &GTraceFile);
    IO_Put_Trace (R_XTR, "Non-valid and non-prime paths from q:");
    LnkData_Dump (nva_npr_fq_p, LNKDT_PATH, &GTraceFile);
    });

  SPath_Combine (va_npr_fq_p, va_pr_fp_p, new_cycles_pp);
  SPath_Combine (va_npr_fq_p, nva_pr_fp_p, new_cycles_pp);
  SPath_Combine (va_pr_fq_p, va_npr_fp_p, new_cycles_pp);
  SPath_Combine (va_pr_fq_p, va_pr_fp_p, new_cycles_pp);
  SPath_Combine (va_pr_fq_p, nva_npr_fp_p, new_cycles_pp);
  SPath_Combine (va_pr_fq_p, nva_pr_fp_p, new_cycles_pp);
  SPath_Combine (nva_npr_fq_p, va_pr_fp_p, new_cycles_pp);
  SPath_Combine (nva_pr_fq_p, va_npr_fp_p, new_cycles_pp);

  SPath_Destroy (va_npr_fp_p);
  SPath_Destroy (va_npr_fq_p);
  SPath_Destroy (nva_npr_fp_p);
  SPath_Destroy (nva_npr_fq_p);
  SPath_Destroy (va_pr_fp_p);
  SPath_Destroy (va_pr_fq_p);
  SPath_Destroy (nva_pr_fp_p);
  SPath_Destroy (nva_pr_fq_p);

  if (!chord_forbidden)
    {
    Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), p_vertex, q_vertex, FALSE);
    Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), q_vertex, p_vertex, FALSE);
    }

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SGenerate, new cycles list = %p", *new_cycles_pp));

  return;
}

/****************************************************************************
*
*  Function Name:                 SInsert
*
*    This routine inserts a Path structure into a list of Paths.
*
*  Used to be:
*
*    insert:
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
static void SInsert
  (
  LinkedData_t    *item_path_p,           /* Newest path */
  LinkedData_t   **position_path_pp,      /* Place in list to insert */
  LinkedData_t   **top_path_pp            /* List of saved paths */
  )
{
  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf, "Entering SInsert,\
 item = %p, position = %p, top = %p", item_path_p, *position_path_pp,
 *top_path_pp));

  /* If position is NULL, then insert at front of list. */

  if (*position_path_pp == NULL)
    {
    LnkData_Next_Put (item_path_p, *top_path_pp);
    *top_path_pp = item_path_p;

    DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
      "Leaving SInsert, status = <void>"));

    return;
    }

  LnkData_Next_Put (item_path_p, LnkData_Next_Get (*position_path_pp));
  LnkData_Next_Put (*position_path_pp, item_path_p);

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SInsert, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPath_Combine
*
*    This routine combines two paths into one by copying.
*
*  Used to be:
*
*    combine_paths:
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
static void SPath_Combine
  (
  LinkedData_t    *path1_p,               /* One path to combine */
  LinkedData_t    *path2_p,               /* The other path to combine */
  LinkedData_t   **outcycleset_pp         /* Address of output path head */
  )
{
  LinkedData_t    *temp1_p;               /* Temp. for list traversal */
  LinkedData_t    *temp2_p;               /* Temp. for list traversal */
  LinkedData_t    *add_cycle_p;           /* Temporary */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Entering SPath_Combine, path 1 = %p, path 2 = %p", path1_p, path2_p));

  temp1_p = path1_p;
  while (temp1_p != NULL)
    {
    temp2_p = path2_p;
    while (temp2_p != NULL)
      {
      add_cycle_p = SCycle_Form (temp1_p, temp2_p);
      SInsert (add_cycle_p, outcycleset_pp, outcycleset_pp);
      temp2_p = LnkData_Next_Get (temp2_p);
      }
    temp1_p = LnkData_Next_Get (temp1_p);
    }

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SPath_Combine, output head = %p", *outcycleset_pp));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPath_Copy
*
*    This function makes a copy of a Path, Node by Node.
*
*  Used to be:
*
*    copy_path:
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
*    Address of copy of Path
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static LinkedData_t *SPath_Copy
  (
  LinkedData_t    *input_path_p           /* Head of path to copy */
  )
{
  LinkedData_t    *pathhead_p;            /* Temporary path head */
  LinkedData_t    *temp_node_p;           /* For list traversal */
  Boolean_t        node_at_end;           /* Flag for where to add node */

  node_at_end = TRUE;
  pathhead_p = LnkData_Create ();
  Path_First_Put (pathhead_p, NULL);
  Path_Last_Put (pathhead_p, NULL);
  Path_Length_Put (pathhead_p, 0);

  temp_node_p = Path_First_Get (input_path_p);

  while (temp_node_p != NULL)
    {
    SAdd_Node (pathhead_p, Node_Id_Get (temp_node_p), node_at_end);
    temp_node_p = LnkData_Next_Get (temp_node_p);
    }

  return pathhead_p;
}

/****************************************************************************
*
*  Function Name:                 SPath_Destroy
*
*    This routine destroys a Path, Node by Node.
*
*  Used to be:
*
*    free_paths:
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
static void SPath_Destroy
  (
  LinkedData_t    *link_p                 /* Path to destroy */
  )
{
  LinkedData_t    *cur_p;                 /* Temporary */
  LinkedData_t    *temp_p;                /* Temporary */
  LinkedData_t    *next_p;                /* Temporary */
  LinkedData_t    *last_p;                /* Temporary */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Entering SPath_Destroy, path = %p", link_p));

  while (link_p != NULL)
    {
    cur_p = Path_First_Get (link_p);
    last_p = Path_Last_Get (link_p);
    next_p = LnkData_Next_Get (link_p);

    while (cur_p != last_p)
      {
      temp_p = LnkData_Next_Get (cur_p);
      LnkData_Destroy (cur_p);
      cur_p = temp_p;
      }

    LnkData_Destroy (last_p);
    LnkData_Destroy (link_p);
    link_p = next_p;
    }

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SPath_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPath_Get
*
*    This routine does generates a set of paths from a given path to the root
*    paying attention to valid detours for path splitting.
*
*  Used to be:
*
*    getpaths:
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
static void SPath_Get
  (
  LinkedData_t   **path_pp,               /* Node to start search on */
  Boolean_t        counter_clockwise,     /* Flag */
  LinkedData_t   **valid_notprime_pp,     /* Valid / non-prime path head */
  LinkedData_t   **valid_prime_pp,        /* Valid / prime path head */
  LinkedData_t   **notvalid_notprime_pp,  /* Non-valid / non-prime path head */
  LinkedData_t   **notvalid_prime_pp,     /* Non-valid / prime path head */
  U16_t            pathroot,              /* Root of the path */
  PrmCycleCB_t    *pccb_p                 /* Control block of arrays */
  )
{
  LinkedData_t    *lasnode_p_t;           /* Temporary */
  LinkedData_t    *detour_path_p;         /* Copy when a detour */
  Xtr_t           *xtr_p;                 /* Temporary */
  U32_t            nodeid;                /* Id of last node in path */
  U16_t            kappa;                 /* Counter */
  U8_t             neigh;                 /* Temporary */
  U8_t             num_neighbors;         /* Compiler bug */
  Boolean_t        valid_path;            /* Flag */
  Boolean_t        add_root;              /* Flag */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf, 
    "Entering SPath_Get, counter clockwise flag = %hu, pathroot = %u", 
    counter_clockwise, pathroot));

  valid_path = TRUE;
  add_root = !counter_clockwise;

  if (counter_clockwise)
    lasnode_p_t = Path_Last_Get (*path_pp);
  else
    lasnode_p_t = Path_First_Get (*path_pp);

  nodeid = Node_Id_Get (lasnode_p_t);
  if (!Array_1d1_Get (PrmCycCB_Valid_Get (pccb_p), nodeid))
    valid_path = FALSE;

  if (Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), nodeid) == pathroot)
    {
    DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf, 
      "Father [nodeid] == pathroot, nodeid = %u, pathroot = %u", nodeid, 
      pathroot));

    if (Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), nodeid,
        Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), nodeid)))
      {
      SPath_Destroy (*path_pp);

      DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf, 
        "Leaving SPath_Get, from point a (forbidden edge from pathroot to"
        " current? node), status = <void>"));

      return;
      }

    if (add_root)
      SAdd_Node (*path_pp, pathroot, counter_clockwise);

    if (Array_1d1_Get (PrmCycCB_Prime_Get (pccb_p), nodeid))
      {
      if (valid_path)
        SInsert (*path_pp, valid_prime_pp, valid_prime_pp);
      else
        SInsert (*path_pp, notvalid_prime_pp, notvalid_prime_pp);
      }
    else
      {
      if (valid_path)
        SInsert (*path_pp, valid_notprime_pp, valid_notprime_pp);
      else
        SInsert (*path_pp, notvalid_notprime_pp, notvalid_notprime_pp);
      }

    DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
      "Leaving SPath_Get, from point b, status = <void>"));

    return;
    }             /* End if-Father[nodeid] == pathroot */

  xtr_p = PrmCycCB_Xtr_Get (pccb_p);
  if (Array_1d1_Get (PrmCycCB_Detour_Get (pccb_p), nodeid))
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, nodeid);
    for (kappa = 0; kappa < num_neighbors; kappa++)
      {
      neigh = Xtr_Attr_NeighborId_Get (xtr_p, nodeid, kappa);
      if (!Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), nodeid, neigh) &&
         (Array_1d16_Get (PrmCycCB_Level_Get (pccb_p), nodeid) ==
          Array_1d16_Get (PrmCycCB_Level_Get (pccb_p), neigh) + 1))
        {
        DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf, 
          "%u is a detour point & %hu satisfies all conditions", nodeid, 
          neigh));

        detour_path_p = SPath_Copy (*path_pp);
        SAdd_Node (detour_path_p, neigh, counter_clockwise);
        SPath_Get (&detour_path_p, counter_clockwise, valid_notprime_pp,
          valid_prime_pp, notvalid_notprime_pp, notvalid_prime_pp, pathroot,
          pccb_p);
        }
      }
      SPath_Destroy (*path_pp);
    }
  else
    if (!Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), nodeid,
        Array_1d16_Get (PrmCycCB_Father_Get (pccb_p), nodeid)))
      {
      SAdd_Node (*path_pp, Array_1d16_Get (PrmCycCB_Father_Get (pccb_p),
        nodeid), counter_clockwise);
      SPath_Get (path_pp, counter_clockwise, valid_notprime_pp, valid_prime_pp,
        notvalid_notprime_pp, notvalid_prime_pp, pathroot, pccb_p);
      }
    else
      SPath_Destroy (*path_pp);

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
    "Leaving SPath_Get, from point c, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPath_Initialize
*
*    This function creates a new Path and initializes it.
*
*  Used to be:
*
*    initialize_path:
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
static LinkedData_t *SPath_Initialize
  (
  U16_t            first_vertex           /* Atom index of first vertex */
  )
{
  LinkedData_t    *path_p;                /* Temporary */
  LinkedData_t    *node_p;                /* Temporary */

  node_p = LnkData_Create ();
  Node_Id_Put (node_p, first_vertex);
  path_p = LnkData_Create ();
  Path_First_Put (path_p, node_p);
  Path_Last_Put (path_p, node_p);
  Path_Length_Put (path_p, 1);

  return path_p;
}

/****************************************************************************
*
*  Function Name:                 SPrimeCycles_Find
*
*    This routine sets up the rest of the environment and controls the BFS
*    search.  It makes sure that all necessary roots are followed.  All
*    cycles are collected and sorted by increasing size.
*
*  Used to be:
*
*    find_primary_cycles:
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
static void SPrimeCycles_Find
  (
  U16_t            initial_root,          /* Index of first root atom */
  PrmCycleCB_t    *pccb_p,                /* Control block of arrays */
  LinkedData_t   **prime_paths_pp,        /* Address of prime paths list */
  U16_t           *num_paths_p            /* # entries in prime paths list */
  )
{
  LinkedData_t    *first_root_p;          /* Root set list head */
  LinkedData_t    *last_root_p;           /* Root set list tail */
  LinkedData_t    *here_root_p;           /* List traversal */
  LinkedData_t    *newpath_set_p;         /* New path set - SCycles_Collect */
  LinkedData_t    *here_valid_p;          /* List traversal */
  LinkedData_t    *here_path_p;           /* List traversal */
  LinkedData_t    *there_path_p;          /* List traversal */
  LinkedData_t    *first_chord_p;         /* Chord list head */
  LinkedData_t    *cur_chord_p;           /* Current chord to process */
  LinkedData_t    *here_chord_p;          /* List traversal */
  LinkedData_t    *objecp_t;              /* Path traversal */
  LinkedData_t    *valid_p;               /* For valid set member creation */
  Xtr_t           *xtr_p;                 /* Molecule in question */
  U32_t            object_pathlen;        /* Length of path object */
  U32_t            point1, point2;        /* Endpoint indices for chord */
  U32_t            thispt, thatpt;        /* From point1, point2 */
  U32_t            num_atoms;             /* Number of atoms in molecule */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Entering SPrimeCycles_Find, init root = %u", initial_root));

  xtr_p = PrmCycCB_Xtr_Get (pccb_p);
  num_atoms = Xtr_NumAtoms_Get (xtr_p);

#ifdef _MIND_MEM_
  mind_Array_1d_Create
    ("PrmCycCB_Fundamental_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Fundamental_Get (pccb_p), num_atoms, ADDRSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Validset_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Validset_Get (pccb_p), num_atoms, ADDRSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Father_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Father_Get (pccb_p), num_atoms, WORDSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Level_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Level_Get (pccb_p), num_atoms, WORDSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Bfsorder_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Bfsorder_Get (pccb_p), num_atoms, WORDSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Minsize_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Minsize_Get (pccb_p), num_atoms, WORDSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_ChordEndpoint_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_ChordEndpoint_Get (pccb_p), num_atoms, WORDSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Detour_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Detour_Get (pccb_p), num_atoms, WORDSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Valid_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Valid_Get (pccb_p), num_atoms, BITSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Multifundamental_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Multifundamental_Get (pccb_p), num_atoms, BITSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Prime_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Prime_Get (pccb_p), num_atoms, BITSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Scratch1_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Scratch1_Get (pccb_p), num_atoms, BITSIZE);
  mind_Array_1d_Create
    ("PrmCycCB_Scratch2_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Scratch2_Get (pccb_p), num_atoms, BITSIZE);
  mind_Array_2d_Create
    ("PrmCycCB_Forbidden_Get(pccb_p)", "ringdefinition{4}", PrmCycCB_Forbidden_Get (pccb_p), num_atoms, num_atoms, BITSIZE);
#else
  Array_1d_Create (PrmCycCB_Fundamental_Get (pccb_p), num_atoms, ADDRSIZE);
  Array_1d_Create (PrmCycCB_Validset_Get (pccb_p), num_atoms, ADDRSIZE);
  Array_1d_Create (PrmCycCB_Father_Get (pccb_p), num_atoms, WORDSIZE);
  Array_1d_Create (PrmCycCB_Level_Get (pccb_p), num_atoms, WORDSIZE);
  Array_1d_Create (PrmCycCB_Bfsorder_Get (pccb_p), num_atoms, WORDSIZE);
  Array_1d_Create (PrmCycCB_Minsize_Get (pccb_p), num_atoms, WORDSIZE);
  Array_1d_Create (PrmCycCB_ChordEndpoint_Get (pccb_p), num_atoms, WORDSIZE);
  Array_1d_Create (PrmCycCB_Detour_Get (pccb_p), num_atoms, WORDSIZE);
  Array_1d_Create (PrmCycCB_Valid_Get (pccb_p), num_atoms, BITSIZE);
  Array_1d_Create (PrmCycCB_Multifundamental_Get (pccb_p), num_atoms, BITSIZE);
  Array_1d_Create (PrmCycCB_Prime_Get (pccb_p), num_atoms, BITSIZE);
  Array_1d_Create (PrmCycCB_Scratch1_Get (pccb_p), num_atoms, BITSIZE);
  Array_1d_Create (PrmCycCB_Scratch2_Get (pccb_p), num_atoms, BITSIZE);
  Array_2d_Create (PrmCycCB_Forbidden_Get (pccb_p), num_atoms, num_atoms,
    BITSIZE);
#endif

  *num_paths_p = 0;

  /* Sets are initially empty */

  *prime_paths_pp = NULL;           /* primary set */

  Array_Set (PrmCycCB_Validset_Get (pccb_p), (U32_t) NULL);     /* valid set */
  Array_Set (PrmCycCB_Forbidden_Get (pccb_p), FALSE);

  valid_p = LnkData_Create ();
  Valid_Id_Put (valid_p, initial_root);
  Array_1dAddr_Put (PrmCycCB_Validset_Get (pccb_p), initial_root, valid_p);

  first_root_p = LnkData_Create ();
  Root_Id_Put (first_root_p, initial_root);
  last_root_p = first_root_p;

  while (first_root_p != NULL)
    {
    Array_Set (PrmCycCB_Father_Get (pccb_p), XTR_INVALID);
    Array_Set (PrmCycCB_Detour_Get (pccb_p), FALSE);
    Array_Set (PrmCycCB_Minsize_Get (pccb_p), INFINITY);
    Array_Set (PrmCycCB_Bfsorder_Get (pccb_p), XTR_INVALID);
    Array_Set (PrmCycCB_ChordEndpoint_Get (pccb_p), 0);
    Array_Set (PrmCycCB_Fundamental_Get (pccb_p), (U32_t) NULL);
    Array_Set (PrmCycCB_Multifundamental_Get (pccb_p), FALSE);

    /* Set the root id for other routines */

    PrmCycCB_Rootid_Put (pccb_p, (U16_t) Root_Id_Get (first_root_p));

    newpath_set_p = NULL;

    DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MAJOR, (outbuf,
      "Call SBfstree_Build, root = %u", PrmCycCB_Rootid_Get (pccb_p)));

    SBfstree_Build (&first_chord_p, pccb_p);

    /* SBfstree_Build grows a bfs spanning tree of the XTR, rooted at rootid
       & in addition, determines the chordset, BFS order, & Fundamental
       matrices.

       After the first time, not all edges are valid, so the valid flags
       must be updated depending on the current root and valid nodes found.
    */

    if (PrmCycCB_First_Get (pccb_p))
      Array_Set (PrmCycCB_Valid_Get (pccb_p), TRUE);
    else
      {
      DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
        "Call SValid_Mark: root = %u", PrmCycCB_Rootid_Get (pccb_p)));

      SValid_Mark ((LinkedData_t *)Array_1dAddr_Get (PrmCycCB_Validset_Get (
        pccb_p), PrmCycCB_Rootid_Get (pccb_p)), PrmCycCB_Rootid_Get (pccb_p),
        pccb_p);
      }

    /* Loop through all the chords found to determine the cycles that they
       define.
    */

    cur_chord_p = first_chord_p;
    while (cur_chord_p != NULL)
      {
      DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_MINOR, (outbuf,
        "Call SCycles_Collect"));

      SCycles_Collect (cur_chord_p, &newpath_set_p, pccb_p);

      /* If no new Paths are generated, then if this is the first time, use
         the chord endpoints as roots for further searching of the graph.
      */

      if (newpath_set_p == NULL)
        {
        if (PrmCycCB_First_Get (pccb_p))
          {
          point1 = Chord_Fundamental_Get (cur_chord_p);
          point2 = Chord_End_Get (cur_chord_p);
          if ((Xtr_Attr_NumNeighbors_Get (xtr_p, point1) >= 
              Xtr_Attr_NumNeighbors_Get (xtr_p, point2)) ||
              Array_1d1_Get (PrmCycCB_Multifundamental_Get (pccb_p), point1))
            {
            thispt = point1;
            thatpt = point2;
            }
          else
            {
            thatpt = point1;
            thispt = point2;
	    }

          for (here_root_p = first_root_p; (here_root_p != NULL) &&
               (Root_Id_Get (here_root_p) != thispt);
               here_root_p = LnkData_Next_Get (here_root_p))
            /* Empty loop body */ ;

          if (here_root_p == NULL)      /* Add this to rootset */
            {
            here_root_p = LnkData_Create ();
            Root_Id_Put (here_root_p, thispt);
            LnkData_Next_Put (last_root_p, here_root_p);
            last_root_p = here_root_p;
            }

          /* Add the fundamental point p to valid set (q). */

          here_valid_p = (LinkedData_t *)Array_1dAddr_Get (
            PrmCycCB_Validset_Get (pccb_p), thispt);
          valid_p = LnkData_Create ();
          Valid_Id_Put (valid_p, thatpt);
          LnkData_Next_Put (valid_p, here_valid_p);
          Array_1dAddr_Put (PrmCycCB_Validset_Get (pccb_p), thispt, valid_p);
          }                            /* End if-first time */
        }                              /* End if-newpath_set_p == NULL */
      else      /*  primaryset = primaryset u newset */
        {
        TRACE_DO (DB_RINGDEFSTATIC, TL_MAJOR,
	  {
          IO_Put_Trace (R_XTR, "New set added to primary set");
          LnkData_Dump (newpath_set_p, LNKDT_PATH, &GTraceFile);
          });

        /* Loop through all the paths generated and sort them by size. */

        while (newpath_set_p != NULL)
          {
          objecp_t = newpath_set_p;
          newpath_set_p = LnkData_Next_Get (newpath_set_p);
          (*num_paths_p)++;
          LnkData_Next_Put (objecp_t, NULL);

          if (*prime_paths_pp == NULL)
            *prime_paths_pp = objecp_t;
          else
            {
            here_path_p = NULL;
            there_path_p = *prime_paths_pp;
            object_pathlen = Path_Length_Get (objecp_t);

            /* Order paths by path length, shortest first, newest last in
               tiebreaker.
            */

            while (there_path_p != NULL)
              {
              if (Path_Length_Get (there_path_p) <= object_pathlen)
                here_path_p = there_path_p;
              else
                there_path_p = NULL;

              if (there_path_p != NULL)
                there_path_p = LnkData_Next_Get (there_path_p);
              }

            SInsert (objecp_t, &here_path_p, prime_paths_pp);
            }
          }                        /* End while-newpath_set_p != NULL */
        }                          /* End else-newpath_set_p == NULL */

      /* Free chord */

      here_chord_p = cur_chord_p;
      cur_chord_p = LnkData_Next_Get (cur_chord_p);
      LnkData_Destroy (here_chord_p);
      here_chord_p = NULL;
      }                            /* while-cur_chord_p != NULL */

    /* Subtract root from rootset  */

    here_root_p = first_root_p;
    first_root_p = LnkData_Next_Get (first_root_p);

    if (first_root_p == NULL)
      last_root_p = NULL;

    LnkData_Destroy (here_root_p);
    here_root_p = NULL;

    /* After the first time through, update the valid node sets and the
       forbidden edges to reflect the chosen root nodes.
    */

    if (PrmCycCB_First_Get (pccb_p))
      {
      PrmCycCB_First_Put (pccb_p, FALSE);

      /* For p = each element of the root set and
         For q = each element of valid set (p),  mark edge (p, q) as forbidden.
      */

      for (here_root_p = first_root_p; here_root_p != NULL; here_root_p =
           LnkData_Next_Get (here_root_p))
        {
        thispt = Root_Id_Get (here_root_p);
        for (here_valid_p = (LinkedData_t *)Array_1dAddr_Get (
             PrmCycCB_Validset_Get (pccb_p), thispt); here_valid_p != NULL;
             here_valid_p = LnkData_Next_Get (here_valid_p))
          {
          thatpt = Valid_Id_Get (here_valid_p);
          Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), thispt, thatpt, TRUE);
          Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), thatpt, thispt, TRUE);
          }
        }
      }                     /* End if-first time */
    }                       /* End while-first_root_p != NULL */

  /* All done, free validset */

  for (thispt = 0; thispt < num_atoms; thispt++)
    {
    here_valid_p = (LinkedData_t *)Array_1dAddr_Get (PrmCycCB_Validset_Get (
      pccb_p), thispt);
    Array_1dAddr_Put (PrmCycCB_Validset_Get (pccb_p), thispt, NULL);
    while (here_valid_p != NULL)
      {
      valid_p = LnkData_Next_Get (here_valid_p);
      LnkData_Destroy (here_valid_p);
      here_valid_p = valid_p;
      }
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("PrmCycCB_Father_Get(pccb_p)", "ringdefinition", PrmCycCB_Father_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Level_Get(pccb_p)", "ringdefinition", PrmCycCB_Level_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Bfsorder_Get(pccb_p)", "ringdefinition", PrmCycCB_Bfsorder_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Minsize_Get(pccb_p)", "ringdefinition", PrmCycCB_Minsize_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_ChordEndpoint_Get(pccb_p)", "ringdefinition", PrmCycCB_ChordEndpoint_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Detour_Get(pccb_p)", "ringdefinition", PrmCycCB_Detour_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Valid_Get(pccb_p)", "ringdefinition", PrmCycCB_Valid_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Multifundamental_Get(pccb_p)", "ringdefinition", PrmCycCB_Multifundamental_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Fundamental_Get(pccb_p)", "ringdefinition", PrmCycCB_Fundamental_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Validset_Get(pccb_p)", "ringdefinition", PrmCycCB_Validset_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Prime_Get(pccb_p)", "ringdefinition", PrmCycCB_Prime_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Scratch1_Get(pccb_p)", "ringdefinition", PrmCycCB_Scratch1_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Scratch2_Get(pccb_p)", "ringdefinition", PrmCycCB_Scratch2_Get (pccb_p));
  mind_Array_Destroy ("PrmCycCB_Forbidden_Get(pccb_p)", "ringdefinition", PrmCycCB_Forbidden_Get (pccb_p));
#else
  Array_Destroy (PrmCycCB_Father_Get (pccb_p));
  Array_Destroy (PrmCycCB_Level_Get (pccb_p));
  Array_Destroy (PrmCycCB_Bfsorder_Get (pccb_p));
  Array_Destroy (PrmCycCB_Minsize_Get (pccb_p));
  Array_Destroy (PrmCycCB_ChordEndpoint_Get (pccb_p));
  Array_Destroy (PrmCycCB_Detour_Get (pccb_p));
  Array_Destroy (PrmCycCB_Valid_Get (pccb_p));
  Array_Destroy (PrmCycCB_Multifundamental_Get (pccb_p));
  Array_Destroy (PrmCycCB_Fundamental_Get (pccb_p));
  Array_Destroy (PrmCycCB_Validset_Get (pccb_p));
  Array_Destroy (PrmCycCB_Prime_Get (pccb_p));
  Array_Destroy (PrmCycCB_Scratch1_Get (pccb_p));
  Array_Destroy (PrmCycCB_Scratch2_Get (pccb_p));
  Array_Destroy (PrmCycCB_Forbidden_Get (pccb_p));
#endif

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SPrimeCycles_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SValid_Mark
*
*    This routine updates which nodes are valid from the current root and 
*    updates the forbidden edges to reflect the fundamental edges and their
*    chords.
*
*  Used to be:
*
*    mark_valid:
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
static void SValid_Mark
  (
  LinkedData_t    *sigma_p,               /* Valid structure */
  U16_t            rho,                   /* Root id */
  PrmCycleCB_t    *pccb_p                 /* Control block of arrays */
  )
{
  LinkedData_t    *temp_ld_p;             /* For list traversal */
  U32_t            num_atoms;             /* Number of atoms in molecule */
  U32_t            point;                 /* Valid identifier */
  U32_t            end, fundament;        /* Temporaries */
  U32_t            k;                     /* Counter */

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Entering SValid_Mark, sigma = %p, rho = %u", sigma_p, rho));

  num_atoms = Xtr_NumAtoms_Get (PrmCycCB_Xtr_Get (pccb_p));

  /* Mark all nodes invalid */

  Array_Set (PrmCycCB_Valid_Get (pccb_p), FALSE);

  /* Mark all points in sigma valid list as valid */

  for (temp_ld_p = sigma_p; temp_ld_p != NULL; temp_ld_p = LnkData_Next_Get (
       temp_ld_p))
    {
    point = Valid_Id_Get (temp_ld_p);
    Array_1d1_Put (PrmCycCB_Valid_Get (pccb_p), point, TRUE);

    /* Edge is no longer forbidden */

    Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), point, rho, FALSE);
    Array_2d1_Put (PrmCycCB_Forbidden_Get (pccb_p), rho, point, FALSE);
    }

  /* Visit each node of molecule in BFS order from the root.  Add all 
     back chords' end points as valid.  The last node obviously doesn't have
     a valid next node to look at hence the loop upper bound.
  */

  for (k = 0, point = rho; k < num_atoms - 1; k++)
    {
    point = Array_1d16_Get (PrmCycCB_Bfsorder_Get (pccb_p), point);
    if (Array_1d1_Get (PrmCycCB_Valid_Get (pccb_p), Array_1d16_Get (
        PrmCycCB_Father_Get (pccb_p), point)))
      Array_1d1_Put (PrmCycCB_Valid_Get (pccb_p), point, TRUE);

    if (Array_1d1_Get (PrmCycCB_Valid_Get (pccb_p), point))
      {
      if (Array_1dAddr_Get (PrmCycCB_Fundamental_Get (pccb_p), point) != 
          (U32_t) NULL)
        {
        temp_ld_p = (LinkedData_t *)Array_1dAddr_Get (PrmCycCB_Fundamental_Get (
          pccb_p), point);
        fundament = point;
        while ((fundament == point) && (temp_ld_p != NULL))
          {
          end = Chord_End_Get (temp_ld_p);
          if (Chord_Isback_Get (temp_ld_p))
            Array_1d1_Put (PrmCycCB_Valid_Get (pccb_p), end, TRUE);

          temp_ld_p = LnkData_Next_Get (temp_ld_p);
          if (temp_ld_p != NULL)
            fundament = Chord_Fundamental_Get (temp_ld_p);
          }
        }
      }              /* End if-valid[point] */
    }                /* End for-k loop */

  /* Mark the root as valid */

  Array_1d1_Put (PrmCycCB_Valid_Get (pccb_p), rho, TRUE);

  TRACE_DO (DB_RINGDEFSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "Valid nodes");
    Array_Dump (PrmCycCB_Valid_Get (pccb_p), &GTraceFile);

    for (k = 0; k < num_atoms; k++)
      for (end = k; end < num_atoms; end++)
        if (Array_2d1_Get (PrmCycCB_Forbidden_Get (pccb_p), k, end))
          TRACE (R_XTR, DB_RINGDEFSTATIC, TL_TRACE, (outbuf,
            "Edge %u -> %u is forbidden", k, end));
    });

  DEBUG (R_XTR, DB_RINGDEFSTATIC, TL_PARAMS, (outbuf,
    "Leaving SValid_Mark, staus = <void>"));

  return;
}
/* End of SValid_Mark */
/* End of RINGDEFINITION.C */
