#ifndef _H_RINGSYSTEMS_
#define _H_RINGSYSTEMS_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RINGSYSTEMS.H
*
*    This module is the abstraction for the Ring Systems data-structure.
*    As such it is a sub-structure of the XTR.  It allows Synchem to know
*    which primary rings go with which ring systems.
*
*    Routines are found in RINGSYSTEMS.C unless otherwise noted
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

#ifndef _H_RINGDEFINITION_
#include "ringdefinition.h"
#endif

/*** Literal Values ***/

/*** Data Structures ***/

/* This data-structure describes a system of rings.  A ring system is a set
   of atoms all involved in at least one ring.  If two rings share a bond, 
   then effectively all atoms are involved in 3(!) rings, two primary and one
   which doesn't include the share(d) bond(s).  The components field contains
   indications of which bonds are in the cycle, plus a quick indication that
   an atom is in the cycle.
*/

typedef struct s_ringsys
  {
  struct s_ringsys *next;                   /* Next structure in linked list */
  Ringdef_t     *ring_definition;           /* Definition of primary rings */
  Array_t        components;                /* 2-d bit bond mask, plus 1 bit
                                               per atom */
  } Ringsys_t;
#define RINGSYSSIZE sizeof (Ringsys_t)

/** Field Access Macros for a Ringsys_t **/

/* Macro Prototypes
   Array_t     *Ringsys_ComponentHandle_Get (Ringsys_t *);
   Boolean_t    Ringsys_Component_Get (Ringsys_t *, U16_t, U8_t);
   void         Ringsys_Component_Put (Ringsys_t *, U16_t, U8_t, Boolean_t);
   Boolean_t    Ringsys_Isin_Get      (Ringsys_t *, U16_t);
   void         Ringsys_Isin_Put      (Ringsys_t *, U16_t, Boolean_t);
   Ringsys_t   *Ringsys_Next_Get      (Ringsys_t *);
   void         Ringsys_Next_Put      (Ringsys_t *, Ringsys_t *);
   U16_t        Ringsys_NumAtoms_Get  (Ringsys_t *);
   Ringdef_t   *Ringsys_Ringdef_Get   (Ringsys_t *);
   void         Ringsys_Ringdef_Put   (Ringsys_t *, Ringdef_t *);
*/

#ifndef XTR_DEBUG
#define Ringsys_ComponentHandle_Get(rsys_p)\
  &(rsys_p)->components

#define Ringsys_Component_Get(rsys_p, atom, neigh)\
  Array_2d1_Get (&(rsys_p)->components, atom, neigh)

#define Ringsys_Component_Put(rsys_p, atom, neigh, value)\
  Array_2d1_Put (&(rsys_p)->components, atom, neigh, value)

#define Ringsys_Isin_Get(rsys_p, atom)\
  Array_2d1_Get (&(rsys_p)->components, atom, MX_NEIGHBORS)

#define Ringsys_Isin_Put(rsys_p, atom, value)\
  Array_2d1_Put (&(rsys_p)->components, atom, MX_NEIGHBORS, value)

#define Ringsys_Next_Get(rsys_p)\
  (rsys_p)->next

#define Ringsys_Next_Put(rsys_p, value)\
  (rsys_p)->next = (value)

#define Ringsys_NumAtoms_Get(rsys_p)\
  Array_Rows_Get (&(rsys_p)->components)

#define Ringsys_Ringdef_Get(rsys_p)\
  (rsys_p)->ring_definition

#define Ringsys_Ringdef_Put(rsys_p, value)\
  (rsys_p)->ring_definition = (value)
#else
#define Ringsys_ComponentHandle_Get(rsys_p)\
  ((rsys_p) < GBAddr ? HALT : &(rsys_p)->components)

#define Ringsys_Component_Get(rsys_p, atom, neigh)\
  ((rsys_p) < GBAddr ? HALT : Array_2d1_Get (&(rsys_p)->components, atom,\
  neigh))

#define Ringsys_Component_Put(rsys_p, atom, neigh, value)\
  { if ((rsys_p) < GBAddr) HALT; else Array_2d1_Put (&(rsys_p)->components,\
  atom, neigh, value); }

#define Ringsys_Isin_Get(rsys_p, atom)\
  ((rsys_p) < GBAddr ? HALT : Array_2d1_Get (&(rsys_p)->components, atom,\
  MX_NEIGHBORS))

#define Ringsys_Isin_Put(rsys_p, atom, value)\
  { if ((rsys_p) < GBAddr) HALT; else Array_2d1_Put (&(rsys_p)->components,\
  atom, MX_NEIGHBORS, value); }

#define Ringsys_Next_Get(rsys_p)\
  ((rsys_p) < GBAddr ? HALT : (rsys_p)->next)

#define Ringsys_Next_Put(rsys_p, value)\
  { if ((rsys_p) < GBAddr) HALT; else (rsys_p)->next = (value); }

#define Ringsys_NumAtoms_Get(rsys_p)\
  ((rsys_p) < GBAddr ? HALT : Array_Rows_Get (&(rsys_p)->components))

#define Ringsys_Ringdef_Get(rsys_p)\
  ((rsys_p) < GBAddr ? HALT : (rsys_p)->ring_definition)

#define Ringsys_Ringdef_Put(rsys_p, value)\
  { if ((rsys_p) < GBAddr) HALT; else (rsys_p)->ring_definition = (value); }
#endif

/** End of Field Access Macros for Ringsys_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

Ringsys_t *Ringsys_Copy    (Ringsys_t *);
Ringsys_t *Ringsys_Create  (U16_t);
void       Ringsys_Destroy (Ringsys_t *);
void       Ringsys_Dump    (Ringsys_t *, FileDsc_t *);

/*** Global Variables ***/

/* End of RingSystems.H */
#endif
