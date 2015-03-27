#ifndef _H_AROMATICS_
#define _H_AROMATICS_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     AROMATICS.H
*
*    This module is the abstraction for the Aromatics data-structure. 
*    This data-structure describes the aromatic rings in an atom, and
*    is a sub-structure of the XTR
*
*    Routines are found in AROMATICS.C unless otherwise noted.
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
* 22-May-95  Cheung     Change marcos Xtr_Aromat_Node_Get, 
*                       Xtr_Aromat_Bond_Get to procedures,
*                       check aromatic_p before returning the value.    
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

/*** Literal Values ***/

#define NODE_COLUMN    MX_NEIGHBORS

/*** Data Structures ***/

/* The structure simply has an array by atoms of bit flags to indicate which
   bonds are partcipating in the aromatic ring.  There is also a column for
   a flag bit to indicate that an atom is in an aromatic ring.
*/

typedef struct s_aromatic
  {
  Array_t       flagsb;                    /* Array of bond flags
					      (2-d bit array) */
  } Aromatics_t;
#define AROMATICSSIZE sizeof (Aromatics_t)

/** Field Access Macros for a Aromatics_t **/

/* Macro Prototypes
   Boolean_t Aromat_Bond_Get  (Aromatics_t *, U16_t, U16_t);
   void      Aromat_Bond_Put  (Aromatics_t *, U16_t, U16_t, Boolean_t);
   Array_t  *Aromat_Flags_Get (Aromatics_t *);
   Boolean   Aromat_Node_Get  (Aromatics_t *, U16_t);
   void      Aromat_Node_Put  (Aromatics_t *, U16_t, Boolean_t);
*/

#ifndef XTR_DEBUG
#define Aromat_Bond_Get(arom_p, node, bond)\
  Array_2d1_Get (Aromat_Flags_Get (arom_p), node, bond)

#define Aromat_Bond_Put(arom_p, node, bond, value)\
  Array_2d1_Put (Aromat_Flags_Get (arom_p), node, bond, value)

#define Aromat_Flags_Get(arom_p)\
  &(arom_p)->flagsb

#define Aromat_Node_Get(arom_p, node)\
  Array_2d1_Get (Aromat_Flags_Get (arom_p), node, NODE_COLUMN)

#define Aromat_Node_Put(arom_p, node, value)\
  Array_2d1_Put (Aromat_Flags_Get (arom_p), node, NODE_COLUMN, value)
#else
/* Assume that if these macros trap the pointer, the Array_2d1_G/P 
   routines will trap the rest.
*/
#define Aromat_Bond_Get(arom_p, node, bond)\
  Array_2d1_Get (Aromat_Flags_Get (arom_p), node, bond)

#define Aromat_Bond_Put(arom_p, node, bond, value)\
  Array_2d1_Put (Aromat_Flags_Get (arom_p), node, bond, value)

#define Aromat_Flags_Get(arom_p)\
  ((arom_p) < GBAddr ? (Array_t *)HALTP : &(arom_p)->flagsb)

#define Aromat_Node_Get(arom_p, node)\
  Array_2d1_Get (Aromat_Flags_Get (arom_p), node, NODE_COLUMN)

#define Aromat_Node_Put(arom_p, node, value)\
  Array_2d1_Put (Aromat_Flags_Get (arom_p), node, NODE_COLUMN, value)
#endif

/** End of Field Access Macros for a Aromatics_t **/

/*** Routine Prototypes ***/

Aromatics_t *Aromatics_Copy    (Aromatics_t *);
Aromatics_t *Aromatics_Create  (U16_t);
void         Aromatics_Destroy (Aromatics_t *);
void         Aromatics_Dump    (Aromatics_t *, FileDsc_t *);

/* In XTR.H
   Aromat_FindBonds
   Xtr_Aromat_Bond_Get
   Xtr_Aromat_Node_Get
   Xtr_Aromat_Set
*/

/*** Global Variables ***/

/* End of Aromatics.H */
#endif
