#ifndef _H_ATOMS
#define _H_ATOMS 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ATOMS.H
*
*    This module is the abstraction of the Atoms functions.  The Atoms_t
*    structure allows Synchem to find out about specific elements in a
*    given molecule.  The Atoms_t structure is a sub-structure of the XTR.
*
*    Routines are found in ATOMS.C unless otherwise noted
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

/*** Literal Values ***/

/*** Data Structures ***/

/* This data-structure describes just initial information about a given
   atomic identifier (number) is.
*/

typedef struct s_atomrow
  {
  U16_t         atomid;                     /* Atomid, no particular order */
  U16_t         first_index;                /* Index of list head */
  U16_t         num_occurences;             /* How many of atom are here */
  } AtomRow_t;
#define ATOMROWSIZE sizeof (AtomRow_t)

/* This is the primary Atoms data-structure.  It describes which atomic numbers
   are present in the molecule (XTR), how to find each instance of them, and
   how many distinct atomic numbers there are.
*/

typedef struct s_atoms
  {
  AtomRow_t    *atoms;                      /* Array of atom id information */
  Array_t       instances;                  /* Linked list of instances */
  U16_t         num_distinct;               /* Number of distinct atom ids */
  } Atoms_t;
#define ATOMSSIZE sizeof (Atoms_t)

/** Field Access Macros for a Atoms_t **/

/* Macro Prototypes
   U16_t      Atoms_Atomid_Get        (Atoms_t *, U16_t);
   void       Atoms_Atomid_Put        (Atoms_t *, U16_t, U16_t);
   AtomRow_t *Atoms_Atoms_Get         (Atoms_t *);
   void       Atoms_Atoms_Put         (Atoms_t *, AtomRow_t *);
   U16_t      Atoms_FirstInstance_Get (Atoms_t *, U16_t);
   void       Atoms_FirstInstance_Put (Atoms_t *, U16_t, U16_t);
   Array_t   *Atoms_Instance_Get      (Atoms_t *);
   U16_t      Atoms_NextInstance_Get  (Atoms_t *, U16_t);
   void       Atoms_NextInstance_Put  (Atoms_t *, U16_t, U16_t);
   U16_t      Atoms_NumAtoms_Get      (Atoms_t *);
   U16_t      Atoms_NumDistinct_Get   (Atoms_t *);
   void       Atoms_NumDistinct_Put   (Atoms_t *, U16_t);
   U16_t      Atoms_NumOccurences_Get (Atoms_t *, U16_t);
   void       Atoms_NumOccurences_Put (Atoms_t *, U16_t, U16_t);
*/

#ifndef XTR_DEBUG
#define Atoms_Atomid_Get(atoms_p, which)\
  (atoms_p)->atoms[which].atomid

#define Atoms_Atomid_Put(atoms_p, which, value)\
  (atoms_p)->atoms[which].atomid = (value)

#define Atoms_Atoms_Get(atoms_p)\
  (atoms_p)->atoms

#define Atoms_Atoms_Put(atoms_p, value)\
  (atoms_p)->atoms = (value)

#define Atoms_FirstInstance_Get(atoms_p, which)\
  (atoms_p)->atoms[which].first_index

#define Atoms_FirstInstance_Put(atoms_p, which, value)\
  (atoms_p)->atoms[which].first_index = (value)

#define Atoms_Instance_Get(atoms_p)\
  &(atoms_p)->instances

#define Atoms_NextInstance_Get(atoms_p, atom)\
  Array_1d16_Get (&(atoms_p)->instances, atom)

#define Atoms_NextInstance_Put(atoms_p, atom, value)\
  Array_1d16_Put (&(atoms_p)->instances, atom, value)

#define Atoms_NumAtoms_Get(atoms_p)\
  ((U16_t) Array_Columns_Get (&(atoms_p)->instances))

#define Atoms_NumDistinct_Get(atoms_p)\
  (atoms_p)->num_distinct

#define Atoms_NumDistinct_Put(atoms_p, value)\
  (atoms_p)->num_distinct = (value)

#define Atoms_NumOccurences_Get(atoms_p, which)\
  (atoms_p)->atoms[which].num_occurences

#define Atoms_NumOccurences_Put(atoms_p, which, value)\
  (atoms_p)->atoms[which].num_occurences = (value)
#else
#define Atoms_Atomid_Get(atoms_p, which)\
  ((atoms_p) < GBAddr || (which) >= (atoms_p)->num_distinct ? HALT :\
  (atoms_p)->atoms[which].atomid)

#define Atoms_Atomid_Put(atoms_p, which, value)\
  { if ((atoms_p) < GBAddr || (which) >= (atoms_p)->num_distinct) HALT; else\
  (atoms_p)->atoms[which].atomid = (value); }

#define Atoms_Atoms_Get(atoms_p)\
  ((atoms_p) < GBAddr ? HALTP : (atoms_p)->atoms)

#define Atoms_Atoms_Put(atoms_p, value)\
  { if ((atoms_p) < GBAddr) HALT; else (atoms_p)->atoms = (value); }

#define Atoms_FirstInstance_Get(atoms_p, which)\
  ((atoms_p) < GBAddr || (which) >= (atoms_p)->num_distinct ? HALT :\
  (atoms_p)->atoms[which].first_index)

#define Atoms_FirstInstance_Put(atoms_p, which, value)\
  { if ((atoms_p) < GBAddr || (which) >= (atoms_p)->num_distinct) HALT; else\
  (atoms_p)->atoms[which].first_index = (value); }

#define Atoms_Instance_Get(atoms_p)\
  ((atoms_p) < GBAddr ? HALTP : &(atoms_p)->instances)

#define Atoms_NextInstance_Get(atoms_p, atom)\
  ((atoms_p) < GBAddr ? HALT : Array_1d16_Get (&(atoms_p)->instances, atom))

#define Atoms_NextInstance_Put(atoms_p, atom, value)\
  { if ((atoms_p) < GBAddr) HALT; else\
    Array_1d16_Put (&(atoms_p)->instances, atom, value); }

#define Atoms_NumAtoms_Get(atoms_p)\
  ((atoms_p) < GBAddr ? HALT \
  : ((U16_t)Array_Columns_Get (&(atoms_p)->instances)))

#define Atoms_NumDistinct_Get(atoms_p)\
  ((atoms_p) < GBAddr ? HALT : (atoms_p)->num_distinct)

#define Atoms_NumDistinct_Put(atoms_p, value)\
  { if ((atoms_p) < GBAddr) HALT; else (atoms_p)->num_distinct = (value); }

#define Atoms_NumOccurences_Get(atoms_p, which)\
  ((atoms_p) < GBAddr || (which) >= (atoms_p)->num_distinct ? HALT :\
  (atoms_p)->atoms[which].num_occurences)

#define Atoms_NumOccurences_Put(atoms_p, which, value)\
  { if ((atoms_p) < GBAddr || (which) >= (atoms_p)->num_distinct) HALT; else\
  (atoms_p)->atoms[which].num_occurences = (value); }

#endif

/** End of Field Access Macros for Atoms_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

Atoms_t *Atoms_Copy    (Atoms_t *);
Atoms_t *Atoms_Create  (U16_t, U16_t);
void     Atoms_Destroy (Atoms_t *);
void     Atoms_Dump    (Atoms_t *, FileDsc_t *);

/* In XTR.H
   Atoms_Count_Find
   Atoms_InstanceIndex_Get
   Xtr_Atoms_Set
*/

/*** Global Variables ***/

/* End of Atoms.H */
#endif
