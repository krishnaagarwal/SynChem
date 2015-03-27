/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ATOMS.C
*
*    This module is the abstraction for the Atoms description of the XTR.
*    The Atoms_t structure keeps track of the number and location of
*    each specific atom, and some of the generics.
*
*  Routines:
*
*    Atoms_Copy
*    Atoms_Count_Find
*    Atoms_Create
*    Atoms_Destroy
*    Atoms_Dump
*    Atoms_InstanceIndex_Get
*    Xtr_Atoms_Set
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
* 22-May-95  Cheung	In Atoms_Count_Find, Atoms_InstanceIndex_Get,
*			added  - if atoms_p is NULL, then call Xtr_Atoms_Set  
*		        
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_ATOMS_
#include "atoms.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif


/****************************************************************************
*
*  Function Name:                 Atoms_Copy
*
*    This function makes a copy of the Atoms_t parameter.
*
*  Used to be:
*
*    natoms:
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
*    NULL    if input was NULL handle
*    Address of copy of Atoms_t structure
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Atoms_t *Atoms_Copy
  (
  Atoms_t      *atoms_p                     /* Address of structure to copy */
  )
{
  Atoms_t      *atoms_tmp;                  /* Temporary */

  if (atoms_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_ATOMCOPY, TL_PARAMS, (outbuf,
    "Entering Atoms_Copy, Atoms = %p", atoms_p));

  atoms_tmp = Atoms_Create (Atoms_NumAtoms_Get (atoms_p),
    Atoms_NumDistinct_Get (atoms_p));

  memcpy (Atoms_Atoms_Get (atoms_tmp), Atoms_Atoms_Get (atoms_p),
    Atoms_NumDistinct_Get (atoms_p) * ATOMROWSIZE);
  Array_CopyContents (Atoms_Instance_Get (atoms_p),
    Atoms_Instance_Get (atoms_tmp));
  Atoms_NumDistinct_Put (atoms_tmp, Atoms_NumDistinct_Get (atoms_p));

  DEBUG (R_XTR, DB_ATOMCOPY, TL_PARAMS, (outbuf,
    "Leaving Atoms_Copy, Atoms  = %p", atoms_tmp));

  return atoms_tmp;
}

/****************************************************************************
*
*  Function Name:                 Atoms_Count_Find
*
*    This function gets the number of atoms with a given atomic number.  In
*    the case of a generic halogen, the number all halogens are summed up.
*
*  Used to be:
*
*    a_count:
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
*    Number of occurences of the given atomic number in the molecule
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Atoms_Count_Find
  (
  Xtr_t        *xtr_p,                      /* Molecule to look at */
  U16_t         atomid                      /* Atom id to look up */
  )
{
  Atoms_t      *atoms_p;                    /* Temporary */
  U16_t         num_halogens;               /* Counter */
  U16_t         temp_id;                    /* Atom id to match */
  U16_t         atom;                       /* Counter */

  atoms_p = Xtr_Atoms_Get (xtr_p);

  if (atoms_p == NULL)
    {
    Xtr_Atoms_Set (xtr_p);
    atoms_p = Xtr_Atoms_Get (xtr_p);
    }

  /* Generic halogen */

  if (atomid == GENERIC_HALOGEN)
    {
    for (atom = 0, num_halogens = 0; atom < Atoms_NumDistinct_Get (atoms_p);
         atom++)
      {
      temp_id = Atoms_Atomid_Get (atoms_p, atom);
      if (Atomid_IsHalogen (temp_id) == TRUE)
        num_halogens += Atoms_NumOccurences_Get (atoms_p, atom);
      }

    return num_halogens;
    }

  for (atom = 0; atom < Atoms_NumDistinct_Get (atoms_p); atom++)
    if (atomid == Atoms_Atomid_Get (atoms_p, atom))
      return Atoms_NumOccurences_Get (atoms_p, atom);

  return 0;
}

/****************************************************************************
*
*  Function Name:                 Atoms_Create
*
*    This function creates an Atoms_t structure, it needs to know the
*    number of distinct elements in the molecule in order to allocate
*    the AtomRow_t array.
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
*    Address of new Atoms structure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Atoms_t *Atoms_Create
  (
  U16_t         num_atoms,                  /* Number of atoms in m'cule */
  U16_t         num_distinct                /* Number of distinct atom ids */
  )
{
  Atoms_t      *atoms_p;                    /* Temporary */
  AtomRow_t    *row_p;                      /* Temporary */

  DEBUG (R_XTR, DB_ATOMCREATE, TL_PARAMS, (outbuf,
    "Entering Atoms_Create, # atoms = %u, # distinct = %u", num_atoms,
    num_distinct));

#ifdef _MIND_MEM_
  mind_malloc ("atoms_p", "atoms{1}", &atoms_p, ATOMSSIZE);
#else
  Mem_Alloc (Atoms_t *, atoms_p, ATOMSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_ATOMCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for an Atoms structure in Atoms_Create at %p", atoms_p));

  if (atoms_p == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for an Atoms structure in Atoms_Create");

#ifdef _MIND_MEM_
  mind_malloc ("row_p", "atoms{1}", &row_p, ATOMROWSIZE * num_distinct);
#else
  Mem_Alloc (AtomRow_t *, row_p, ATOMROWSIZE * num_distinct, GLOBAL);
#endif

  DEBUG (R_XTR, DB_ATOMCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for an Atom row array in Atoms_Create at %p", row_p));

  if (row_p == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for an Atom row array in Atoms_Create");

  Atoms_NumDistinct_Put (atoms_p, num_distinct);
  Atoms_Atoms_Put (atoms_p, row_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("Atoms_Instance_Get(atoms_p)", "atoms{1}", Atoms_Instance_Get (atoms_p), num_atoms, WORDSIZE);
#else
  Array_1d_Create (Atoms_Instance_Get (atoms_p), num_atoms, WORDSIZE);
#endif

  DEBUG (R_XTR, DB_ATOMCREATE, TL_PARAMS, (outbuf,
    "Leaving Atoms_Create, Atoms = %p", atoms_p));

  return atoms_p;
}

/****************************************************************************
*
*  Function Name:                 Atoms_Destroy
*
*    This routine destroys an Atoms_t structure.
*
*  Used to be:
*
*    zatoms:
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
void Atoms_Destroy
  (
  Atoms_t      *atoms_p                     /* Structure to destroy */
  )
{
  if (atoms_p == NULL)
    return;

  DEBUG (R_XTR, DB_ATOMDESTROY, TL_PARAMS, (outbuf,
    "Entering Atoms_Destroy, Atoms = %p", atoms_p));

#ifdef _MIND_MEM_
  mind_Array_Destroy ("Atoms_Instance_Get(atoms_p)", "atoms", Atoms_Instance_Get (atoms_p));
#else
  Array_Destroy (Atoms_Instance_Get (atoms_p));
#endif

  DEBUG_DO (DB_ATOMDESTROY, TL_MEMORY,
    {
    memset (Atoms_Atoms_Get (atoms_p), 0,
      Atoms_NumDistinct_Get (atoms_p) * ATOMROWSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("Atoms_Atoms_Get(atoms_p)", "atoms", Atoms_Atoms_Get (atoms_p));
#else
  Mem_Dealloc (Atoms_Atoms_Get (atoms_p),
    Atoms_NumDistinct_Get (atoms_p) * ATOMROWSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_ATOMDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Atom row array in Atoms_Destroy at %p",
    Atoms_Atoms_Get (atoms_p)));

  DEBUG_DO (DB_ATOMDESTROY, TL_MEMORY,
    {
    memset (atoms_p, 0, ATOMSSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("atoms_p", "atoms", atoms_p);
#else
  Mem_Dealloc (atoms_p, ATOMSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_ATOMDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Atoms structure in Atoms_Destroy at %p", atoms_p));

  DEBUG (R_XTR, DB_ATOMDESTROY, TL_PARAMS, (outbuf,
    "Leaving Atoms_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Atoms_Dump
*
*    This routine prints a formatted dump of an Atoms structure.
*
*  Used to be:
*
*    patoms:
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
void Atoms_Dump
  (
  Atoms_t      *atoms_p,                    /* Address of instance to dump */
  FileDsc_t    *filed_p                     /* File to dump to */
  )
{
  FILE         *f;                          /* Temporary */
  U16_t         i, j;                       /* Counters */
  U16_t         link;                       /* For linked list traversal */

  f = IO_FileHandle_Get (filed_p);
  if (atoms_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Atoms\n\n");
    return;
    }

  DEBUG_ADDR (R_ATOMS, DB_ATOMCREATE, atoms_p);
  fprintf (f, "Dump of Atoms structure, # distinct atom ids = %u\n",
    Atoms_NumDistinct_Get (atoms_p));

  fprintf (f, "Id#\t# atoms\tInstances\n---\t-------\t---------\n");
  for (i = 0; i < Atoms_NumDistinct_Get (atoms_p); i++)
    {
    fprintf (f, "%u\t%u\t", Atoms_Atomid_Get (atoms_p, i),
      Atoms_NumOccurences_Get (atoms_p, i));

    for (j = 0, link = Atoms_FirstInstance_Get (atoms_p, i); j <
         Atoms_NumOccurences_Get (atoms_p, i); j++)
      {
      fprintf (f, "%3u ", link);
      link = Atoms_NextInstance_Get (atoms_p, link);
      }

    fprintf (f, "\n");
    }

  fprintf (f, "\n");

  return;
}

/****************************************************************************
*
*  Function Name:                 Atoms_InstanceIndex_Get
*
*    This function returns the atom index of the specified instance of the
*    specified atom.  In the case of generic halogens then all halogens
*    are considered to be specified so it simply goes through them in an
*    implementation defined order.  Note that the first atom has index of 1.
*
*  Used to be:
*
*    a_inst:
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
*    Atom index of instance of atom
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Atoms_InstanceIndex_Get 
  (
  Xtr_t        *xtr_p,                      /* Molecule to look at */
  U16_t         atomid,                     /* Atom id to look up */
  U16_t         instance                    /* Which instance to fetch */
  )
{
  Atoms_t      *atoms_p;                    /* Temporary */
  U16_t         temp_id;                    /* Temporary */
  U16_t         atom;                       /* Counter */
  U16_t         first;                      /* First occurence */
  U16_t         count;                      /* For consistency check */
  Boolean_t     halogen;                    /* Flag */

  first = XTR_INVALID;
  count = 0;
  halogen = FALSE;
  atoms_p = Xtr_Atoms_Get (xtr_p);

  if (atoms_p == NULL)
    {
    Xtr_Atoms_Set (xtr_p);
    atoms_p = Xtr_Atoms_Get (xtr_p);
    }

  for (atom = 0; atom < Atoms_NumDistinct_Get (atoms_p) && first ==
       XTR_INVALID; atom++)
    {
    /* Generic halogen case is a bit trickier */

    if (atomid == GENERIC_HALOGEN)
      {
      temp_id = Atoms_Atomid_Get (atoms_p, atom);
      if (Atomid_IsHalogen (temp_id) == TRUE)
        {
        if (instance > Atoms_NumOccurences_Get (atoms_p, atom))
          {
          instance -= Atoms_NumOccurences_Get (atoms_p, atom);
          count += Atoms_NumOccurences_Get (atoms_p, atom);
          }
        else
          halogen = TRUE;
        }
      }

    if ((atomid == Atoms_Atomid_Get (atoms_p, atom) && atomid !=
        GENERIC_HALOGEN) || halogen == TRUE)
      {
      first = Atoms_FirstInstance_Get (atoms_p, atom);
      count += Atoms_NumOccurences_Get (atoms_p, atom);
      }
    }                    /* End of for-atoms loop */

  ASSERT ((first != (U16_t)XTR_INVALID) && (instance <= count),
    {
    IO_Exit_Error (R_XTR, X_SYNERR,
      "Constraint error in Atoms_InstanceIndex_Find");
    });

  /* First atom has index of 1, not 0 !! */

  for (atom = 1; atom < instance; atom++)
    first = Atoms_NextInstance_Get (atoms_p, first);

  return first;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Atoms_Set
*
*    This routine calculates all the atom information, number of occurences
*    of a distinct atomic number, builds a linked list ordered by atom index.
*
*  Used to be:
*
*    build_atom_info:
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
void Xtr_Atoms_Set
  (
  Xtr_t        *xtr_p                       /* Molecule to analyze */
  )
{
  Atoms_t      *atoms_p;                    /* Atoms struct for this m'cule */
  U16_t         num_atoms;                  /* # atoms in molecule */
  U16_t         atom;                       /* Counter */
  U16_t         num_found;                  /* Counter */
  U16_t         atomid;                     /* Atomic number of atom */
  U16_t         j;                          /* Counter */
  Boolean_t     found;                      /* Flag */
  Array_t       distinct_ids;               /* Distinct atomic numbers found */
  Array_t       first;                      /* Lowest number atom index */
  Array_t       last;                       /* Largest number atom index */
  Array_t       num_occurences;             /* Number of occurences */
  Array_t       instances;                  /* Linked list of instances */

  DEBUG (R_XTR, DB_ATOMSET, TL_PARAMS, (outbuf,
    "Entering Xtr_Atoms_Set, Xtr = %p", xtr_p));

  num_atoms = Xtr_NumAtoms_Get (xtr_p);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&distinct_ids", "atoms{2}", &distinct_ids, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&num_occurences", "atoms{2}", &num_occurences, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&first", "atoms{2}", &first, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&last", "atoms{2}", &last, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&instances", "atoms{2}", &instances, num_atoms, WORDSIZE);
#else
  Array_1d_Create (&distinct_ids, num_atoms, WORDSIZE);
  Array_1d_Create (&num_occurences, num_atoms, WORDSIZE);
  Array_1d_Create (&first, num_atoms, WORDSIZE);
  Array_1d_Create (&last, num_atoms, WORDSIZE);
  Array_1d_Create (&instances, num_atoms, WORDSIZE);
#endif
  Array_Set (&distinct_ids, 0);
  Array_Set (&num_occurences, 0);
  Array_Set (&first, XTR_INVALID);
  Array_Set (&last, XTR_INVALID);
  Array_Set (&instances, XTR_INVALID);

  /* Loop through all the atoms in the molecule
     - Store each new element in distinct_ids to be checked against
     - Keep count of # of distinct elements found so far
     - Keep a linked list with head in first (array), tail in last (array)
       and the middle ones in instances (array).  Keep the count in 
       num_occurences (array).
  */

  for (atom = 0, num_found = 0; atom < num_atoms; atom++)
    {
    atomid = Xtr_Attr_Atomid_Get (xtr_p, atom);
    found = FALSE;

    for (j = 0; j < num_found && !found; j++)
      {
      /* Search for previous occurrence of atom.  If find one, insert in
         linked list.
      */

      if (Array_1d16_Get (&distinct_ids, j) == atomid)
        {
        Array_1d16_Put (&num_occurences, j, Array_1d16_Get (&num_occurences,
          j) + 1);
        Array_1d16_Put (&instances, Array_1d16_Get (&last, j), atom);
        Array_1d16_Put (&last, j, atom);
        found = TRUE;
        }
      }

    /* First occurrence */

    if (found == FALSE)
      {
       Array_1d16_Put (&distinct_ids, num_found, atomid);
       Array_1d16_Put (&first, num_found, atom);
       Array_1d16_Put (&last, num_found, atom);
       Array_1d16_Put (&num_occurences, num_found, 1);
       num_found++;
       }
    }               /* End for-atom loop */

  /* Create an Atoms_t structure to store the information in
     - For each distinct element, copy the atomid (atomic number), first
       instance index and number of occurences into the Atoms_t
     - Copy instances (array) into Atoms_t
  */

  atoms_p = Atoms_Create (num_atoms, num_found);

  for (atom = 0; atom < num_found; atom++)
    {
    Atoms_Atomid_Put (atoms_p, atom, Array_1d16_Get (&distinct_ids, atom));
    Atoms_FirstInstance_Put (atoms_p, atom, Array_1d16_Get (&first, atom));
    Atoms_NumOccurences_Put (atoms_p, atom, Array_1d16_Get (&num_occurences,
      atom));
    }

  Array_CopyContents (&instances, Atoms_Instance_Get (atoms_p));
  Xtr_Atoms_Put (xtr_p, atoms_p);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&distinct_ids", "atoms", &distinct_ids);
  mind_Array_Destroy ("&first", "atoms", &first);
  mind_Array_Destroy ("&last", "atoms", &last);
  mind_Array_Destroy ("&num_occurences", "atoms", &num_occurences);
  mind_Array_Destroy ("instances", "atoms", &instances);
#else
  Array_Destroy (&distinct_ids);
  Array_Destroy (&first);
  Array_Destroy (&last);
  Array_Destroy (&num_occurences);
  Array_Destroy (&instances);
#endif

  DEBUG (R_XTR, DB_ATOMSET, TL_PARAMS, (outbuf,
    "Leaving Xtr_Atoms_Set, new atom addr = %p", atoms_p));

  return;
}
/* End of Xtr_Atoms_Set */
/* End of ATOMS.C */
