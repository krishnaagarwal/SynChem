#ifndef _H_NAME_
#define _H_NAME_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     NAME.H
*
*    This module is the abstraction for the Name data-structure.  It is a 
*    sub-structure of the Xtr.  This contains the canonical name (Sling) for
*    the molecule as well as information about constitutionally and
*    stereochemically equivilant atoms.
*
*    Routines are found in NAME.C unless otherwise noted
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

#ifndef _H_SLING_
#include "sling.h"
#endif

/*** Literal Values ***/

/*** Data Structures ***/

/* Name structure is below.  It has the canonical Sling, plus a set of 
   equivilance classes for the atoms.
*/

typedef struct s_xtrname
  {
  U16_t         const_len;                   /* Length of constitutionally
                                                equivilant portion of Sling */
  U16_t         flags;                       /* Flag bit vector */
  Array_t        equiv_classesb;             /* Equivilance classes of Func.
                                                Groups */
  Sling_t       name;                        /* Canonical name */
  } Name_t;
#define NAMESIZE sizeof (Name_t)

/* Flag Literal Values */

#define NameM_AnyCE    0x1
#define NameM_AnySE    0x2

/** Field Access Macros for a Name_t **/

/* Macro Prototypes
   Sling_t   Name_Canonical_Get    (Name_t *);
   void      Name_Canonical_Put    (Name_t *, Sling_t);
   U16_t     Name_ConstitutionalLength_Get (Name_t *);
   void      Name_ConstitutionalLength_Put (Name_t *, U16_t);
   U16_t     Name_CEMember_Get     (Name_t *, U16_t);
   void      Name_CEMember_Put     (Name_t *, U16_t, U16_t);
   Array_t   Name_EquivClasses_Get (Name_t *);
   void      Name_Flags_Put        (Name_t *, U16_t);
   Boolean_t Name_Flags_AnyCE_Get  (Name_t *);
   void      Name_Flags_AnyCE_Put  (Name_t *, Boolean_t);
   Boolean_t Name_Flags_AnySE_Get  (Name_t *);
   void      Name_Flags_AnySE_Put  (Name_t *, Boolean_t);
   U16_t     Name_NumAtoms_Get     (Name_t *);
   U16_t     Name_SEMember_Get     (Name_t *, U16_t);
   void      Name_SEMember_Put     (Name_t *, U16_t, U16_t);
*/

#ifndef XTR_DEBUG
#define Name_Canonical_Get(name_p)\
  (name_p)->name

#define Name_Canonical_Put(name_p, value)\
  (name_p)->name = (value)

#define Name_ConstitutionalLength_Get(name_p)\
  (name_p)->const_len

#define Name_ConstitutionalLength_Put(name_p, value)\
  (name_p)->const_len = (value)

#define Name_CEMember_Get(name_p, index)\
  Array_2d16_Get (&(name_p)->equiv_classesb, 0, index)

#define Name_CEMember_Put(name_p, index, value)\
  Array_2d16_Put (&(name_p)->equiv_classesb, 0, index, value)

#define Name_EquivClasses_Get(name_p)\
  &(name_p)->equiv_classesb

#define Name_Flags_Put(name_p, value)\
  (name_p)->flags = (value)

#define Name_Flags_AnyCE_Get(name_p)\
  ((name_p)->flags & NameM_AnyCE ? TRUE : FALSE)

#define Name_Flags_AnyCE_Put(name_p, value)\
  { if ((value) == TRUE)\
    (name_p)->flags |= NameM_AnyCE;\
  else\
    (name_p)->flags &= ~NameM_AnyCE; }

#define Name_Flags_AnySE_Get(name_p)\
  ((name_p)->flags & NameM_AnySE ? TRUE : FALSE)

#define Name_Flags_AnySE_Put(name_p, value)\
  { if ((value) == TRUE)\
    (name_p)->flags |= NameM_AnySE;\
  else\
    (name_p)->flags &= ~NameM_AnySE; }

#define Name_NumAtoms_Get(name_p)\
  Array_Columns_Get (&(name_p)->equiv_classesb)

#define Name_SEMember_Get(name_p, index)\
  Array_2d16_Get (&(name_p)->equiv_classesb, 1, index)

#define Name_SEMember_Put(name_p, index, value)\
  Array_2d16_Put (&(name_p)->equiv_classesb, 1, index, value)
#else
#define Name_Canonical_Get(name_p)\
  ((name_p) < GBAddr ? HALTSL : (name_p)->name)

#define Name_Canonical_Put(name_p, value)\
  { if ((name_p) < GBAddr) HALT; else (name_p)->name = (value); }

#define Name_ConstitutionalLength_Get(name_p)\
  ((name_p) < GBAddr ? HALT : (name_p)->const_len)

#define Name_ConstitutionalLength_Put(name_p, value)\
  { if ((name_p) < GBAddr) HALT; else (name_p)->const_len = (value); }

#define Name_CEMember_Get(name_p, index)\
  ((name_p) < GBAddr ? HALT : Array_2d16_Get (&(name_p)->equiv_classesb, 0,\
  index))

#define Name_CEMember_Put(name_p, index, value)\
  { if ((name_p) < GBAddr) HALT; else Array_2d16_Put (\
  &(name_p)->equiv_classesb, 0, index, value); }

#define Name_EquivClasses_Get(name_p)\
  ((name_p) < GBAddr ? HALT : &(name_p)->equiv_classesb)

#define Name_Flags_Put(name_p, value)\
  { if ((name_p) < GBAddr) HALT; else (name_p)->flags = (value); }

#define Name_Flags_AnyCE_Get(name_p)\
  ((name_p) < GBAddr ? HALT : (name_p)->flags & NameM_AnyCE ? TRUE : FALSE)

#define Name_Flags_AnyCE_Put(name_p, value)\
  { if ((name_p) < GBAddr) HALT; else if ((value) == TRUE)\
  (name_p)->flags |= NameM_AnyCE; else\
  (name_p)->flags &= ~NameM_AnyCE; }

#define Name_Flags_AnySE_Get(name_p)\
  ((name_p) < GBAddr ? HALT : (name_p)->flags & NameM_AnySE ? TRUE : FALSE)

#define Name_Flags_AnySE_Put(name_p, value)\
  { if ((name_p) < GBAddr) HALT; else if ((value) == TRUE)\
  (name_p)->flags |= NameM_AnySE; else\
  (name_p)->flags &= ~NameM_AnySE; }

#define Name_NumAtoms_Get(name_p)\
  ((name_p) < GBAddr ? HALT : Array_Columns_Get (&(name_p)->equiv_classesb))

#define Name_SEMember_Get(name_p, index)\
  ((name_p) < GBAddr ? HALT : Array_2d16_Get (&(name_p)->equiv_classesb, 1,\
  index))

#define Name_SEMember_Put(name_p, index, value)\
  { if ((name_p) < GBAddr) HALT; else Array_2d16_Put (\
  &(name_p)->equiv_classesb, 1, index, value); }
#endif

/** End of Field Access Macros for Name_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

Name_t *Name_Copy    (Name_t *);
void    Name_Destroy (Name_t *);
void    Name_Dump    (Name_t *, FileDsc_t *);

/* In XTR.H
   Name_ConstituitionalEquivilance_Get
   Name_Create
   Name_Sling_Get
   Name_SlingMap_Get
   Name_StereochemicalEquivilance_Get
   Xtr_Name_Set
*/

/*** Global Variables ***/

/* End of Name.H */
#endif
