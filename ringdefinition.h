#ifndef _H_RINGDEFINITION_
#define _H_RINGDEFINITION_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RINGDEFINITION.H
*
*    This module is the abstraction for the Ring Definition data-structure.
*    As such it is a sub-structure of the Xtr.  It allows Synchem to answer
*    questions about the sizes of rings and the atoms involved in particular
*    primary rings.
*
*    Routines are found in RINGDEFINITION.C unless otherwise noted
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

/*** Literal Values ***/

/*** Data Structures ***/

/* This data-structure contains the definitions of all the rings in a ring
   system.  Each cycle has an array that lists all of the atoms in the array??
   There is also a vector of ???
*/

typedef struct s_ringdef
  {
  Array_t        cycle_vector;              /* 2-d bit, flags for which cycles
                                               atoms are in */
  AtomArray_t  **primary_cycles;            /* Array of cycle pointers */
  U16_t          num_cycles;                /* Number of elements in array */
  } Ringdef_t;
#define RINGDEFSIZE sizeof (Ringdef_t)

/** Field Access Macros for a Ringdef_t **/

/* Macro Prototypes
   Array_t     *Ringdef_CycleVector_Get      (Ringdef_t *);
   Boolean_t    Ringdef_CycleVector_Node_Get (Array_t *, U16_t, U16_t);
   void         Ringdef_CycleVector_Node_Put (Array_t *, U16_t, U16_t, 
                  Boolean_t);
   U16_t        Ringdef_NumCycles_Get        (Ringdef_t *);
   void         Ringdef_NumCycles_Put        (Ringdef_t *, U16_t);
   AtomArray  **Ringdef_PrimeCycleHandle_Get (Ringdef_t *);
   void         Ringdef_PrimeCycleHandle_Put (Ringdef_t *, AtomArray_t **);
   Boolean_t    Ringdef_PrimeCycle_Carbocyclic_Get (AtomArray_t *);
   void         Ringdef_PrimeCycle_Carbocyclic_Put (AtomArray_t *, Boolean_t);
   AtomArray_t *Ringdef_PrimeCycle_Get       (Ringdef_t *, U16_t);
   void         Ringdef_PrimeCycle_Put       (Ringdef_t *, U16_t,
                  AtomArray_t *);
   U16_t        Ringdef_PrimeCycle_Node_Get  (AtomArray_t *, U16_t);
   void         Ringdef_PrimeCycle_Node_Put  (AtomArray_t *, U16_t, U16_t);
   U16_t        Ringdef_PrimeCycle_NumNodes_Get (AtomArray_t *);
*/

/* Now for some fancy manipulation since I'm reusing the AtomArray_t type
   for the RingDefinition Cycle structure, and for the Vector structure
   I'm using the Array_t type.
*/

#ifndef XTR_DEBUG
#define Ringdef_CycleVector_Get(ringdef_p)\
  &(ringdef_p)->cycle_vector

#define Ringdef_CycleVector_Node_Get(vector_p, node, cycle)\
  Array_2d1_Get (vector_p, node, cycle)

#define Ringdef_CycleVector_Node_Put(vector_p, node, cycle, value)\
  Array_2d1_Put (vector_p, node, cycle, value)

#define Ringdef_NumCycles_Get(ringdef_p)\
  (ringdef_p)->num_cycles

#define Ringdef_NumCycles_Put(ringdef_p, value)\
  (ringdef_p)->num_cycles = (value)

#define Ringdef_PrimeCycleHandle_Get(ringdef_p)\
  (ringdef_p)->primary_cycles

#define Ringdef_PrimeCycleHandle_Put(ringdef_p, value)\
  (ringdef_p)->primary_cycles = (value)

#define Ringdef_PrimeCycle_Carbocyclic_Get(cycle_p)\
  (cycle_p)->x.carbocyclic

#define Ringdef_PrimeCycle_Carbocyclic_Put(cycle_p, value)\
  (cycle_p)->x.carbocyclic = (value)

#define Ringdef_PrimeCycle_Get(ringdef_p, index)\
  (ringdef_p)->primary_cycles[index]

#define Ringdef_PrimeCycle_Put(ringdef_p, index, value)\
  (ringdef_p)->primary_cycles[index] = (value)

#define Ringdef_PrimeCycle_Node_Get(cycle_p, node)\
  Array_1d16_Get (AtmArr_Array_Get (cycle_p), node)

#define Ringdef_PrimeCycle_Node_Put(cycle_p, node, value)\
  Array_1d16_Put (AtmArr_Array_Get (cycle_p), node, value)

#define Ringdef_PrimeCycle_NumNodes_Get(cycle_p)\
  ((U16_t) AtmArr_NumAtoms_Get (cycle_p))
#else
#define Ringdef_CycleVector_Get(ringdef_p)\
  ((ringdef_p) < GBAddr ? HALT : &(ringdef_p)->cycle_vector)

#define Ringdef_CycleVector_Node_Get(vector_p, node, cycle)\
  ((vector_p) < GBAddr ? HALT : Array_2d1_Get (vector_p, node, cycle))

#define Ringdef_CycleVector_Node_Put(vector_p, node, cycle, value)\
  { if ((vector_p) < GBAddr) HALT; else Array_2d1_Put (vector_p, node, cycle,\
  value); }

#define Ringdef_NumCycles_Get(ringdef_p)\
  ((ringdef_p) < GBAddr ? HALT : (ringdef_p)->num_cycles)

#define Ringdef_NumCycles_Put(ringdef_p, value)\
  { if ((ringdef_p) < GBAddr) HALT; else (ringdef_p)->num_cycles = (value); }

#define Ringdef_PrimeCycleHandle_Get(ringdef_p)\
  ((ringdef_p) < GBAddr ? HALT : (ringdef_p)->primary_cycles)

#define Ringdef_PrimeCycleHandle_Put(ringdef_p, value)\
  { if ((ringdef_p) < GBAddr) HALT; else (ringdef_p)->primary_cycles =\
  (value); }

#define Ringdef_PrimeCycle_Carbocyclic_Get(cycle_p)\
  ((cycle_p) < GBAddr ? HALT : (cycle_p)->x.carbocyclic)

#define Ringdef_PrimeCycle_Carbocyclic_Put(cycle_p, value)\
  { if ((cycle_p) < GBAddr) HALT; else (cycle_p)->x.carbocyclic = (value); }

#define Ringdef_PrimeCycle_Get(ringdef_p, index)\
  ((ringdef_p) < GBAddr || (index) >= Ringdef_NumCycles_Get (ringdef_p) ?\
  (AtomArray_t *)HALTP : (ringdef_p)->primary_cycles[index])

#define Ringdef_PrimeCycle_Put(ringdef_p, index, value)\
  { if ((ringdef_p) < GBAddr || (index) >= Ringdef_NumCycles_Get (ringdef_p))\
  HALT; else (ringdef_p)->primary_cycles[index] = (value); }

#define Ringdef_PrimeCycle_Node_Get(cycle_p, node)\
  ((cycle_p) < GBAddr ? HALT : Array_1d16_Get (AtmArr_Array_Get (cycle_p),\
  node))

#define Ringdef_PrimeCycle_Node_Put(cycle_p, node, value)\
  { if ((cycle_p) < GBAddr) HALT; else Array_1d16_Put (AtmArr_Array_Get (\
  cycle_p), node, value); }

#define Ringdef_PrimeCycle_NumNodes_Get(cycle_p)\
  ((cycle_p) < GBAddr ? HALT : ((U16_t)AtmArr_NumAtoms_Get (cycle_p)))
#endif

/** End of Field Access Macros for Ringdef_t **/

typedef struct s_prcyc
  {
  void            *xtr;                   /* Molecule in XTR format */
  U16_t            firsttime;             /* Flag for first time called */
  U16_t            rootid;                /* Atom index of root of this tree */
  Array_t          fatherb;               /* For father nodes in BFS */
  Array_t          levelb;                /* Discovery level in BFS */
  Array_t          bfsorderb;             /* Order found in BFS */
  Array_t          minsizeb;              /* Minimum size path */
  Array_t          chord_endpointb;       /* Far end of chords */
  Array_t          detourb;               /* Detours on paths, 3+ neighbors */
  Array_t          validb;                /* Flags for valid atoms */
  Array_t          fundamentalb;          /* Which nodes ??? are fundamental */
  Array_t          multifundamentalb;     /* Flags for which are multi-fund. */
  Array_t          validsetb;             /* Valid neighbors */
  Array_t          primeb;                /* Prime nodes */
  Array_t          forbiddenb;            /* Forbidden edges */
  Array_t          scratch1b;             /* To save on repeated allocation */
  Array_t          scratch2b;             /* As above */
  } PrmCycleCB_t;

/** Field Access Macros for PrmCycleCB_t **/

/* Macro Prototypes
   Array_t *PrmCycCB_Bfsorder_Get         (PrmCycleCB_t *);
   Array_t *PrmCycCB_ChordEndpoint_Get    (PrmCycleCB_t *);
   Array_t *PrmCycCB_Detour_Get           (PrmCycleCB_t *);
   Array_t *PrmCycCB_Father_Get           (PrmCycleCB_t *);
   U16_t    PrmCycCB_First_Get            (PrmCycleCB_t *);
   void     PrmCycCB_First_Put            (PrmCycleCB_t *, U16_t);
   Array_t *PrmCycCB_Forbidden_Get        (PrmCycleCB_t *);
   Array_t *PrmCycCB_Fundamental_Get      (PrmCycleCB_t *);
   Array_t *PrmCycCB_Level_Get            (PrmCycleCB_t *);
   Array_t *PrmCycCB_Minsize_Get          (PrmCycleCB_t *);
   Array_t *PrmCycCB_Multifundamental_Get (PrmCycleCB_t *);
   Array_t *PrmCycCB_Prime_Get            (PrmCycleCB_t *);
   U16_t    PrmCycCB_Rootid_Get           (PrmCycleCB_t *);
   void     PrmCycCB_Rootid_Put           (PrmCycleCB_t *, U16_t);
   Array_t *PrmCycCB_Scratch1_Get         (PrmCycleCB_t *);
   Array_t *PrmCycCB_Scratch2_Get         (PrmCycleCB_t *);
   Array_t *PrmCycCB_Valid_Get            (PrmCycleCB_t *);
   Array_t *PrmCycCB_Validset_Get         (PrmCycleCB_t *);
   Xtr_t   *PrmCycCB_Xtr_Get              (PrmCycleCB_t *);
*/

#ifndef XTR_DEBUG
#define PrmCycCB_Bfsorder_Get(pccb_p)\
  &(pccb_p)->bfsorderb

#define PrmCycCB_ChordEndpoint_Get(pccb_p)\
  &(pccb_p)->chord_endpointb

#define PrmCycCB_Detour_Get(pccb_p)\
  &(pccb_p)->detourb

#define PrmCycCB_Father_Get(pccb_p)\
  &(pccb_p)->fatherb

#define PrmCycCB_First_Get(pccb_p)\
  (pccb_p)->firsttime

#define PrmCycCB_First_Put(pccb_p, value)\
  (pccb_p)->firsttime = (value)

#define PrmCycCB_Forbidden_Get(pccb_p)\
  &(pccb_p)->forbiddenb

#define PrmCycCB_Fundamental_Get(pccb_p)\
  &(pccb_p)->fundamentalb

#define PrmCycCB_Level_Get(pccb_p)\
  &(pccb_p)->levelb

#define PrmCycCB_Minsize_Get(pccb_p)\
  &(pccb_p)->minsizeb

#define PrmCycCB_Multifundamental_Get(pccb_p)\
  &(pccb_p)->multifundamentalb

#define PrmCycCB_Prime_Get(pccb_p)\
  &(pccb_p)->primeb

#define PrmCycCB_Rootid_Get(pccb_p)\
  (pccb_p)->rootid

#define PrmCycCB_Rootid_Put(pccb_p, value)\
  (pccb_p)->rootid = (value)

#define PrmCycCB_Scratch1_Get(pccb_p)\
  &(pccb_p)->scratch1b

#define PrmCycCB_Scratch2_Get(pccb_p)\
  &(pccb_p)->scratch2b

#define PrmCycCB_Valid_Get(pccb_p)\
  &(pccb_p)->validb

#define PrmCycCB_Validset_Get(pccb_p)\
  &(pccb_p)->validsetb

#define PrmCycCB_Xtr_Get(pccb_p)\
  ((Xtr_t *)(pccb_p)->xtr)
#else
#define PrmCycCB_Bfsorder_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->bfsorderb)

#define PrmCycCB_ChordEndpoint_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->chord_endpointb)

#define PrmCycCB_Detour_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->detourb)

#define PrmCycCB_Father_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->fatherb)

#define PrmCycCB_First_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : (pccb_p)->firsttime)

#define PrmCycCB_First_Put(pccb_p, value)\
  { if ((pccb_p) < GBAddr) HALT; else (pccb_p)->firsttime = (value); }

#define PrmCycCB_Forbidden_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->forbiddenb)

#define PrmCycCB_Fundamental_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->fundamentalb)

#define PrmCycCB_Level_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->levelb)

#define PrmCycCB_Minsize_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->minsizeb)

#define PrmCycCB_Multifundamental_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->multifundamentalb)

#define PrmCycCB_Prime_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->primeb)

#define PrmCycCB_Rootid_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : (pccb_p)->rootid)

#define PrmCycCB_Rootid_Put(pccb_p, value)\
  { if ((pccb_p) < GBAddr) HALT; else (pccb_p)->rootid = (value); }

#define PrmCycCB_Scratch1_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->scratch1b)

#define PrmCycCB_Scratch2_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->scratch2b)

#define PrmCycCB_Valid_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->validb)

#define PrmCycCB_Validset_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : &(pccb_p)->validsetb)

#define PrmCycCB_Xtr_Get(pccb_p)\
  ((pccb_p) < GBAddr ? HALT : ((Xtr_t *)(pccb_p)->xtr))
#endif

/** End of Field Access Macros for PrmCycleCB_t **/

/*** Macros ***/

/* Macro prototypes
   void         Ringdef_CycleVector_Copy     (Array_t *, Array_t *);
   AtomArray_t *Ringdef_PrimeCycle_Copy      (AtomArray_t *);
*/

#ifndef XTR_DEBUG
#define Ringdef_CycleVector_Copy(ivec_p, ovec_p)\
  Array_Copy (ivec_p, ovec_p)

#define Ringdef_PrimeCycle_Copy(cycle_p)\
  AtmArr_Copy (cycle_p)
#else
#define Ringdef_CycleVector_Copy(ivec_p, ovec_p)\
  ((ivec_p) < GBAddr || (ovec_p) < GBAddr ? HALT : Array_Copy (ivec_p, ovec_p))

#define Ringdef_PrimeCycle_Copy(cycle_p)\
  ((cycle_p) < GBAddr ? HALT : AtmArr_Copy (cycle_p))
#endif

/*** Routine Prototypes ***/

Ringdef_t *Ringdef_Copy    (Ringdef_t *);
Ringdef_t *Ringdef_Create  (U16_t);
void       Ringdef_Destroy (Ringdef_t *);
void       Ringdef_Dump    (Ringdef_t *, FileDsc_t *);

/* In XTR.H
   Ringdef_Carbocyclic_Find
   Ringdef_CommonRing_Find
   Ringdef_CycleMember_Set
   Ringdef_CycleSize_Find
   Ringdef_MinPrimaryRing_Find
   Ringdef_Min4PrimaryRing_Find
   Ringdef_NumPrimaryCycles_Find
   Ringdef_PrimaryRingNodes_Get
   Ringdef_PrimeCycles_Find
*/

/*** Global Variables ***/

/* End of RingDefinition.H */
#endif
