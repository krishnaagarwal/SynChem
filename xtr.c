/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     XTR.C
*
*    This module is the code for the abstract data-type, XTR.  This is 
*    a different and more complete representation of a molecule than a TSD.
*    There are many related data-structures, each has their own module.
*    Only the Attributes data-structure and the routines that operate on the
*    XTR as a whole are in here.
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PL/I code),
*    - and from John Rose's C code.
*
*  Routines:
*
*    Xtr_Attr_Atoms_Link
*    Xtr_Attr_NeighborBond_Find
*    Xtr_Attr_NeighborIndex_Find
*    Xtr_Attr_NeighborStereo_Find
*    Xtr_Attr_NextNonHydrogen_Get
*    Xtr_Attr_NumNonHydrogen_Get
*    Xtr_Attr_NonHydrogenNeighbor_Get
*    Xtr_Attr_ResonantBonds_Set
*    Xtr_CompressRow
*    Xtr_ConnectedSize_Find
*    Xtr_Copy
*    Xtr_CopyExpand
*    Xtr_CopySubset_Atom
*    Xtr_CopySubset
*    Xtr_Create
*    Xtr_Destroy
*    Xtr_Dump
*    Xtr_FuncGroups_Destroy
*    Xtr_Rings_NumRingSys_Get
*    Xtr_Rings_Set
*    Xtr_Ringdef_Set
*    SAtomCopy
*    SVisit
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
* 10-Nov-96  Krebsbach  Vector should be based on the original Xtr, not the 
*                       subset xtr.  Moved vector set code to Xtr_Ringdef_Set 
*                       from Ringdef_PrimeCycles_Find.
* 18-May-95  Cheung	Xtr_Attr_NonHydrogenNeighbor_Get is added.
* 22-May-95  Cheung	Added Xtr_FuncGroups_Destroy
* 23-May-95  Cheung     Added Xtr_Rings_NumRingSys_Get
* 27-Jun-95  Cheung	Added Xtr_Attr_ResonantBonds_Set in routine 
*			Xtr_Attr_NeighborBond_Find
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

#ifndef _H_XTR_
#include "xtr.h"
#endif

/* Static routine prototypes */

static void SAtomCopy    (Xtr_t *, Xtr_t *, U16_t, U16_t);
static void SXtrCompress (Xtr_t *);
static void SVisit       (Xtr_t *, Array_t *, U16_t, U16_t *);


/****************************************************************************
*
*  Function Name:                 Xtr_Attr_Atoms_Link
*
*    This routine creates an adjaceny between atom and neighbor with a
*    bond of bondsize.  Of course neighbor is an atom index, this routine
*    makes a neighbor index sensible.  No error checking is done.
*
*  Used to be:
*
*    $bndmlt:
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
void Xtr_Attr_Atoms_Link
  (
  Xtr_t        *xtr_p,                     /* Molecule to look in */
  U16_t         atom,                      /* Atom we are looking at */
  U16_t         neighbor,                  /* Neighbor - bond is interesting */
  U8_t          bondsize                   /* Strength of the bond */
  )
{
  U8_t          index;                     /* Neighbor index in atom of
                                              neighbor. */

  ASSERT (bondsize != BOND_NONE,
    {
    IO_Exit_Error (R_XTR, X_SYNERR,
      "Bondsize is zero in Xtr_Attr_Atoms_Link\n");
    });

  index = Xtr_Attr_NeighborIndex_Find (xtr_p, atom, neighbor);
  Xtr_Attr_NeighborBond_Put (xtr_p, atom, index, bondsize);

  index = Xtr_Attr_NeighborIndex_Find (xtr_p, neighbor, atom);
  Xtr_Attr_NeighborBond_Put (xtr_p, neighbor, index, bondsize);

  return;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Attr_NeighborBond_Find
*
*    This function returns the multiplicity (value) of the bond between 
*    the atom and the neighbor.  The neighbor is treated as an atom index
*    value and not a neighbor index value.  If the neighbor is not adjacent
*    to the atom, then XTR_INVALID is returned.
*
*  Used to be:
*
*    bndmulr:, bndmult:
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
*    XTR_INVALID - neighbor is not adjacent to atom
*    <value> - multiplicy of bond between atom and neighbor
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U8_t Xtr_Attr_NeighborBond_Find
  (
  Xtr_t        *xtr_p,                     /* Molecule to look in */
  U16_t         atom,                      /* Atom we are looking at */
  U16_t         neighbor                   /* Neighbor - bond is interesting */
  )
{
  U8_t          index;                     /* Counter */

  index = Xtr_Attr_NeighborIndex_Find (xtr_p, atom, neighbor);
  if (index != BOND_DIR_INVALID)
    return Xtr_Attr_NeighborBond_Get (xtr_p, atom, index);

  return BOND_NONE;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Attr_NeighborIndex_Find
*
*    This function returns the index into the set of adjacent atoms of the
*    atom that the neighbor occupies.  If the neighbor is not adjacent, then
*    an invalid index is returned.
*
*  Used to be:
*
*    bndindx:
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
*    BOND_DIR_INVALID - neighbor is not adjacent to atom
*    <value> - index in the neighbor array of the atom that neighbor occupies
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U8_t Xtr_Attr_NeighborIndex_Find
  (
  Xtr_t        *xtr_p,                     /* Molecule to look in */
  U16_t         atom,                      /* Atom we are looking at */
  U16_t         neighbor                   /* Neighbor - bond is interesting */
  )
{
  U8_t          i;                         /* Counter */

  if (atom == XTR_INVALID || neighbor == XTR_INVALID)
    return BOND_DIR_INVALID;

  for (i = 0; i < MX_NEIGHBORS; i++)
    if (Xtr_Attr_NeighborId_Get (xtr_p, atom, i) == neighbor)
      return i;

  return BOND_DIR_INVALID;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Attr_NeighborStereo_Find
*
*    This function returns the stereo attribute of a bond between an
*    atom and its neighbor.  It will look up the neighbor by atom index
*    and not by neighbor index.
*
*  Used to be:
*
*    bnddrct:, rlocant:, drctbnd:
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
*    XTR_INVALID - neighbor is not a "neighbor" of atom
*    <value> - stereo attribute of bond specified
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U8_t Xtr_Attr_NeighborStereo_Find
  (
  Xtr_t        *xtr_p,                     /* Molecule to look in */
  U16_t         atom,                      /* Atom we are looking at */
  U16_t         neighbor                   /* Neighbor - bond is interesting */
  )
{
  U8_t          index;                     /* Index tmp. */

  index = Xtr_Attr_NeighborIndex_Find (xtr_p, atom, neighbor);
  if (index != BOND_DIR_INVALID)
    return Xtr_Attr_NeighborStereo_Get (xtr_p, atom, index);

  return BOND_DIR_INVALID;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Attr_NextNonHydrogen_Get
*
*    This function returns the atom index of the specified neighbor of 
*    the specified atom.  If such does not exist, then XTR_INVALID is
*    returned, same if atom or index is invalid.
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
*    XTR_INVALID - couldn't find index non-hydrogen adjacent atoms
*    <value> - atom index of index'th non-hydrogen adjacent atom
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Xtr_Attr_NextNonHydrogen_Get
  (
  Xtr_t        *xtr_p,                     /* Molecule to look at */
  U16_t         atom,                      /* Atom to look at */
  U8_t          index                      /* Which one to find */
  )
{
  U16_t         neighbor;                  /* Neighbor atom index */
  U8_t          i, cnt;                    /* Counters */
  U8_t          num_neighbors;             /* Compiler bug */

  if (atom == XTR_INVALID || index == XTR_INVALID)
    return XTR_INVALID;

  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
  for (i = 0, cnt = 0; i < num_neighbors; i++)
    {
    neighbor = Xtr_Attr_NeighborId_Get (xtr_p, atom, i);
    if (Xtr_Attr_Atomid_Get (xtr_p, neighbor) != HYDROGEN)
      cnt++;

    if (cnt == index)
      return neighbor;
    }

  return XTR_INVALID;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Attr_NumNonHydrogen_Get
*
*    This function looks calculates the number of non-hydrogen atoms
*    adjacent to the specified atom.  Returns XTR_INVALID if the atom
*    is invalid.
*
*  Used to be:
*
*    nonhdeg:
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
*    0 - No non-hydrogen atoms adjacent to atom, or atom was invalid
*    <value> - Number of non-hydrogen atoms adjacent to atom
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U8_t Xtr_Attr_NumNonHydrogen_Get
  (
  Xtr_t        *xtr_p,                     /* Molecule to look at */
  U16_t         atom                       /* Atom to get degree of */
  )
{
  U16_t         neighbor;                  /* Id on other side of bond */
  U8_t          i, cnt;                    /* Counters */
  U8_t          num_neighbors;             /* Compiler bug */

  if (atom == XTR_INVALID)
    return 0;

  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
  for (i = 0, cnt = 0; i < num_neighbors; i++)
    {
    neighbor = Xtr_Attr_NeighborId_Get (xtr_p, atom, i);
    if (Xtr_Attr_Atomid_Get (xtr_p, neighbor) != HYDROGEN)
      cnt++;
    }

  return cnt;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Attr_NonHydrogenNeighbor_Get
*
*    This function returns the index(th) nonhydrogen neighbor of atom.
*
*  Used to be:
*
*    nonhnbr
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
*    XTR_INVALID - No neighbor is found, or atom was invalid
*    <value> - Neighbor Id
*
*  Side Effects:
*
*    N/A
*
*****************************************************************************/
U16_t Xtr_Attr_NonHydrogenNeighbor_Get
  (
  Xtr_t		*xtr_p,
  U16_t		atom,
  U16_t		index
  )
{
  U16_t         neighbor;                  /* Id on other side of bond */
  U8_t          i, cnt;                    /* Counters */
  U8_t          num_neighbors;            

  if (atom == XTR_INVALID || index == 0)
    return XTR_INVALID;

  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
  for (i = 0, cnt = 0; i < num_neighbors; i++)
    {
    neighbor = Xtr_Attr_NeighborId_Get (xtr_p, atom, i);
    if (Xtr_Attr_Atomid_Get (xtr_p, neighbor) != HYDROGEN)
      cnt++;
    if (cnt == index)
       return neighbor;
    }
  return XTR_INVALID; 
}

/****************************************************************************
*
*  Function Name:                 Xtr_Attr_ResonantBonds_Set
*
*    This routine knows how to call ResonantBonds_Find and then 
*    reflect the results back into the XTR.  This routine should be called
*    by any routine which creates an XTR and will want to use resonant bonds.
*    Must call Xtr_Rings_Set before calling ResonantBonds_Find because it
*    will need the ring information.
*
*  Used to be:
*
*    compute_res_bonds:
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
void Xtr_Attr_ResonantBonds_Set
  (
  Xtr_t        *xtr_p                      /* Molecule to update */
  )
{
  U16_t         i, j;                      /* Counters */
  Array_t       bonds;                     /* Array for bond results */

  DEBUG (R_XTR, DB_XTRRESBONDSET, TL_PARAMS, (outbuf,
    "Entering Xtr_Attr_ResonantBonds_Set, Xtr = %p", xtr_p));

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "xtr{1}", &bonds, Xtr_NumAtoms_Get (xtr_p), MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&bonds, Xtr_NumAtoms_Get (xtr_p), MX_NEIGHBORS, BITSIZE);
#endif

  Xtr_Rings_Set (xtr_p);
  ResonantBonds_Find (xtr_p, &bonds);

  for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
    for (j = 0; j < MX_NEIGHBORS; j++)
      if (Array_2d1_Get (&bonds, i, j))
        Xtr_Attr_NeighborBond_Put (xtr_p, i, j, BOND_RESONANT);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bonds", "xtr", &bonds);
#else
  Array_Destroy (&bonds);
#endif

  DEBUG (R_XTR, DB_XTRRESBONDSET, TL_PARAMS, (outbuf,
    "Leaving Xtr_Attr_ResonantBonds_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Xtr_CompressRow
*
*    This routine compresses a single atom in a molecule.  It gets rid of
*    neighbor indexes that have XTR_INVALID for atom id, or BOND_NONE
*    for bond type, ...
*
*  Used to be:
*
*    $ljust:
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
void Xtr_CompressRow
  (
  Xtr_t        *xtr_p,                     /* Molecule to compress */
  U16_t         atom                       /* Row to compress */
  )
{
  U8_t          good;                      /* Counter */
  U8_t          neigh;                     /* Counter */
  XtrRow_t      rowtmp;                    /* Temporary */

  DEBUG (R_XTR, DB_XTRCOMPRESS, TL_PARAMS, (outbuf,
    "Entering Xtr_CompressRow, Xtr  = %p, row = %u", xtr_p, atom));

  for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
    {
    rowtmp.neighbors[neigh].id = XTR_INVALID;
    rowtmp.neighbors[neigh].bond = BOND_NONE;
    rowtmp.neighbors[neigh].stereo = BOND_DIR_INVALID;
    }

  for (neigh = 0, good = 0; neigh < MX_NEIGHBORS; neigh++)
    if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) != BOND_NONE)
      {
      rowtmp.neighbors[good] = xtr_p->attributes[atom].neighbors[neigh];
      good++;
      }

  for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
    xtr_p->attributes[atom].neighbors[neigh] = rowtmp.neighbors[neigh];

  Xtr_Attr_NumNeighbors_Put (xtr_p, atom, good);

  DEBUG (R_XTR, DB_XTRCOMPRESS, TL_PARAMS, (outbuf,
    "Leaving Xtr_CompressRow, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Xtr_ConnectedSize_Find
*
*    This function finds the number of atoms in the connected component
*    of the XTR starting from a given node.
*
*  Used to be:
*
*    nreach:
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
U16_t Xtr_ConnectedSize_Find
  (
  Xtr_t        *xtr_p,                     /* Handle to XTR to examine */
  U16_t         atom                       /* Atom index to start at */
  )
{
  U16_t         count;                     /* Output - size of component */
  Array_t       visited;                   /* 1d-bit, flags for visted atoms */

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&visited", "xtr{2}", &visited, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
  Array_Set (&visited, FALSE);
  count = 0;

  SVisit (xtr_p, &visited, atom, &count);
  mind_Array_Destroy ("&visited", "xtr", &visited);
#else
  Array_1d_Create (&visited, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
  Array_Set (&visited, FALSE);
  count = 0;

  SVisit (xtr_p, &visited, atom, &count);
  Array_Destroy (&visited);
#endif

  return count;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Copy
*
*    This routine makes a copy of an XTR, with all associated
*    data-structures.
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
*    Address of copy of XTR
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Xtr_t *Xtr_Copy
  (
  Xtr_t        *xtr_p                      /* Molecule to copy from */
  )
{
  Xtr_t        *xtr_tmp;                   /* Temporary ptr */
  U16_t         i;                         /* Counter */

  if (xtr_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_XTRCOPY, TL_PARAMS, (outbuf,
    "Entering Xtr_Copy, Xtr = %p", xtr_p));

  xtr_tmp = Xtr_Create (Xtr_NumAtoms_Get (xtr_p));

  /* Copy all the atoms one by one */

  for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
    SAtomCopy (xtr_p, xtr_tmp, i, i);

  Xtr_Aromatics_Put (xtr_tmp, Aromatics_Copy (Xtr_Aromatics_Get (xtr_p)));
  Xtr_Atoms_Put (xtr_tmp, Atoms_Copy (Xtr_Atoms_Get (xtr_p)));
  Xtr_FuncGroups_Put (xtr_tmp, FuncGroups_Copy (Xtr_FuncGroups_Get (xtr_p)));
  Xtr_Name_Put (xtr_tmp, Name_Copy (Xtr_Name_Get (xtr_p)));
  Xtr_Rings_Put (xtr_tmp, Rings_Copy (Xtr_Rings_Get (xtr_p)));

  TRACE_DO (DB_XTRCOPY, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In Xtr_Copy");
    Xtr_Dump (xtr_tmp, &GTraceFile);
    });

  DEBUG (R_XTR, DB_XTRCOPY, TL_PARAMS, (outbuf,
    "Leaving Xtr_Copy, Xtr = %p", xtr_tmp));

  return xtr_tmp;
}

/****************************************************************************
*
*  Function Name:                 Xtr_CopyExpand
*
*    This routine creates a new XTR with more room for atoms, and then 
*    copies the atoms that already exist into it.  This routine does
*    NOT copy any of the associated data-structures of the XTR.
*
*  Used to be:
*
*    xtndxtr:
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
*    Address of copy of XTR
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Xtr_t *Xtr_CopyExpand
  (
  Xtr_t        *xtr_p,                     /* Molecule to make expanded copy */
  U16_t         num_more                   /* Number of additional atoms */
  )
{
  Xtr_t        *xtr_tmp;                   /* Pointer to new XTR */
  XtrRow_t     *xrow_tmp;                  /* Pointer to new XTR rows */

  if (xtr_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_XTRCOPYEXPAND, TL_PARAMS, (outbuf,
    "Entering Xtr_CopyExpand, Xtr = %p, # more = %u", xtr_p, num_more));

  xtr_tmp = Xtr_Create (Xtr_NumAtoms_Get (xtr_p) + num_more);
  xrow_tmp = Xtr_Attributes_Get (xtr_tmp);
  (void) memcpy (xrow_tmp, Xtr_Attributes_Get (xtr_p),
    XTRROWSIZE * Xtr_NumAtoms_Get (xtr_p));

  TRACE_DO (DB_XTRCOPYEXPAND, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In Xtr_CopyExpand, input XTR is");
    Xtr_Dump (xtr_p, &GTraceFile);
    IO_Put_Trace (R_XTR, "In Xtr_CopyExpand, output XTR is");
    Xtr_Dump (xtr_tmp, &GTraceFile);
    });

  DEBUG (R_XTR, DB_XTRCOPYEXPAND, TL_PARAMS, (outbuf,
    "Leaving Xtr_CopyExpand, Xtr = %p", xtr_tmp));

  return xtr_tmp;
}

/****************************************************************************
*
*  Function Name:                 Xtr_CopySubset_Atom
*
*    This routine copies part of an XTR.  It uses a atom mask to eliminate
*    or retain atoms in new XTR.  It also fills in a
*    mapping array so that the caller can determine which atom in the new
*    XTR corresponds to which in the old one.
*
*
*  Used to be:
*
*    subxtra:
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
*    NULL - Address of XTR to copy was NULL
*    <value> - Address of heap storage for copy of XTR
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Xtr_t *Xtr_CopySubset_Atom
  (
  Xtr_t        *xtr_p,                      /* Molecule to copy from */
  Array_t      *atom_p,                     /* 1-d bit, atom mask */
  Array_t      *map_p                       /* 1-d word, atom indices */
  )
{
  U16_t		i;
  U16_t		j;
  U8_t		num_neighbors;
  U16_t	        next_slot;	
  U16_t		neighbor;
  U16_t		new_num_atoms;
  U16_t		num_atoms;
  Xtr_t		*xtr_tmp;
  Array_t	map_tmp;

  if (xtr_p == NULL)
    return NULL;
 
  num_atoms = Xtr_NumAtoms_Get (xtr_p);
   
  for (i=0, new_num_atoms = 0; i < num_atoms; ++i)
     if (Array_1d1_Get (atom_p, i) == TRUE)
        ++new_num_atoms;

  xtr_tmp = Xtr_Create (new_num_atoms);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&map_tmp", "xtr{3}", &map_tmp, num_atoms, WORDSIZE);
#else
  Array_1d_Create (&map_tmp, num_atoms, WORDSIZE);
#endif
  Array_Set (&map_tmp, XTR_INVALID);
 
  for (i=0, next_slot = 0; i < num_atoms; ++i) 
     if (Array_1d1_Get (atom_p, i) == TRUE) {
        SAtomCopy (xtr_p, xtr_tmp, i, next_slot);        
        Array_1d16_Put(map_p, next_slot, i);
        Array_1d16_Put (&map_tmp, i, next_slot);
        ++next_slot;
     }

  for (i=0; i < new_num_atoms; ++i) {
     num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_tmp, i);
     for (j=0; j < num_neighbors; ++j) {
        neighbor = Xtr_Attr_NeighborId_Get (xtr_tmp, i, j);  
        Xtr_Attr_NeighborId_Put (xtr_tmp, i, j, 
	    Array_1d16_Get (&map_tmp, neighbor));
        if (Array_1d16_Get (&map_tmp, neighbor) == XTR_INVALID) {
           Xtr_Attr_NeighborBond_Put (xtr_tmp, i, j, BOND_NONE);
           Xtr_Attr_NeighborStereo_Put (xtr_tmp, i, j, BOND_DIR_INVALID);
           Xtr_Attr_NumNeighbors_Put (xtr_tmp, i, num_neighbors - 1);
        }
     }
  }

  SXtrCompress (xtr_tmp);
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&map_tmp", "xtr", &map_tmp);
#else
  Array_Destroy (&map_tmp);
#endif
  
  return xtr_tmp;
}


/****************************************************************************
*
*  Function Name:                 Xtr_CopySubset
*
*    This routine copies part of an XTR.  It uses a bond mask to eliminate
*    bonds and therefore the atoms that they connect to.  It also fills in a
*    mapping array so that the caller can determine which atom in the new
*    XTR corresponds to which in the old one.
*    
*
*  Used to be:
*
*    subxtrb:
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
*    NULL - Address of XTR to copy was NULL 
*    <value> - Address of heap storage for copy of XTR
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Xtr_t *Xtr_CopySubset
  (
  Xtr_t        *xtr_p,                     /* Molecule to copy from */
  Array_t      *adj_p,                     /* 2-d bit, adjacency mask */
  Array_t      *map_p                      /* 1-d word, atom indices */
  )
{
  Xtr_t        *xtr_tmp;                   /* New XTR */
  U16_t         atom;                      /* Counter */
  U16_t         neigh_index;               /* Counter */
  U16_t         tnum_atoms;                /* # atoms to preserve */ 
  U16_t         num_atoms;                 /* Temporary */
  U16_t         new_atom_index;            /* Index in copy m'cule of atom */
  U8_t          num_neighbors;             /* Temporary */
  Boolean_t     found;                     /* Flag */
  Array_t       map_tmp;                   /* Temporary mapping array */

  if (xtr_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_XTRCOPYSUBSET, TL_PARAMS, (outbuf,
    "Entering Xtr_CopySubset, Xtr = %p, adjacency = %p, mapping = %p", xtr_p,
    adj_p, map_p));

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&map_tmp", "xtr{4}", &map_tmp, num_atoms, WORDSIZE);
#else
  Array_1d_Create (&map_tmp, num_atoms, WORDSIZE);
#endif
  Array_Set (&map_tmp, XTR_INVALID);

  /* Check to see if atom has any neighbors still.
     Keep track of which atoms need to be kept.  Remember old atom index and
     new atom index.
  */

  for (atom = 0, tnum_atoms = 0; atom < num_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh_index = 0, found = FALSE; neigh_index < num_neighbors;
         neigh_index++)
      if (Array_2d1_Get (adj_p, atom, neigh_index))
        found = TRUE;

    if (found == TRUE)
      {
      Array_1d16_Put (map_p, tnum_atoms, atom);
      Array_1d16_Put (&map_tmp, atom, tnum_atoms);
      tnum_atoms++;
      }
    }

  /* Allocate space for copy, then copy atom by atom the ones which are
     needed in the copy, then go through and zero-out the fields that aren't
     in the copy.
  */

  xtr_tmp = Xtr_Create (tnum_atoms);
  for (atom = 0; atom < num_atoms; atom++)
    {
    new_atom_index = Array_1d16_Get (&map_tmp, atom);
    if (new_atom_index != XTR_INVALID)
      {
      SAtomCopy (xtr_p, xtr_tmp, atom, new_atom_index);
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_tmp, new_atom_index);
      for (neigh_index = 0; neigh_index < num_neighbors; neigh_index++)
        {
        if ((Array_2d1_Get (adj_p, atom, neigh_index)) == FALSE)
          {
          Xtr_Attr_NeighborId_Put (xtr_tmp, new_atom_index, neigh_index,
            XTR_INVALID);
          Xtr_Attr_NeighborBond_Put (xtr_tmp, new_atom_index, neigh_index,
            BOND_NONE);
          Xtr_Attr_NeighborStereo_Put (xtr_tmp, new_atom_index,
            neigh_index, BOND_DIR_INVALID);
          Xtr_Attr_NumNeighbors_Put (xtr_tmp, new_atom_index,
            Xtr_Attr_NumNeighbors_Get (xtr_tmp, new_atom_index) - 1);
          }
        }
      }
    }

  /* Compress out all the zeroed-out fields.  Fill in the correct atom index
     values for copy.
  */

  SXtrCompress (xtr_tmp);
  for (atom = 0; atom < tnum_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_tmp, atom);
    for (neigh_index = 0; neigh_index < num_neighbors; neigh_index++)
      {
      Xtr_Attr_NeighborId_Put (xtr_tmp, atom, neigh_index, Array_1d16_Get (
        &map_tmp, Xtr_Attr_NeighborId_Get (xtr_tmp, atom, neigh_index)));
      }
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&map_tmp", "xtr", &map_tmp);
#else
  Array_Destroy (&map_tmp);
#endif

  TRACE_DO (DB_XTRCOPYSUBSET, TL_MAJOR, 
    {
    IO_Put_Trace (R_XTR, "Input XTR to Xtr_CopySubset");
    Xtr_Dump (xtr_p, &GTraceFile);
    IO_Put_Trace (R_XTR, "Output XTR to Xtr_CopySubset");
    Xtr_Dump (xtr_tmp, &GTraceFile);
    });

  DEBUG (R_XTR, DB_XTRCOPYSUBSET, TL_PARAMS, (outbuf,
    "Leaving Xtr_CopySubset, Xtr = %p", xtr_tmp));

  return xtr_tmp;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Create
*
*    This routine creates a new XTR in the heap.  It also allocates the
*    atom attributes array.
*
*  Used to be:
*
*    make_xtr:
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
*    Address of the new XTR
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Xtr_t *Xtr_Create
  (
  U16_t         num_atoms                  /* Number of atoms */
  )
{
  Xtr_t        *xtr_tmp;                   /* Temporary ptr to XTR */
  XtrRow_t     *row_tmp;                   /* For allocation of XTR rows */
  U16_t         atom;                      /* Counter */
  U8_t          neigh;                     /* Counter */

  DEBUG (R_XTR, DB_XTRCREATE, TL_PARAMS, (outbuf,
    "Entering Xtr_Create, # atoms = %u", num_atoms));

#ifdef _MIND_MEM_
  mind_malloc ("xtr_tmp", "xtr{5}", &xtr_tmp, XTRSIZE);

  DEBUG (R_XTR, DB_XTRCREATE, TL_MEMORY, (outbuf,
    "Allocated an Xtr in Xtr_Create at %p", xtr_tmp));

  if (xtr_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL, "No memory for an Xtr in Xtr_Create");

  memset (xtr_tmp, 0, XTRSIZE);

  mind_malloc ("row_tmp", "xtr{5}", &row_tmp, XTRROWSIZE * num_atoms);
#else
  Mem_Alloc (Xtr_t *, xtr_tmp, XTRSIZE, GLOBAL);

  DEBUG (R_XTR, DB_XTRCREATE, TL_MEMORY, (outbuf,
    "Allocated an Xtr in Xtr_Create at %p", xtr_tmp));

  if (xtr_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL, "No memory for an Xtr in Xtr_Create");

  memset (xtr_tmp, 0, XTRSIZE);

  Mem_Alloc (XtrRow_t *, row_tmp, XTRROWSIZE * num_atoms, GLOBAL);
#endif
  Xtr_Attributes_Put (xtr_tmp, row_tmp);

  DEBUG (R_XTR, DB_XTRCREATE, TL_MEMORY, (outbuf,
    "Allocated Xtr Attributes in Xtr_Create at %p",
    Xtr_Attributes_Get (xtr_tmp)));

  if (Xtr_Attributes_Get (xtr_tmp) == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for Xtr Attributes in Xtr_Create");

  Xtr_NumAtoms_Put (xtr_tmp, num_atoms);

  for (atom = 0; atom < num_atoms; atom++)
    {
    Xtr_Attr_Atomid_Put (xtr_tmp, atom, XTR_INVALID);
    Xtr_Attr_NumNeighbors_Put (xtr_tmp, atom, 0);
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      Xtr_Attr_NeighborId_Put (xtr_tmp, atom, neigh, XTR_INVALID);
      Xtr_Attr_NeighborBond_Put (xtr_tmp, atom, neigh, BOND_NONE);
      Xtr_Attr_NeighborStereo_Put (xtr_tmp, atom, neigh, BOND_DIR_INVALID);
      }
    Xtr_Attr_Flags_Put (xtr_tmp, atom, 0);
    }

  DEBUG (R_XTR, DB_XTRCREATE, TL_PARAMS, (outbuf,
    "Leaving Xtr_Create, Xtr = %p", xtr_tmp));

  return xtr_tmp;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Destroy
*
*    This routine destroys an XTR, and all associated data-structures.
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
void Xtr_Destroy
  (
  Xtr_t        *xtr_p                      /* Molecule to destroy */
  )
{
  DEBUG (R_XTR, DB_XTRDESTROY, TL_PARAMS, (outbuf,
    "Entering Xtr_Destroy, Xtr  = %p", xtr_p));

  if (xtr_p == NULL)
     return;

  DEBUG_DO (DB_XTRDESTROY, TL_MEMORY,
    {
    memset (Xtr_Attributes_Get (xtr_p), 0, Xtr_NumAtoms_Get (xtr_p)
      * XTRROWSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("Xtr_Attributes_Get(xtr_p)", "xtr", Xtr_Attributes_Get (xtr_p));
#else
  Mem_Dealloc (Xtr_Attributes_Get (xtr_p), Xtr_NumAtoms_Get (xtr_p)
    * XTRROWSIZE, GLOBAl);
#endif

  DEBUG (R_XTR, DB_XTRDESTROY, TL_MEMORY, (outbuf,
    "Deallocated XTR row memory in Xtr_Destroy at %p",
    Xtr_Attributes_Get (xtr_p)));

  Aromatics_Destroy (Xtr_Aromatics_Get (xtr_p));
  Atoms_Destroy (Xtr_Atoms_Get (xtr_p));
  FuncGroups_Destroy (Xtr_FuncGroups_Get (xtr_p));
  Name_Destroy (Xtr_Name_Get (xtr_p));
  Rings_Destroy (Xtr_Rings_Get (xtr_p));

  DEBUG_DO (DB_XTRDESTROY, TL_MEMORY,
    {
    memset (xtr_p, 0, XTRSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("xtr_p", "xtr", xtr_p);
#else
  Mem_Dealloc (xtr_p, XTRSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_XTRDESTROY, TL_MEMORY, (outbuf,
    "Deallocated Xtr memory in Xtr_Destroy at %p", xtr_p));

  DEBUG (R_XTR, DB_XTRDESTROY, TL_PARAMS, (outbuf,
    "Leaving Xtr_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Dump
*
*    This routine prints a formatted dump of an XTR.
*
*  Used to be:
*
*    printsr:
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
void Xtr_Dump
  (
  Xtr_t        *xtr_p,                     /* Molecule to look at */
  FileDsc_t    *filed_p                    /* File descriptor to print on */
  )
{
  FILE         *f;                         /* Temporary */
  U16_t         i, j;                      /* Counters */
  U16_t         main_id;                   /* Main atom id */
  U16_t         neighbor;                  /* Neighbor atom index */
  U16_t         id;                        /* Neighbor atom id */
  U16_t         len;                       /* Buffer length index */
  U8_t          bondsize;                  /* Size of bond between main and
    neighbor */
  U8_t          degree;                    /* Valence of main atom */
  char          neighbuf[8];               /* For string setup */

  f = IO_FileHandle_Get (filed_p);
  if (xtr_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL XTR\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_XTRCREATE, xtr_p);
  fprintf (f, "Dump of an XTR\n");
  fprintf (f, "node  flags/symbol\t\tneighbors\n");
  fprintf (f, "----  -----------------------------------------------------\n");

  for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
    {
    main_id = Xtr_Attr_Atomid_Get (xtr_p, i);
    degree = Xtr_Attr_NumNeighbors_Get (xtr_p, i);

    /* If atom has only 1 neighbor, then it will appear on that neighbor's
       list of neighbors and bonds and doesn't need a line of its' own.
    */

    if ((degree == 1) && (main_id < ATOM_END)
        && (Xtr_NumAtoms_Get (xtr_p) > 2))
      continue;

    fprintf (f, "%4d  %2x   %s\t", i, Xtr_Attr_Flags_Get (xtr_p, i),
      Atomid2Symbol (main_id));
    for (j = 0; j < degree; j++)
      {
      neighbor = Xtr_Attr_NeighborId_Get (xtr_p, i, j);
      id = Xtr_Attr_Atomid_Get (xtr_p, neighbor);
      memset (neighbuf, 0, sizeof (neighbuf));

      /* If this neighbor has only the current atom as a neighbor, then must
         arrange to print it here.
      */

      if ((id < ATOM_END) && (Xtr_Attr_NumNeighbors_Get (xtr_p, neighbor)
          == 1))
        {
        /* Put atom symbol in place, and if bond is other than single, put
           bond type after atomic symbol, using a ':' as a separator.
        */

        (void) strcat (neighbuf, Atomid2Symbol (id));
        len = strlen (neighbuf);
        bondsize = Xtr_Attr_NeighborBond_Get (xtr_p, i, j);
        if (bondsize != 1)
          {
          neighbuf[len++] = ':';
          if (bondsize == BOND_RESONANT)
            neighbuf[len++] = 'r';
          else
            if (bondsize == BOND_VARIABLE)
              neighbuf[len++] = 'v';
          else
            if (bondsize > MX_BONDMULTIPLICITY)
              neighbuf[len++] = '?';
          else
            neighbuf[len++] = '0' + bondsize;
          }
        }
      else
        {
        Number2Char (neighbor, neighbuf);
        len = strlen (neighbuf);
        neighbuf[len++] = ':';

        bondsize = Xtr_Attr_NeighborBond_Find (xtr_p, i, neighbor);
        if (bondsize == BOND_RESONANT)
          neighbuf[len++] = 'r';
        else
          if (bondsize == BOND_VARIABLE)
            neighbuf[len++] = 'v';
        else
          if (bondsize > MX_BONDMULTIPLICITY)
            neighbuf[len++] = '?';
        else
          neighbuf[len++] = '0' + bondsize;
        }

      for ( ; len < 7; len++)
        neighbuf[len] = ' ';
      fprintf (f, "  %s", neighbuf);
      }          /* End for-j loop */
    fprintf (f, "\n");
    }            /* End for-i loop */

  fprintf (f, "\n");
  Aromatics_Dump (Xtr_Aromatics_Get (xtr_p), filed_p);
  Atoms_Dump (Xtr_Atoms_Get (xtr_p), filed_p);
  FuncGroups_Dump (Xtr_FuncGroups_Get (xtr_p), filed_p);
  Name_Dump (Xtr_Name_Get (xtr_p), filed_p);
  Rings_Dump (Xtr_Rings_Get (xtr_p), filed_p);

  return;
}

/****************************************************************************
*
*  Function Name:                 Xtr_FuncGroups_Destroy
*
*    This routine deallocates the memory associated with a FuncGroups_t in 
*    a Xtr_t.
*
*  Used to be:
*
*    delfgrp:
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
void Xtr_FuncGroups_Destroy
  (
  Xtr_t		*xtr_p
  )
{
  FuncGroups_t	*fungrp_p;

  fungrp_p = Xtr_FuncGroups_Get (xtr_p);

  if (fungrp_p == NULL)
    return;
  
#ifdef _MIND_MEM_
  mind_Array_Destroy ("FuncGrp_Substructures_Get(fungrp_p)", "xtr", FuncGrp_Substructures_Get (fungrp_p));
  mind_Array_Destroy ("FuncGrp_Preservable_Get(fungrp_p)", "xtr", FuncGrp_Preservable_Get (fungrp_p));
 
  mind_free ("fungrp_p", "xtr", fungrp_p);
#else
  Array_Destroy (FuncGrp_Substructures_Get (fungrp_p));
  Array_Destroy (FuncGrp_Preservable_Get (fungrp_p));
 
  Mem_Dealloc (fungrp_p, FUNCGROUPSSIZE, GLOBAL);
#endif
  
  Xtr_FuncGroups_Put (xtr_p, NULL);

}
/****************************************************************************
*
*  Function Name:                 Xtr_Rings_NumRingSys_Get
*
*    This routine returns the number of ring systems in a given xtr.
*
*  Used to be:
*
*    nmrnsys
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
*    number of ring systems
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Xtr_Rings_NumRingSys_Get
  (
  Xtr_t		*xtr_p
  )
{
  if (Xtr_Rings_Get (xtr_p) == NULL)
    Xtr_Rings_Set (xtr_p);

  if (Xtr_Rings_Get (xtr_p) == NULL)
    return 0;
  else
    return Ring_NumRingSystems_Get (Xtr_Rings_Get (xtr_p));
}

/****************************************************************************
*
*  Function Name:                 Xtr_Rings_Set
*
*    This function sets up the ring data-structures so that they can be
*    searched and manipulated.
*
*  Used to be:
*
*    build_rings:
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
void Xtr_Rings_Set
  (
  Xtr_t        *xtr_p                     /* Molecule to check */
  )
{
  Ringsys_t    *ringsyslist_p;            /* Temporary */
  Rings_t      *ring_p;                   /* Temporary */
  U16_t         num_ringsys;              /* Accumulator */

  if (Xtr_Rings_Get (xtr_p) != NULL)
    return;

  if (Xtr_NumAtoms_Get (xtr_p) == 0)
    return;

  DEBUG (R_XTR, DB_XTRRINGSET, TL_PARAMS, (outbuf,
    "Entering Xtr_Rings_Set, Xtr = %p", xtr_p));

  ring_p = Rings_Create (Xtr_NumAtoms_Get (xtr_p), FALSE);
  Ring_Systems_Find (xtr_p, ring_p, &ringsyslist_p, &num_ringsys);

  Ring_NumRingSystems_Put (ring_p, num_ringsys);
  Ring_RingsysList_Put (ring_p, ringsyslist_p);
  Xtr_Rings_Put (xtr_p, ring_p);

  DEBUG (R_XTR, DB_XTRRINGSET, TL_PARAMS, (outbuf,
    "Leaving Xtr_Rings_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Xtr_Ringdef_Set
*
*    This function fills in the RingDefinition portion of the XTR.  It calls
*    a lot of PrimeCycle stuff.  This routine must be called before any of
*    the RingDefinition data-structures are looked at.  See the Rings modules
*    for more details.
*
*  Used to be:
*
*    build_cycles:
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
void Xtr_Ringdef_Set
  (
  Xtr_t           *xtr_p,                 /* Xtr to enhance */
  U16_t            ringsys_idx            /* RingSystem to build it for */
  )
{
  Xtr_t           *xtr_tmp;               /* To subset the Xtr */
  Ringdef_t       *ringdef_p;             /* Temporary for new Ring Def. */
  AtomArray_t     *cycle_p;               /* Temporary */
  Array_t         *vector_p;              /* Cycle vector in Ringdef */
  U16_t            num_atoms;             /* # atoms in m'cule */
  U16_t            cycle_idx;             /* Counter */
  U16_t            node;                  /* Temporary */
  U16_t            nodeid;                /* Temporary */
  Array_t          bonds;                 /* Bond array */
  Array_t          map;                   /* Mapping array */

  DEBUG (R_XTR, DB_XTRRINGDEFSET, TL_PARAMS, (outbuf,
    "Entering Xtr_Ringdef_Set, Xtr = %p, Ring System = %u", xtr_p,
    ringsys_idx));

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "xtr{6}", &bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
  mind_Array_1d_Create ("&map", "xtr{6}", &map, num_atoms, WORDSIZE);
#else
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
  Array_1d_Create (&map, num_atoms, WORDSIZE);
#endif
  Array_Set (&map, XTR_INVALID);

  Ring_Bonds_Set (xtr_p, ringsys_idx, &bonds);
  xtr_tmp = Xtr_CopySubset (xtr_p, &bonds, &map);

  /* Now find the primary cycles in this subset of the XTR.  Don't forget
     to remap the atoms back to their original indices.
  */
  ringdef_p = Ringdef_PrimeCycles_Find (xtr_tmp);
  vector_p = Ringdef_CycleVector_Get (ringdef_p);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("vector_p", "xtr{6}", vector_p, num_atoms, Ringdef_NumCycles_Get (ringdef_p), 
    BITSIZE);
#else
  Array_2d_Create (vector_p, num_atoms, Ringdef_NumCycles_Get (ringdef_p), 
    BITSIZE);
#endif
  Array_Set (vector_p, FALSE);

  for (cycle_idx = 0; cycle_idx < Ringdef_NumCycles_Get (ringdef_p);
       cycle_idx++)
    {
    cycle_p = Ringdef_PrimeCycle_Get (ringdef_p, cycle_idx);
    for (node = 0; node < Ringdef_PrimeCycle_NumNodes_Get (cycle_p); node++)
      {
      nodeid = Array_1d16_Get (&map, Ringdef_PrimeCycle_Node_Get (cycle_p, 
        node));
      Ringdef_PrimeCycle_Node_Put (cycle_p, node, nodeid);
      Ringdef_CycleVector_Node_Put (vector_p, nodeid, cycle_idx, TRUE);
      }
    }

  Ringsys_Ringdef_Insert (xtr_p, ringsys_idx, ringdef_p);

  Xtr_Destroy (xtr_tmp);
#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bonds", "xtr", &bonds);
  mind_Array_Destroy ("&map", "xtr", &map);
#else
  Array_Destroy (&bonds);
  Array_Destroy (&map);
#endif

  DEBUG (R_XTR, DB_XTRRINGDEFSET, TL_PARAMS, (outbuf,
    "Leaving Xtr_Ringdef_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SAtomCopy
*
*    This routine copies an atom from one XTR and atom index to another XTR
*    and atom index.
*
*  Used to be:
*
*    conn_from_to:
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
static void SAtomCopy
  (
  Xtr_t        *from_xtr,                  /* Molecule to copy from */
  Xtr_t        *to_xtr,                    /* Molecule to copy to */
  U16_t         from_atom,                 /* Atom to copy from */
  U16_t         to_atom                    /* Atom to copy to */
  )
{
  to_xtr->attributes[to_atom] = from_xtr->attributes[from_atom];
  return;
}

/****************************************************************************
*
*  Function Name:                 SXtrCompress
*
*    This routine makes sure that all neighbors are in the lowest possible
*    slots in the neighbor array, and that all atoms have the lowest 
*    possible slots in the attributes array.
*
*  Used to be:
*
*    lefjustify_t:
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
static void SXtrCompress
  (
  Xtr_t        *xtr_p                      /* Molecule to compress */
  )
{
  U16_t         last;                      /* Index of last valid id */
  U16_t         i, k, j;                   /* Counters */
  U8_t          num_neighbors;             /* Compiler bug */

  DEBUG (R_XTR, DB_XTRSTATIC, TL_PARAMS, (outbuf,
    "Entering SXtrCompress, Xtr = %p", xtr_p));

  for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
    {
    last = -1;
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, i);
    for (j = 0; j < num_neighbors; j++)
      {
      /* Find the next valid id field */

      for (k = last + 1; (k < MX_NEIGHBORS)
           && (Xtr_Attr_NeighborId_Get (xtr_p, i, k) == XTR_INVALID); k++)
        /* Empty loop body */ ;

      /* Swap the first non-zero field into the lowest still zero field */

      last = k;
      if (k != j)
        {
        Xtr_Attr_NeighborId_Put (xtr_p, i, j,
          Xtr_Attr_NeighborId_Get (xtr_p, i, k));
        Xtr_Attr_NeighborBond_Put (xtr_p, i, j,
          Xtr_Attr_NeighborBond_Get (xtr_p, i, k));
        Xtr_Attr_NeighborStereo_Put (xtr_p, i, j,
          Xtr_Attr_NeighborStereo_Get (xtr_p, i, k));
        Xtr_Attr_NeighborId_Put (xtr_p, i, k, XTR_INVALID);
        Xtr_Attr_NeighborBond_Put (xtr_p, i, k, BOND_NONE);
        Xtr_Attr_NeighborStereo_Put (xtr_p, i, k, BOND_DIR_INVALID);
        }
      }                                    /* End of for-j loop */
    }                                      /* End of for-i loop */

  DEBUG (R_XTR, DB_XTRSTATIC, TL_PARAMS, (outbuf,
    "Leaving SXtrCompress, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                SVisit
*
*    This function does a BFS of an XTR to figure out the size of the
*    connected component (not yet visited).
*
*  Used to be:
*
*    visit (from nreach):
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
static void SVisit
  (
  Xtr_t        *xtr_p,                     /* Handle of XTR to examine */
  Array_t      *visited_p,                 /* 1d-bit, flags */
  U16_t         atom,                      /* Atom to start at */
  U16_t        *count_p                    /* Current count */
  )
{
  U16_t         i;                         /* Counter */
  U16_t         next;                      /* Neighbor id */
  U8_t          num_neigh;                 /* Compiler bug */

  Array_1d1_Put (visited_p, atom, TRUE);
  (*count_p)++;
  num_neigh = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
  for (i = 0; i < num_neigh; i++)
    {
    next = Xtr_Attr_NeighborId_Get (xtr_p, atom, i);
    if (Array_1d1_Get (visited_p, next) != TRUE)
      SVisit (xtr_p, visited_p, next, count_p);
    }

  return;
}
/* End of SVisit */
/* End of XTR.C */
