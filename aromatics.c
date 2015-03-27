/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     AROMATICS.C
*
*    This module contains the routines for computing the Aromatics component
*    of the XTR.  This component comprises all atoms that are part of 
*    aromatic rings which are defined to be cycles with 1 or more hetero-
*    atoms in them.  The end result is to mark a set of bonds in the XTR
*    Attributes structure as aromatic as opposed to their original single
*    and double bonds.
*
*  Routines:
*
*    Aromatics_Copy
*    Aromatics_Create
*    Aromatics_Destroy
*    Aromatics_Dump
*    Aromat_FindBonds
*    Xtr_Aromat_Set
*    Xtr_Aromat_Bond_Get
*    Xtr_Aromat_Node_Get
*    SAtomsConnect
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
* 22-May-95  Cheung	Added Xtr_Aromat_Node_Get, Xtr_Aromat_Bond_Get
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

#ifndef _H_AROMATICS_
#include "aromatics.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/* Static Routine Prototypes */

static void SAtomsConnect (Xtr_t *, U16_t, U16_t, U8_t);

/****************************************************************************
*
*  Function Name:                 Aromatics_Copy
*
*    This function makes a copy of the Aromatics_t parameter.
*
*  Used to be:
*
*    #aromtc:
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
*    NULL    if input is NULL handle
*    Address of copy if successful
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Aromatics_t *Aromatics_Copy
  (
  Aromatics_t  *aromatics_p                /* Address of instance to copy */
  )
{
  Aromatics_t  *aromat_tmp;                /* Temporary */

  if (aromatics_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_AROMCOPY, TL_PARAMS, (outbuf,
    "Entering Aromatics_Copy, aromatics = %p", aromatics_p));

#ifdef _MIND_MEM_
  mind_malloc ("aromat_tmp1", "aromatics{1}", &aromat_tmp, AROMATICSSIZE);
#else
  Mem_Alloc (Aromatics_t *, aromat_tmp, AROMATICSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_AROMCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for an Aromatics structure in Aromatics_Copy at %p",
    aromat_tmp));

  if (aromat_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for aromatics structure in Aromatics_Copy");

#ifdef _MIND_MEM_
  mind_Array_Copy ("Aromat_Flags_Get(aromat_tmp)", "aromatics{1}", Aromat_Flags_Get (aromatics_p), Aromat_Flags_Get (aromat_tmp));
#else
  Array_Copy (Aromat_Flags_Get (aromatics_p), Aromat_Flags_Get (aromat_tmp));
#endif

  DEBUG (R_XTR, DB_AROMCOPY, TL_PARAMS, (outbuf,
    "Leaving Aromatics_Copy, aromatics = %p", aromat_tmp));

  return aromat_tmp;
}

/****************************************************************************
*
*  Function Name:                 Aromatics_Create
*
*    This function creates a new Aromatics_t.  The flags for the bond and
*    atom flags are initialized to FALSE.
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
*    Address of allocated storage for Aromatics_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Aromatics_t *Aromatics_Create
  (
  U16_t         num_atoms                  /* Number of atoms in the m'cule */
  )
{
  Aromatics_t  *aromatic_tmp;              /* Temporary */

  DEBUG (R_XTR, DB_AROMCREATE, TL_PARAMS, (outbuf,
    "Entering Aromatics_Create, # atoms = %hu", num_atoms));

#ifdef _MIND_MEM_
  mind_malloc ("aromatic_tmp", "aromatics{2}", &aromatic_tmp, AROMATICSSIZE);
#else
  Mem_Alloc (Aromatics_t *, aromatic_tmp, AROMATICSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_AROMCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for an Aromatics structure in Aromatics_Create at %p",
    aromatic_tmp));

  if (aromatic_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for aromatics structure in Aromatics_Create");

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("Aromat_Flags_Get(aromatic_tmp)", "aromatics{2}", Aromat_Flags_Get (aromatic_tmp), num_atoms,
    MX_NEIGHBORS + 1, BITSIZE);
#else
  Array_2d_Create (Aromat_Flags_Get (aromatic_tmp), num_atoms,
    MX_NEIGHBORS + 1, BITSIZE);
#endif
  Array_Set (Aromat_Flags_Get (aromatic_tmp), FALSE);

  DEBUG (R_XTR, DB_AROMCREATE, TL_PARAMS, (outbuf,
    "Leaving Aromatics_Create, Aromatics = %p", aromatic_tmp));

  return aromatic_tmp;
}

/****************************************************************************
*
*  Function Name:                 Aromatics_Destroy
*
*    This function destroys the Aromatics_t parameter.
*
*  Used to be:
*
*    zaromtc:
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
void Aromatics_Destroy
  (
  Aromatics_t  *aromatic_p                 /* Aromatic structure to delete */
  )
{
  if (aromatic_p == NULL)
    return;

  DEBUG (R_XTR, DB_AROMDESTROY, TL_PARAMS, (outbuf,
    "Entering Aromatics_Destroy, aromatics = %p", aromatic_p));

#ifdef _MIND_MEM_
  mind_Array_Destroy ("Aromat_Flags_Get(aromatic_p)", "aromatics", Aromat_Flags_Get (aromatic_p));
#else
  Array_Destroy (Aromat_Flags_Get (aromatic_p));
#endif

  DEBUG_DO (DB_AROMDESTROY, TL_MEMORY,
    {
    memset (aromatic_p, 0, AROMATICSSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("aromatic_p", "aromatics", aromatic_p);
#else
  Mem_Dealloc (aromatic_p, AROMATICSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_AROMDESTROY, TL_MEMORY, (outbuf,
    "Deallocated Aromatics structure in Aromatics_Destroy at %p", aromatic_p));

  DEBUG (R_XTR, DB_AROMDESTROY, TL_PARAMS, (outbuf,
    "Leaving Aromatics_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Aromatics_Dump
*
*    This routine prints a formatted dump of an Aromatics_t.
*
*  Used to be:
*
*    paromtc:
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
void Aromatics_Dump
  (
  Aromatics_t   *aromatic_p,                 /* Instance to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (aromatic_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Aromatics\n\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_AROMCREATE, aromatic_p);
  fprintf (f, "Dump of Aromatics structure - # atoms : %lu\n",
    Array_Rows_Get (Aromat_Flags_Get (aromatic_p)));
  fprintf (f, "Bonds, with aromatic atom flags in last column\n");
  Array_Dump (Aromat_Flags_Get (aromatic_p), filed_p);

  return;
}

/****************************************************************************
*
*  Function Name:                 Aromat_FindBonds
*
*    This routine looks for aromatic bonds in a molecule.  Resonant bonds must
*    be replaced by corresponding single and double bonds.  The algorithm is
*    as follows: call ResonantBonds_Find on a modified XTR, constructed as
*    follows - Insert double bonds between pairs of single bonds at the
*    following hetero-atoms: N, O, Si, P, S, and Se with 2 restrictions.
*    1) Ignore N's that have a bond multiplicity > 1 or degree other than 3.
*    2) Ignore atoms satisfying the above conditions if they are adjacent.
*
*  Used to be:
*
*    arombnd:
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
void Aromat_FindBonds
  (
  Xtr_t        *xtr_p,                 /* Molecule to search */
  Array_t      *aromatbonds_p          /* 2-d bit Array of results  */
  )
{
  Xtr_t       *xtr_tmp;                /* Copy of XTR with psuedoatoms added */
  U16_t        atom;                   /* Counter (atom index) */
  U16_t        atomid;                 /* Atom identifier */
  U16_t        neighid;                /* Neighbor atom indentifier */
  U16_t        num_atoms;              /* # atoms in molecule */
  U16_t        new_num_atoms;          /* # atoms counting pseudo-atoms */
  U16_t        k;                      /* Counter */
  U16_t        last;                   /* Counter */
  U16_t        last_pseudoatom;        /* Counter */
  U8_t         neigh;                  /* Counter */
  U8_t         num_neighbors;          /* Compiler bug */
  Boolean_t    ok;                     /* Flag */
  Array_t      num_pseudoatoms;        /* 1-d byte array, # psuedo-atoms
    associated with a particular atom */
  Array_t      turnoff;                /* 1-d bit array, atoms to ignore */
  Array_t      resbonds;               /* 2-d bit array of resonant bonds */
  Array_t      map;                    /* 1-d word array, maps atoms to
    pseudo-atoms */

  DEBUG (R_XTR, DB_AROMFINDBONDS, TL_PARAMS, (outbuf,
    "Entering Aromat_FindBonds, Xtr = %p, bonds = %p", xtr_p, aromatbonds_p));

  TRACE_DO (DB_AROMFINDBONDS, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "XTR in Aromat_FindBonds");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&num_pseudoatoms", "aromatics{3}", &num_pseudoatoms, num_atoms, BYTESIZE);
  mind_Array_1d_Create ("&turnoff", "aromatics{3}", &turnoff, num_atoms, BITSIZE);
#else
  Array_1d_Create (&num_pseudoatoms, num_atoms, BYTESIZE);
  Array_1d_Create (&turnoff, num_atoms, BITSIZE);
#endif
  Array_Set (&num_pseudoatoms, 0);
  Array_Set (aromatbonds_p, FALSE);

  /* After initializing the data-structures for the algorithm
     - Find all hetero-atoms in rings and count the number of single bonds,
       the number of pseudo-atoms is the number of single bonds (less 1?)
       - For Nitrogen atoms, only triple single bonds allow it to qualify
  */

  for (atom = 0; atom < num_atoms; atom++)
    {
    if (Ring_AtomIn (xtr_p, atom) == FALSE)
      continue;

    atomid = Xtr_Attr_Atomid_Get (xtr_p, atom);
    if (atomid == OXYGEN || atomid == SILICON || atomid == PHOSPHORUS ||
        atomid == SULFUR || atomid == SELENIUM)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0, k = 0; neigh < num_neighbors; neigh++)
        if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) == BOND_SINGLE)
          k++;

      if (k > 0)
        Array_1d8_Put (&num_pseudoatoms, atom, k - 1);
      }
    else
      if (atomid == NITROGEN)
        if (Xtr_Attr_NumNeighbors_Get (xtr_p, atom) == 3)
          {
          ok = TRUE;
          for (neigh = 0; neigh < 3; neigh++)
            if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) != BOND_SINGLE)
              ok = FALSE;

          if (ok == TRUE)
            Array_1d8_Put (&num_pseudoatoms, atom, 2);
          }
    }                                  /* End of for-atom loop */

  /* Check for adjacent hetero-atoms.  We don't know they share a cycle,
     but it is likely.
  */

  Array_Set (&turnoff, FALSE);
  for (atom = 0; atom < num_atoms; atom++)
    if (Array_1d8_Get (&num_pseudoatoms, atom) != 0)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0; neigh < num_neighbors; neigh++)
        {
        neighid = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);
        if (Array_1d8_Get (&num_pseudoatoms, neighid) != 0)
          {
          Array_1d1_Put (&turnoff, atom, TRUE);
          Array_1d1_Put (&turnoff, neighid, TRUE);
          }
        }
      }

  /* Count the number of psuedo-atoms so we can expand the XTR.
     Need some arrays with the new number of atoms in it.
  */

  for (atom = 0, k = 0; atom < num_atoms; atom++)
    {
    if (Array_1d1_Get (&turnoff, atom) == TRUE)
      Array_1d8_Put (&num_pseudoatoms, atom, 0);

    k += Array_1d8_Get (&num_pseudoatoms, atom);
    }

  new_num_atoms = num_atoms + k;
  xtr_tmp = Xtr_CopyExpand (xtr_p, k);

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&map", "aromatics{3}", &map, new_num_atoms, WORDSIZE);
  mind_Array_2d_Create ("&resbonds", "aromatics{3}", &resbonds, new_num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_1d_Create (&map, new_num_atoms, WORDSIZE);
  Array_2d_Create (&resbonds, new_num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  Array_Set (&resbonds, FALSE);

  for (atom = 0; atom < num_atoms; atom++)
    Array_1d16_Put (&map, atom, atom);

  for (atom = 0, last_pseudoatom = num_atoms; atom < num_atoms; atom++)
    {
    if (Array_1d8_Get (&num_pseudoatoms, atom) != 0)
      {
      /* The pseudo-atoms are linked by single bonds to atoms that are
         linked to the hetero-atom in question.  The hetero-atom is then
         unlinked from this single-bonded neighbor.  This creates two
         "ends" which get tied up below.
      */

      for (k = 0, last = 0; k < Array_1d8_Get (&num_pseudoatoms, atom); k++)
        {
        Xtr_Attr_Atomid_Put (xtr_tmp, last_pseudoatom + k, VARIABLE_START);
        Array_1d16_Put (&map, last_pseudoatom + k, atom);

        num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
        for (neigh = last + 1; neigh < num_neighbors &&
             Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) != BOND_SINGLE;
             neigh++)
          /* Empty loop body */ ;

        last = neigh;
        neighid = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);
        Xtr_Attr_NeighborId_Put (xtr_tmp, last_pseudoatom + k, 0, neighid);
        Xtr_Attr_NeighborBond_Put (xtr_tmp, last_pseudoatom + k, 0,
          BOND_SINGLE);
        Xtr_Attr_NumNeighbors_Put (xtr_tmp, last_pseudoatom + k, 1);
        Xtr_Attr_NeighborId_Put (xtr_tmp, neighid,
          Xtr_Attr_NeighborIndex_Find (xtr_p, neighid, atom),
          last_pseudoatom + k);
        Xtr_Attr_NeighborBond_Put (xtr_tmp, neighid,
          Xtr_Attr_NeighborIndex_Find (xtr_p, neighid, atom), BOND_SINGLE);

        /* We ignore stereo identification throughout the aromatic bond
           determination process.
        */

        Xtr_Attr_NeighborId_Put (xtr_tmp, atom, neigh, XTR_INVALID);
        Xtr_Attr_NeighborBond_Put (xtr_tmp, atom, neigh, BOND_NONE);
        Xtr_Attr_NeighborStereo_Put (xtr_tmp, atom, neigh, BOND_DIR_INVALID);
        Xtr_Attr_NumNeighbors_Put (xtr_tmp, atom, Xtr_Attr_NumNeighbors_Get (
          xtr_tmp, atom) - 1);
        }                              /* End of for-k loop */

      /* Compress the atom hetero-atom, and then add a double bond between it
         and the pseudo-atom, this closes the two "ends" noted above and
         completes the process described in the routine header.
      */

      Xtr_CompressRow (xtr_tmp, atom);
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0; neigh < num_neighbors; neigh++)
        if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) != BOND_SINGLE)
          {
          for (k = 0; k < Array_1d8_Get (&num_pseudoatoms, atom); k++)
            SAtomsConnect (xtr_tmp, last_pseudoatom + k,
              Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh),
              Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh));
          }

      for (k = 0; k < Array_1d8_Get (&num_pseudoatoms, atom); k++)
        {
        SAtomsConnect (xtr_tmp, last_pseudoatom + k, atom, BOND_DOUBLE);
        for (neigh = k + 1; neigh < Array_1d8_Get (&num_pseudoatoms, atom);
             neigh++)
          SAtomsConnect (xtr_tmp, last_pseudoatom + k, last_pseudoatom + neigh,
            BOND_DOUBLE);
        }

      last_pseudoatom += Array_1d8_Get (&num_pseudoatoms, atom);
      }                      /* End if-num_pseudoatoms[atom] */
    }                        /* End for-atoms loop */

  TRACE_DO (DB_AROMFINDBONDS, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In Aromat_FindBonds");
    Xtr_Dump (xtr_tmp, &GTraceFile);
    });

  /* This modified XTR needs to have its' rings recomputed and then to have
     its resonant bonds set.
  */

  Xtr_Rings_Set (xtr_tmp);
  ResonantBonds_Find (xtr_tmp, &resbonds);

  /* All remapped atoms (must have been hetero-atoms) that have resonant
     bonds are marked as actually having aromatic bonds in the output
     array.
  */

  for (atom = 0; atom < new_num_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_tmp, atom);
    for (neigh = 0; neigh < num_neighbors; neigh++)
      {
      if (Array_2d1_Get (&resbonds, atom, neigh) == TRUE)
        {
        neighid = Xtr_Attr_NeighborId_Get (xtr_tmp, atom, neigh);
        if (Array_1d16_Get (&map, atom) != Array_1d16_Get (&map, neighid))
          Array_2d1_Put (aromatbonds_p, Array_1d16_Get (&map, atom),
            Xtr_Attr_NeighborIndex_Find (xtr_p, Array_1d16_Get (&map, atom),
            Array_1d16_Get (&map, neighid)), TRUE);
        }
      }
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&map", "aromatics", &map);
  mind_Array_Destroy ("&resbonds", "aromatics", &resbonds);
  mind_Array_Destroy ("&turnoff", "aromatics", &turnoff);
  mind_Array_Destroy ("&num_pseudoatoms", "aromatics", &num_pseudoatoms);
#else
  Array_Destroy (&map);
  Array_Destroy (&resbonds);
  Array_Destroy (&turnoff);
  Array_Destroy (&num_pseudoatoms);
#endif
  Xtr_Destroy (xtr_tmp);

  TRACE_DO (DB_AROMFINDBONDS, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In Aromat_FindBonds");
    Array_Dump (aromatbonds_p, &GTraceFile);
    });

  DEBUG (R_XTR, DB_AROMFINDBONDS, TL_PARAMS, (outbuf,
    "Leaving Aromat_FindBonds, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Aromat_Bond_Get
*
*     This routine determines if the mth bond of node n is aromatic.
*
*  Used to be:
*
*    armtcbd
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
*    TRUE   or
*    FALSE
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Xtr_Aromat_Bond_Get
  (
  Xtr_t		*xtr_p,
  U16_t		node,
  U16_t		bond
  )
{
  Aromatics_t		*aromatic_p;

  aromatic_p = Xtr_Aromatics_Get (xtr_p);
 
  if (aromatic_p == NULL)
    {
    Xtr_Aromat_Set (xtr_p);
    aromatic_p = Xtr_Aromatics_Get (xtr_p);
    }

  return Array_2d1_Get (Aromat_Flags_Get (aromatic_p), node, bond);
}
/****************************************************************************
*
*  Function Name:                 Xtr_Aromat_Node_Get
*
*     This routine determines if the nth atom has one or more aromatic bonds.
*
*  Used to be:
*
*    armtcnd
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
*    TRUE	or
*    FALSE
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Xtr_Aromat_Node_Get
  (
  Xtr_t		*xtr_p,
  U16_t		node
  )
{
  Aromatics_t		*aromatic_p;

  aromatic_p = Xtr_Aromatics_Get (xtr_p);

  if (aromatic_p == NULL)
    {
    Xtr_Aromat_Set (xtr_p);
    aromatic_p = Xtr_Aromatics_Get (xtr_p);
    }

  return Array_2d1_Get (Aromat_Flags_Get (aromatic_p), node, NODE_COLUMN);
}
/****************************************************************************
*
*  Function Name:                 Xtr_Aromat_Set
*
*    This routine sets up an XTR to find the aromatic bonds on.  If the
*    resonant bonds have been set (should know! this before call) then it
*    fixes them on a copy before calling Aromat_FindBonds.  The Aromatics_t
*    structure is entered into the XTR.
*
*  Used to be:
*
*    build_aromaticity:
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
void Xtr_Aromat_Set
  (
  Xtr_t        *xtr_p                  /* Xtr to enhance */
  )
{
  Aromatics_t  *aromatic_tmp;          /* Aromatics data-structure */
  Xtr_t        *xtr_tmp;               /* Temporary for enhancement */
  U16_t         atom;                  /* Counter */
  U8_t          bond;                  /* Counter */
  U16_t         atom_save, bond_save;  /* Temporaries */
  U16_t         num_atoms;             /* # atoms in M'cule */
  U8_t          num_neighbors;         /* Compiler bug */
  Boolean_t     resonant_bonds;        /* Flag */
  Array_t       bonds;                 /* Bond array */

  DEBUG (R_XTR, DB_AROMSET, TL_PARAMS, (outbuf,
    "Entering Xtr_Aromat_Set, Xtr = %p", xtr_p));

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
  aromatic_tmp = Aromatics_Create (num_atoms);

  /* Search for a resonant bond to see if there can be any aromatic rings.
     Save results for first search.
  */

  for (atom = 0, resonant_bonds = FALSE, atom_save = XTR_INVALID,
       bond_save = BOND_DIR_INVALID; atom < num_atoms && !resonant_bonds;
       atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (bond = 0; bond < num_neighbors && resonant_bonds == FALSE; bond++)
      {
      if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, bond) == BOND_RESONANT)
        {
        atom_save = atom;
        bond_save = bond;
        resonant_bonds = TRUE;
        }
      }
    }

  /* Create the output array.  If we have already set the resonant bonds
     (why don't we know if they have been set?), then make a copy and
     "fix" the bonds before finding the aromatic bonds
  */

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "aromatics{4}", &bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  if (resonant_bonds == TRUE)
    {
    xtr_tmp = Xtr_Copy (xtr_p);
    ResonantBonds_Fix (xtr_tmp, atom_save, bond_save, BOND_SINGLE);
    Aromat_FindBonds (xtr_tmp, &bonds);
    Xtr_Destroy (xtr_tmp);
    }
  else
    {
    if (Xtr_Rings_Get (xtr_p) == NULL)
      Xtr_Rings_Set (xtr_p);

    Aromat_FindBonds (xtr_p, &bonds);
    }

  TRACE_DO (DB_AROMSET, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In Xtr_Aromat_Set");
    Array_Dump (&bonds, &GTraceFile);
    });

  /* Set the aromatic bonds and mark any atoms that have aromatic bonds */

  for (atom = 0; atom < Xtr_NumAtoms_Get (xtr_p); atom++)
    for (bond = 0; bond < Xtr_Attr_NumNeighbors_Get (xtr_p, atom); bond++)
      {
      Aromat_Bond_Put (aromatic_tmp, atom, bond,
        Array_2d1_Get (&bonds, atom, bond));
      if (Array_2d1_Get (&bonds, atom, bond) == TRUE)
        Aromat_Node_Put (aromatic_tmp, atom, TRUE);
      }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bonds", "aromatics", &bonds);
#else
  Array_Destroy (&bonds);
#endif
  Xtr_Aromatics_Put (xtr_p, aromatic_tmp);

  DEBUG (R_XTR, DB_AROMSET, TL_PARAMS, (outbuf,
    "Leaving Xtr_Aromatics_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SAtomsConnect
*
*    This routine connects two atoms in an XTR.
*
*    Note that the ASSERT relies on MX_NEIGHBORS being 6.
*
*  Used to be:
*
*    connect:
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
static void SAtomsConnect
  (
  Xtr_t        *xtr_p,                     /* Molecule to use */
  U16_t         atom1,                     /* One atom to connect */
  U16_t         atom2,                     /* Another atom to connect */
  U8_t          bondsize                   /* Multiplicity of bond connctn */
  )
{
  U16_t         neigh_cnt1;                /* Temporary */
  U16_t         neigh_cnt2;                /* Temporary */

  neigh_cnt1 = Xtr_Attr_NumNeighbors_Get (xtr_p, atom1);
  neigh_cnt2 = Xtr_Attr_NumNeighbors_Get (xtr_p, atom2);

  ASSERT (neigh_cnt1 <= MX_NEIGHBORS - 1 && neigh_cnt2 <= MX_NEIGHBORS - 1,
    {
    IO_Exit_Error (R_XTR, X_SYNERR,
      "Implementation constraint, max 6 neighbors");
    });

  Xtr_Attr_NeighborId_Put (xtr_p, atom1, neigh_cnt1, atom2);
  Xtr_Attr_NeighborId_Put (xtr_p, atom2, neigh_cnt2, atom1);
  Xtr_Attr_NeighborBond_Put (xtr_p, atom1, neigh_cnt1, bondsize);
  Xtr_Attr_NeighborBond_Put (xtr_p, atom2, neigh_cnt2, bondsize);
  Xtr_Attr_NumNeighbors_Put (xtr_p, atom1, neigh_cnt1 + 1);
  Xtr_Attr_NumNeighbors_Put (xtr_p, atom2, neigh_cnt2 + 1);

  return;
}
/* End of SAtomsConnect */
/* End of AROMATICS.C */


