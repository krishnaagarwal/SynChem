#ifndef _H_RINGS_
#define _H_RINGS_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RINGS.H
*
*    This module is the abstraction for the Rings data-structure.
*    As such it is a sub-structure of the XTR.  It allows Synchem to know
*    about rings in general and which atoms are involved in any ring.
*
*    Routines are found in RINGS.C unless otherwise noted
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
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred	xxx
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_RINGSYSTEMS_
#include "ringsystems.h"
#endif

/*** Literal Values ***/

/*** Data Structures ***/

/* This is the high-level rings descriptor.  It has the head of the ring
   systems list which has all the rings broken down into ?clumps?.  It also
   has the number of ring systems and an array of flags indicating whether
   a particular atom is in any ring at all.
 */

typedef struct s_rings
  {
  Ringsys_t    *ringsys_list;               /* List of all ring systems */
  Array_t       ringbitsb;                  /* 1-d bit array, atoms in rings */
  U16_t         num_ringsys;                /* Number of entries in list */
  } Rings_t;
#define RINGSSIZE sizeof (Rings_t)

/** Field Access Macros for a Rings_t **/

/* Macro Prototypes
   U16_t      Ring_NumAtoms_Get       (Rings_t *);
   U16_t      Ring_NumRingSystems_Get (Rings_t *);
   void       Ring_NumRingSystems_Put (Rings_t *, U16_t);
   Array_t   *Ring_RingBitAddr_Get    (Rings_t *);
   Boolean_t  Ring_RingBit_Get        (Rings_t *, U16_t);
   void       Ring_RingBit_Put        (Rings_t *, U16_t, Boolean_t);
   Ringsys_t *Ring_RingsysList_Get    (Rings_t *);
   void       Ring_RingsysList_Put    (Rings_t *, Ringsys_t *);
*/

#ifndef XTR_DEBUG
#define Ring_NumAtoms_Get(ring_p)\
  ((U16_t) Array_Columns_Get (&(ring_p)->ringbitsb))

#define Ring_NumRingSystems_Get(ring_p)\
  (ring_p)->num_ringsys

#define Ring_NumRingSystems_Put(ring_p, value)\
  (ring_p)->num_ringsys = (value)

#define Ring_RingBitHandle_Get(ring_p)\
  &(ring_p)->ringbitsb

#define Ring_RingBit_Get(ring_p, index)\
  Array_1d1_Get (&(ring_p)->ringbitsb, index)

#define Ring_RingBit_Put(ring_p, index, value)\
  Array_1d1_Put (&(ring_p)->ringbitsb, index, value)

#define Ring_RingsysList_Get(ring_p)\
  (ring_p)->ringsys_list

#define Ring_RingsysList_Put(ring_p, value)\
  (ring_p)->ringsys_list = (value)
#else
#define Ring_NumAtoms_Get(ring_p)\
  ((ring_p) < GBAddr ? HALT \
  : ((U16_t) Array_Columns_Get (&(ring_p)->ringbitsb)))

#define Ring_NumRingSystems_Get(ring_p)\
  ((ring_p) < GBAddr ? HALT : (ring_p)->num_ringsys)

#define Ring_NumRingSystems_Put(ring_p, value)\
  { if ((ring_p) < GBAddr) HALT; else (ring_p)->num_ringsys = (value); }

#define Ring_RingBitHandle_Get(ring_p)\
  ((ring_p) < GBAddr ? HALTP : &(ring_p)->ringbitsb)

#define Ring_RingBit_Get(ring_p, index)\
  ((ring_p) < GBAddr ? HALT : Array_1d1_Get (&(ring_p)->ringbitsb, index))

#define Ring_RingBit_Put(ring_p, index, value)\
  { if ((ring_p) < GBAddr) HALT; else Array_1d1_Put (&(ring_p)->ringbitsb,\
  index, value); }

#define Ring_RingsysList_Get(ring_p)\
  ((ring_p) < GBAddr ? HALT : (ring_p)->ringsys_list)

#define Ring_RingsysList_Put(ring_p, value)\
  { if ((ring_p) < GBAddr) HALT; else (ring_p)->ringsys_list = (value); }
#endif

/** End of Field Access Macros for Rings_t **/

/* Edge data-structure, for finding rings */

typedef struct s_edge
  {
  U16_t         first;
  U16_t         second;
  } Edge_t;
#define EDGESIZE sizeof (Edge_t)

/** Field Access Macros for a Edge_t **/

/* Macro Prototypes
   U16_t Edge_First_Get  (Edge_t *);
   void  Edge_First_Put  (Edge_t *, U16_t);
   U16_t Edge_Second_Get (Edge_t *);
   void  Edge_Second_Put (Edge_t *, U16_t);
*/

#ifndef XTR_DEBUG
#define Edge_First_Get(edge)\
  (edge)->first

#define Edge_First_Put(edge, value)\
  (edge)->first = (value)

#define Edge_Second_Get(edge)\
  (edge)->second

#define Edge_Second_Put(edge, value)\
  (edge)->second = (value)
#else
#define Edge_First_Get(edge_p)\
  ((edge_p) < GBAddr ? HALT : (edge_p)->first)

#define Edge_First_Put(edge_p, value)\
  { if ((edge_p) < GBAddr) HALT; else (edge_p)->first = (value); }

#define Edge_Second_Get(edge_p)\
  ((edge_p) < GBAddr ? HALT : (edge_p)->second)

#define Edge_Second_Put(edge_p, value)\
  { if ((edge_p) < GBAddr) HALT; else (edge_p)->second = (value); }
#endif

/** End of Field Access Macros for Edge_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

Rings_t *Rings_Copy    (Rings_t *);
Rings_t *Rings_Create  (U16_t, Boolean_t);
void     Rings_Destroy (Rings_t *);
void     Rings_Dump    (Rings_t *, FileDsc_t *);

/*** Global Variables ***/

/* End of Rings.H */
#endif
