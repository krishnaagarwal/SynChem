/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     TSD.C
*
*    This module is the data-abstraction of the TSD.  The TSD is the
*    preferred representation of molecules in SYNCHEM.  It is the most 
*    compact format.  Each atom has an entry in an array which has as
*    substituents - the atomic number of the atom, and atom indices for
*    the atoms at the other end of the bonds to this atom.
*
*  Routines:
*
*    Tsd_Atom_Bond_Change
*    Tsd_Atom_Connect
*    Tsd_Atom_ConnectChange
*    Tsd_Atom_Disconnect
*    Tsd_Atom_Empty_Find
*    Tsd_Atom_Index_Find
*    Tsd_Atom_Invert
*    Tsd_Atom_MaxValence_Get
*    Tsd_Atom_Neighbors_Get
*    Tsd_Atom_Valence_Get
*    Tsd_Copy
*    Tsd_Create
*    Tsd_Destroy
*    Tsd_Dump
*    Tsd_Expand
*    Tsd_LastAtomId_Find
*    Tsd_MatchCompress
*    Tsd_Strip_H
*    Tsd_Verify
*    SNeighborSwitch
*    SRowCompress
*    SRowSwap
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
* 21-Feb-95  Cheung     Change b++ to b-- in routine SRowCompress
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym_chalcogen.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

/* Static Routine Prototypes */

static void     SNeighborSwitch (Tsd_t *, U16_t, U8_t, U8_t);
static void     SRowCompress    (Tsd_t *, U16_t);
static void     SRowSwap        (Tsd_t *, Tsd_t *, U16_t, U16_t);


/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Bond_Change
*
*    This routine sets the bond type in both atoms so that they are
*    consistent.
*
*  Used to be:
*
*    change_bond:
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
void Tsd_Atom_Bond_Change
  (
  Tsd_t         *tsd_p,                     /* Molecule we are looking at */
  U16_t          atom1,                     /* Atom1 is related to atom2 */
  U16_t          atom2,                     /* Atom2 is related to atom1 */
  U8_t           bondsize                   /* New: (single, double, etc.) */
  )
{
  U8_t           index;                     /* Temp for atom index value */

  DEBUG (R_TSD, DB_TSDBONDCHANGE, TL_PARAMS, (outbuf,
    "Entering Tsd_Atom_Bond_Change, Tsd = %p, atom1 = %u, atom2 = %u,"
    " bondsize = %u", tsd_p, atom1, atom2, bondsize));

  index = Tsd_Atom_Index_Find (tsd_p, atom1, atom2);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom1, index, bondsize);
  index = Tsd_Atom_Index_Find (tsd_p, atom2, atom1);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom2, index, bondsize);

  DEBUG (R_TSD, DB_TSDBONDCHANGE, TL_PARAMS, (outbuf,
    "Leaving Tsd_Atom_Bond_Change, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Connect
*
*    This routine marks two atoms as respectively neighbors and also sets
*    the bond attribute correctly.  It first looks for empty slots in the
*    neighbor vector and assigns the neighbors to the first ones.
*    Note: The ASSERT test relies on the fact that MX_NEIGHBORS is 6
*
*  Used to be:
*
*    connect_nodes:
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
void Tsd_Atom_Connect
  (
  Tsd_t         *tsd_p,                     /* Molecule we are looking at */
  U16_t          atom1,                     /* Atom1 is related to atom2 */
  U16_t          atom2,                     /* Atom2 is related to atom1 */
  U8_t           bondsize                   /* Bond: (single, double, etc.) */
  )
{
  U16_t          next_1;                    /* 1st free neighbor slot
    in atom1 */
  U16_t          next_2;                    /* 1st free neighbor slot
    in atom2 */

  DEBUG (R_TSD, DB_TSDATOMCONN, TL_PARAMS, (outbuf,
    "Entering Tsd_Atom_Connect, Tsd = %p, atom1 = %u, atom2 = %u,"
    " bondsize = %u", tsd_p, atom1, atom2, bondsize));

  next_1 = Tsd_Atom_Index_Find (tsd_p, atom1, TSD_INVALID);
  next_2 = Tsd_Atom_Index_Find (tsd_p, atom2, TSD_INVALID);

  ASSERT (next_1 != TSD_INVALID && next_2 != TSD_INVALID,
    {
    DEBUG (R_TSD, DB_TSDATOMCONN, TL_ALWAYS, (outbuf,
      "Node already has 6 neighbors in Tsd_Atom_Connect, Tsd = %p,"
      " atom1 = %u, atom2 = %u", tsd_p, atom1, atom2));

    IO_Exit_Error (R_TSD, X_SYNERR, "Assert failure");
    });

  Tsd_Atom_NeighborId_Put (tsd_p, atom1, next_1, atom2);
  Tsd_Atom_NeighborId_Put (tsd_p, atom2, next_2, atom1);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom1, next_1, bondsize);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom2, next_2, bondsize);

  DEBUG (R_TSD, DB_TSDATOMCONN, TL_PARAMS, (outbuf,
    "Leaving Tsd_Atom_Connect, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_ConnectChange
*
*    This routine connects two atoms in at TSD.  This routine is called
*    rather haphazardly, so it must check to see if the nodes are already
*    connected or if the new bondsize is less than zero which indicates that
*    the atoms should actually be disconnected.  The routine also checks for
*    error conditions, such as trying to connect an atom to itself, and also
*    for an illegal bond type.
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
void Tsd_Atom_ConnectChange
  (
  Tsd_t         *tsd_p,                     /* Molecule to look in */
  U16_t          atom1,                     /* Atom1 is related to atom2 */
  U16_t          atom2,                     /* Atom2 is related to atom1 */
  U8_t           bondsize                   /* Mltplcty of atom1-atom2 bond*/
  )
{
  Boolean_t      conn_flag;                 /* Check if atoms connected yet */

  DEBUG (R_TSD, DB_TSDATOMCONCHNG, TL_PARAMS, (outbuf,
    "Entering Tsd_Atom_ConnectChange, Tsd = %p, atom1 = %u, atom2 = %u,"
    " bondsize = %u", tsd_p, atom1, atom2, bondsize));

  if (atom1 == atom2)
    return;

  ASSERT (bondsize < MX_BONDMULTIPLICITY || bondsize == BOND_VARIABLE
          || bondsize == BOND_RESONANT,
    {
    DEBUG (R_TSD, DB_TSDATOMCONCHNG, TL_ALWAYS, (outbuf,
      "Bond multiplictity value is illegal in Tsd_Atom_Connect, value is %u",
      bondsize));

    IO_Exit_Error (R_TSD, X_SYNERR, "Assert failure");
    });

  conn_flag = Tsd_Atom_AreConnected (tsd_p, atom1, atom2);
  if (conn_flag == TRUE && bondsize == BOND_NONE)
    Tsd_Atom_Disconnect (tsd_p, atom1, atom2);
  else
    if (conn_flag == TRUE)
      Tsd_Atom_Bond_Change (tsd_p, atom1, atom2, bondsize);
  else
    Tsd_Atom_Connect (tsd_p, atom1, atom2, bondsize);

  DEBUG (R_TSD, DB_TSDATOMCONCHNG, TL_PARAMS, (outbuf,
    "Leaving Tsd_Atom_ConnectChange, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Disconnect
*
*    This routine disconnects two atoms, it makes them no longer neighbors
*    and makes sure the bonds attribute is cleared as well.
*
*  Used to be:
*
*    disconnect:
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
void Tsd_Atom_Disconnect
  (
  Tsd_t         *tsd_p,                     /* Molecule we are looking at */
  U16_t          atom1,                     /* Atom1 is related to atom2 */
  U16_t          atom2                      /* Atom2 is related to atom1 */
  )
{
  U8_t           index1;                    /* Neigh. index of atom1 v atom2 */
  U8_t           index2;                    /* Neigh. index of atom2 v atom2 */

  DEBUG (R_TSD, DB_TSDATOMDISC, TL_PARAMS, (outbuf,
    "Entering Tsd_Atom_Disconnect, Tsd = %p, atom1 = %u, atom2 = %u",
    tsd_p, atom1, atom2));

  index1 = Tsd_Atom_Index_Find (tsd_p, atom1, atom2);
  index2 = Tsd_Atom_Index_Find (tsd_p, atom2, atom1);

  Tsd_Atom_NeighborId_Put (tsd_p, atom1, index1, TSD_INVALID);
  Tsd_Atom_NeighborId_Put (tsd_p, atom2, index2, TSD_INVALID);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom1, index1, BOND_NONE);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom2, index2, BOND_NONE);

  DEBUG (R_TSD, DB_TSDATOMDISC, TL_PARAMS, (outbuf,
    "Leaving Tsd_Atom_Disconnect, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Empty_Find
*
*    This routine finds simply the atom slot with the lowest index value
*    that is unused.
*
*  Used to be:
*
*    N/A
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
*    TSD_INVALID - couldn't find an empty atom in the molecule
*    <val> - the subscript value for the empty atom slot
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Tsd_Atom_Empty_Find
  (
  Tsd_t         *tsd_p                      /* Molecule to look in */
  )
{
  U16_t          i;                         /* Counter */

  for (i = 0; i < Tsd_NumAtoms_Get (tsd_p); i++)
    if (Tsd_Atom_Valence_Get (tsd_p, i) == 0)
      return i;

  return TSD_INVALID;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Index_Find
*
*    This routine returns the neighbor number of one atom with respect to
*    another.  It is possible that this should be a static routine.
*
*  Used to be:
*
*    find_index:
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
*    TSD_INVALID - neighbor is not a "neighbor" of atom
*    <val> - subscript of neighbor in atom's descriptor
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Tsd_Atom_Index_Find
  (
  Tsd_t         *tsd_p,                     /* TSD we are looking at */
  U16_t          atom,                      /* Atom we are looking at */
  U16_t          neighbor                   /* Neighbor of atom whose index we
    want */
  )
{
  U16_t          i;                         /* Counter */

  for (i = 0; i < MX_NEIGHBORS; i++)
    if (Tsd_Atom_NeighborId_Get (tsd_p, atom, i) == neighbor)
      return i;

  return TSD_INVALID;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Invert
*
*    This routine flips an atom around by its first and second neighbors.
*    This is relevant to stereochemistry.
*
*  Used to be:
*
*    invert:
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
void Tsd_Atom_Invert
  (
  Tsd_t         *tsd_p,                     /* Molecule to play with */
  U16_t          atom                       /* Atom to invert */
  )
{
  SNeighborSwitch (tsd_p, atom, 0, 1);
  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Neighbors_Get
*
*    This routine fills in a 1-D array with the ids of the neighbors of an
*    atom in a molecule (TSD).
*
*  Used to be:
*
*    nb:
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
void Tsd_Atom_Neighbors_Get
  (
  Tsd_t         *tsd_p,                     /* Address of TSD */
  U16_t          atom,                      /* Which atom */
  Array_t       *array_p                    /* Uninitialized 1-d word array */
  )
{
  U8_t           i;                         /* Counter */

  DEBUG (R_TSD, DB_TSDATOMNEIGHGET, TL_PARAMS, (outbuf,
    "Entering Tsd_Neighbors_Get, Tsd = %p, atom # = %u, output = %p",
    tsd_p, atom, array_p));

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("array_p", "tsd{1}", array_p, MX_NEIGHBORS, WORDSIZE);
#else
  Array_1d_Create (array_p, MX_NEIGHBORS, WORDSIZE);
#endif
  for (i = 0; i < MX_NEIGHBORS; i++)
    Array_1d16_Put (array_p, i, Tsd_Atom_NeighborId_Get (tsd_p, atom, i));

  DEBUG (R_TSD, DB_TSDATOMNEIGHGET, TL_PARAMS, (outbuf,
    "Leaving Tsd_Neighbors_Get, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Atom_Valence_Get
*
*    This routine calculates the valence of an atom by looking at the bond
*    between it and each of its neighbors.  Note different representation of
*    aromatic bonds.
*
*  Used to be:
*
*    valence:, getvalence:
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
*    Valence of specified atom
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U8_t Tsd_Atom_Valence_Get
  (
  Tsd_t         *tsd_p,                     /* Molecule we are looking at */
  U16_t          atom                       /* Atom whose valence we want */
  )
{
  U8_t           i;                         /* Counter */
  U8_t           aromatic_cnt;              /* Number of aromatic bonds */
  U8_t           valence;                   /* Number of bonds */

  DEBUG (R_TSD, DB_TSDATOMVALGET, TL_PARAMS, (outbuf,
    "Entering Tsd_Atom_Valence_Get, Tsd = %p, atom # = %u", tsd_p, atom));

  for (i = 0, aromatic_cnt = 0, valence = 0; i < MX_NEIGHBORS; i++)
    {
    if (Tsd_Atom_NeighborBond_Get (tsd_p, atom, i) == BOND_VARIABLE ||
        Tsd_Atom_NeighborBond_Get (tsd_p, atom, i) == BOND_RESONANT)
      aromatic_cnt++;
    else
      if (Tsd_Atom_NeighborBond_Get (tsd_p, atom, i) != BOND_NONE)
        valence++;
    }

  switch (aromatic_cnt)
    {
    case 0:
    case 1:

      valence += aromatic_cnt;
      break;

    case 2:

      valence += 3;
      break;

    case 3:

      valence += 4;
      break;

    default:

      ASSERT (FALSE,
        {
        DEBUG (R_TSD, DB_TSDATOMVALGET, TL_ALWAYS, (outbuf, 
          "Aromatic valence error in Tsd_Atom_Valence_Get, Tsd = %p,"
          " atom = %u, found aromatic cnt = %u", tsd_p, atom, aromatic_cnt));

        valence += 5;

        IO_Exit_Error (R_TSD, X_SYNERR, "Assert failure");
        });
        break;
    }

  DEBUG (R_TSD, DB_TSDATOMVALGET, TL_PARAMS, (outbuf,
    "Leaving Tsd_Atom_Valence_Get, valence = %u", valence));

  return valence;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Copy
*
*    This function creates a copy of a TSD, allocated memory and all.
*
*  Used to be:
*
*    ntsd:
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
*    Address of newly allocated storeage
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Tsd_t *Tsd_Copy
  (
  Tsd_t         *tsd_p                      /* TSD to copy */
  )
{
  Tsd_t         *tsd_tmp;                   /* Temporary pointer */

  if (tsd_p == NULL)
    return NULL;

  DEBUG (R_TSD, DB_TSDCOPY, TL_PARAMS, (outbuf,
    "Entering Tsd_Copy, Tsd = %p", tsd_p));

#ifdef _MIND_MEM_
  mind_malloc ("tsd_tmp", "tsd{2}", &tsd_tmp, TSDSIZE);

  DEBUG (R_TSD, DB_TSDCOPY, TL_MEMORY, (outbuf,
    "Allocated a Tsd in Tsd_Copy at %p", tsd_tmp));

  if (tsd_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for a Tsd in Tsd_Copy");

  mind_malloc ("TsdAtomHandle_Get(tsd_tmp)", "tsd{2}", &Tsd_AtomHandle_Get (tsd_tmp), TSDROWSIZE * Tsd_NumAtoms_Get (tsd_p));
#else
  Mem_Alloc (Tsd_t *, tsd_tmp, TSDSIZE, GLOBAL);

  DEBUG (R_TSD, DB_TSDCOPY, TL_MEMORY, (outbuf,
    "Allocated a Tsd in Tsd_Copy at %p", tsd_tmp));

  if (tsd_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for a Tsd in Tsd_Copy");

  Mem_Alloc (TsdRow_t *, Tsd_AtomHandle_Get (tsd_tmp),
    TSDROWSIZE * Tsd_NumAtoms_Get (tsd_p), GLOBAL);
#endif

  DEBUG (R_TSD, DB_TSDCOPY, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_Copy at %p", Tsd_AtomHandle_Get (tsd_tmp)));

  if (Tsd_AtomHandle_Get (tsd_tmp) == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for Tsd Rows in Tsd_Copy");

  Tsd_NumAtoms_Put (tsd_tmp, Tsd_NumAtoms_Get (tsd_p));
  (void) memcpy (Tsd_AtomHandle_Get (tsd_tmp), Tsd_AtomHandle_Get (tsd_p),
    TSDROWSIZE * Tsd_NumAtoms_Get (tsd_p));

  DEBUG (R_TSD, DB_TSDCOPY, TL_PARAMS, (outbuf,
    "Leaving Tsd_Copy, Tsd = %p", tsd_tmp));

  return tsd_tmp;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Create
*
*    This function allocates memory for a TSD block, plus the associated
*    atoms array.
*
*  Used to be:
*
*    N/A
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
*    Address of newly allocated storage
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Tsd_t *Tsd_Create
  (
  U16_t          num_atoms                  /* Number of atoms in molecule */
  )
{
  Tsd_t         *tsd_tmp;                   /* Temporary TSD ptr */
  U16_t          atom;                      /* Counter */
  U8_t           neigh;                     /* Counter */

  DEBUG (R_TSD, DB_TSDCREATE, TL_PARAMS, (outbuf,
    "Entering Tsd_Create, # atoms = %u", num_atoms));

#ifdef _MIND_MEM_
  mind_malloc ("tsd_tmp", "tsd{3}", &tsd_tmp, TSDSIZE);

  DEBUG (R_TSD, DB_TSDCREATE, TL_MEMORY, (outbuf,
    "Allocated a Tsd in Tsd_Create at %p", tsd_tmp));

  if (tsd_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for a Tsd in Tsd_Create");

  mind_malloc ("Tsd_AtomHandle_Get(tsd_tmp)", "tsd{3}", &Tsd_AtomHandle_Get (tsd_tmp), TSDROWSIZE * num_atoms);
#else
  Mem_Alloc (Tsd_t *, tsd_tmp, TSDSIZE, GLOBAL);

  DEBUG (R_TSD, DB_TSDCREATE, TL_MEMORY, (outbuf,
    "Allocated a Tsd in Tsd_Create at %p", tsd_tmp));

  if (tsd_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for a Tsd in Tsd_Create");

  Mem_Alloc (TsdRow_t *, Tsd_AtomHandle_Get (tsd_tmp), TSDROWSIZE * num_atoms,
    GLOBAL);
#endif

  DEBUG (R_TSD, DB_TSDCREATE, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_Create at %p", Tsd_AtomHandle_Get (tsd_tmp)));

  if (Tsd_AtomHandle_Get (tsd_tmp) == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for Tsd Rows in Tsd_Create");

  Tsd_NumAtoms_Put (tsd_tmp, num_atoms);

  for (atom = 0; atom < num_atoms; atom++)
    {
    Tsd_Atomid_Put (tsd_tmp, atom, TSD_INVALID);
    Tsd_Atom_Flags_Put (tsd_tmp, atom, 0);
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++)
      {
      Tsd_Atom_NeighborId_Put (tsd_tmp, atom, neigh, TSD_INVALID);
      Tsd_Atom_NeighborBond_Put (tsd_tmp, atom, neigh, BOND_NONE);
      }
    }

  DEBUG (R_TSD, DB_TSDCREATE, TL_PARAMS, (outbuf,
    "Leaving Tsd_Create, Tsd = %p", tsd_tmp));

  return tsd_tmp;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Destroy
*
*    This routine deallocates the memory used by a TSD.
*
*  Used to be:
*
*    N/A
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
void Tsd_Destroy
  (
  Tsd_t         *tsd_p                      /* Address of TSD */
  )
{
  if (tsd_p == NULL)
    return;

  DEBUG (R_TSD, DB_TSDDESTROY, TL_PARAMS, (outbuf,
    "Entering Tsd_Destroy, Tsd = %p", tsd_p));

  DEBUG_DO (DB_TSDDESTROY, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (tsd_p), 0, Tsd_NumAtoms_Get (tsd_p) *
      TSDROWSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("Tsd_AtomHandle_Get(tsd_p)", "tsd", Tsd_AtomHandle_Get (tsd_p));

  DEBUG (R_TSD, DB_TSDDESTROY, TL_MEMORY, (outbuf,
    "Deallocated Tsd Row in Tsd_Destroy at %p", Tsd_AtomHandle_Get (tsd_p)));

  DEBUG_DO (DB_TSDDESTROY, TL_MEMORY,
    {
    memset (tsd_p, 0, TSDSIZE);
    });

  mind_free ("tsd_p", "tsd", tsd_p);
#else
  Mem_Dealloc (Tsd_AtomHandle_Get (tsd_p), Tsd_NumAtoms_Get (tsd_p) *
    TSDROWSIZE, GLOBAL);

  DEBUG (R_TSD, DB_TSDDESTROY, TL_MEMORY, (outbuf,
    "Deallocated Tsd Row in Tsd_Destroy at %p", Tsd_AtomHandle_Get (tsd_p)));

  DEBUG_DO (DB_TSDDESTROY, TL_MEMORY,
    {
    memset (tsd_p, 0, TSDSIZE);
    });

  Mem_Dealloc (tsd_p, TSDSIZE, GLOBAL);
#endif

  DEBUG (R_TSD, DB_TSDDESTROY, TL_MEMORY, (outbuf,
    "Deallocated a Tsd in Tsd_Destroy at %p", tsd_p));

  DEBUG (R_TSD, DB_TSDDESTROY, TL_PARAMS, (outbuf,
    "Leaving Tsd_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Dump
*
*    This routine prints a formatted dump of a TSD.
*
*  Used to be:
*
*    prtsd:, qprtsd:
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
void Tsd_Dump
  (
  Tsd_t         *tsd_p,                      /* Address of TSD to format */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */
  U16_t          i, j;                       /* Counters */
  U8_t           bondsize, type;             /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (tsd_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL TSD\n");
    return;
    }

  DEBUG_ADDR (R_TSD, DB_TSDCREATE, tsd_p);
  fprintf (f, "Dump of TSD\n");
  fprintf (f, "Atom  Flags/Id\tUp\tDown\tLeft\tRight\tIn\tOut\n");
  for (i = 0; i < Tsd_NumAtoms_Get (tsd_p); i++)
    {
    fprintf (f, "#%4u  %2x  %s", i, Tsd_Atom_Flags_Get (tsd_p, i),
      Atomid2Symbol (Tsd_Atomid_Get (tsd_p, i)));
    for (j = 0; j < MX_NEIGHBORS; j++)
      {
      bondsize = Tsd_Atom_NeighborBond_Get (tsd_p, i, j);
      switch (bondsize)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:

          type = '0' + bondsize;
          break;

        case BOND_VARIABLE:

          type = 'v';
          break;

        case BOND_RESONANT:

          type = 'r';
          break;

        default:

          type = '?';
          break;
        }

/* This makes no sense whatsoever - the whole purpose is to debug, not hide relevant data!! */
      if (bondsize != BOND_NONE || Tsd_Atom_NeighborId_Get (tsd_p, i, j) != TSD_INVALID)
        fprintf (f, "\t%u:%c", Tsd_Atom_NeighborId_Get (tsd_p, i, j), type);
      else
        fprintf (f, "\t");
      }

    fprintf (f, "\n");
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Expand
*
*    This routine expands a TSD.  It simply reallocates the atoms array
*    and copies the old one in, then it frees the old array.
*
*  Used to be:
*
*    exptsd:
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
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
void Tsd_Expand
  (
  Tsd_t         *tsd_p,                     /* Address of TSD */
  U16_t          new_number                 /* Number of atoms in molecule */
  )
{
  TsdRow_t      *trow_tmp;                  /* Temporary TSD row ptr */
  U16_t          old_number;                /* Temporary */
  U16_t          atom;                      /* Counter */
  U8_t           neigh;                     /* Counter */

  DEBUG (R_TSD, DB_TSDEXPAND, TL_PARAMS, (outbuf,
    "Entering Tsd_Expand, Tsd = %p, expand = %u", tsd_p, new_number));

  if (new_number <= Tsd_NumAtoms_Get (tsd_p))
    return;

#ifdef _MIND_MEM_
  mind_malloc ("trow_tmp", "tsd{4}", &trow_tmp, TSDROWSIZE * new_number);

  DEBUG (R_TSD, DB_TSDEXPAND, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_Expand at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for Tsd Rows in Tsd_Expand");

  (void) memset (trow_tmp, 0, TSDROWSIZE * new_number);
  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (tsd_p),
    Tsd_NumAtoms_Get (tsd_p) * TSDROWSIZE);

  DEBUG_DO (DB_TSDEXPAND, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (tsd_p), 0, Tsd_NumAtoms_Get (tsd_p) *
      TSDROWSIZE);
    });

  mind_free ("Tsd_AtomHandle_Get(tsd_p)", "tsd", Tsd_AtomHandle_Get (tsd_p));
#else
  Mem_Alloc (TsdRow_t *, trow_tmp, TSDROWSIZE * new_number, GLOBAL);

  DEBUG (R_TSD, DB_TSDEXPAND, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_Expand at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL, "No memory for Tsd Rows in Tsd_Expand");

  (void) memset (trow_tmp, 0, TSDROWSIZE * new_number);
  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (tsd_p),
    Tsd_NumAtoms_Get (tsd_p) * TSDROWSIZE);

  DEBUG_DO (DB_TSDEXPAND, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (tsd_p), 0, Tsd_NumAtoms_Get (tsd_p) *
      TSDROWSIZE);
    });

  Mem_Dealloc (Tsd_AtomHandle_Get (tsd_p), Tsd_NumAtoms_Get (tsd_p) *
    TSDROWSIZE, GLOBAL);
#endif

  DEBUG (R_TSD, DB_TSDEXPAND, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_Expand at %p", Tsd_AtomHandle_Get (tsd_p)));

  old_number = Tsd_NumAtoms_Get (tsd_p);
  Tsd_AtomHandle_Put (tsd_p, trow_tmp);
  Tsd_NumAtoms_Put (tsd_p, new_number);

  for (atom = old_number; atom < new_number; atom++)
    for (neigh = 0; neigh < MX_NEIGHBORS; neigh++) 
      {
      Tsd_Atom_NeighborId_Put (tsd_p, atom, neigh, TSD_INVALID);
      Tsd_Atom_NeighborBond_Put (tsd_p, atom, neigh, BOND_NONE);
      }

  DEBUG (R_TSD, DB_TSDEXPAND, TL_PARAMS, (outbuf,
    "Leaving Tsd_Expand, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_LastAtomid_Find
*
*    This routine returns the number of the lowest number slot in the TSD
*    that is used.  For some reason it distinguishes between odd and
*    even slot numbers.
*
*  Used to be:
*
*    lastvar:
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
*    The lowest atomic id ???
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t Tsd_LastAtomid_Find
  (
  Tsd_t         *tsd_p,                     /* Molecule to look in */
  U16_t          id                         /* Is id odd? or even?, it counts */
  )
{
  U16_t          last;                      /* Current minimum ??? */
  U16_t          i;                         /* Counter */
  U16_t          temp;                      /* For checking minimum */

  /* The algorithm seems a bit odd, id must start positive ??? but identifiers
     are all negative ??? */

  for (i = 0, last = id % 2; i < Tsd_NumAtoms_Get (tsd_p); i++)
    {
    temp = Tsd_Atomid_Get (tsd_p, i);
    if ((S16_t)temp < 0 && ((temp % 2) == (id % 2)))
      last = MIN (last, temp);
    }

  return last;
}

/****************************************************************************
*
*  Function Name:                 Tsd_MatchCompress
*
*    This routine compresses two TSDs of the same size.  The caller implies
*    they are related as goal and sub-goal.  There are several things to do,
*    first is to Compress each row, then eliminate any unused rows (atoms)
*    in the TSD, then actually reallocate the memory used by the rows down
*    to the minimum size needed.
*
*  Used to be:
*
*    pack:
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
*    Allocates memory
*    Deallocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
void Tsd_MatchCompress
  (
  Tsd_t         *gtsd_p,                    /* Goal TSD */
  Tsd_t         *sgtsd_p,                   /* Subgoal TSD (must match) */
  U32_t         *roots                      /* incomplete without this!!! */
  )
{
  U16_t          i, j, root_inx;            /* Counters */
  TsdRow_t      *trow_tmp;                  /* Temp for shuffling the atoms */
  Boolean_t      sorted;

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_PARAMS, (outbuf,
    "Entering Tsd_MatchCompress, goal = %p, subgoal = %p", gtsd_p, sgtsd_p));

  ASSERT (gtsd_p != NULL && sgtsd_p != NULL
      && Tsd_NumAtoms_Get (gtsd_p) == Tsd_NumAtoms_Get (sgtsd_p),
    {
    DEBUG (R_TSD, DB_TSDMATCOMP, TL_ALWAYS, (outbuf,
      "TSDs not suited to compression in Tsd_MatchCompress"));

    IO_Exit_Error (R_TSD, X_SYNERR, "Assert failure");
    });

  /* Compress each atom's representation in both the goal and sub-goal TSDs */

  for (i = 0; i < Tsd_NumAtoms_Get (gtsd_p); i++)
    {
    SRowCompress (gtsd_p, i);
    SRowCompress (sgtsd_p, i);
    }

  /* Sort all the empty rows (eliminated atoms?) to the end of the TSD.
     If the zeroth (first) neighbor is TSD_INVALID, then the atom
     (as represented by the row) is non-existent.
  */

  for (i = 0, j = Tsd_NumAtoms_Get (gtsd_p); i < j; )
    {
/*
    while (Tsd_Atom_NeighborId_Get (gtsd_p, i, 0) != TSD_INVALID && i < j)
      i++;
    while (Tsd_Atom_NeighborId_Get (gtsd_p, j, 0) == TSD_INVALID)
      j--;
*/
    while (Tsd_Atom_NeighborId_Get (gtsd_p, i, 0) != TSD_INVALID &&
           Tsd_Atom_NeighborId_Get (sgtsd_p, i, 0) != TSD_INVALID &&
           i < Tsd_NumAtoms_Get (gtsd_p))
      i++;
    do --j; while (Tsd_Atom_NeighborId_Get (gtsd_p, j, 0) == TSD_INVALID &&
                   Tsd_Atom_NeighborId_Get (sgtsd_p, j, 0) == TSD_INVALID &&
                   j > i);

    if (i < j)
      {
      for (root_inx = 0; root_inx < MX_ROOTS && roots[root_inx] != REACT_NODE_INVALID; root_inx++)
        {
        if (roots[root_inx] == i) roots[root_inx] = j;
        else if (roots[root_inx] == j) roots[root_inx] = i;
        }
      SRowSwap (gtsd_p, sgtsd_p, i, j);
      }
    }

/* Now fix up the atom order so that all the disconnected atoms in the GP are at the END (to prevent "Major foobar" in SubGenr) -
   Note that this is a kludge on top of another kludge - the gratuitous swapping of rows is confusing, and the criterion for
   determining the number of neighbors is too simplistic to be robust enough for stereochemically correct patterns.  This
   portion of tsd.c needs a major overhaul, as well as a convenient function or macro, e.g., Tsd_Atom_NumNbrs_Get */
  for (i = 0, sorted = FALSE; i < Tsd_NumAtoms_Get (gtsd_p) - 1 && !sorted; i++)
    {
    sorted = TRUE;
    if (Tsd_Atom_NeighborId_Get (gtsd_p, i, 0) == TSD_INVALID && Tsd_Atom_NeighborId_Get (gtsd_p, i + 1, 0) != TSD_INVALID)
      {
      sorted = FALSE;
      for (root_inx = 0; root_inx < MX_ROOTS && roots[root_inx] != REACT_NODE_INVALID; root_inx++)
        {
        if (roots[root_inx] == i) roots[root_inx] = i + 1;
        else if (roots[root_inx] == i + 1) roots[root_inx] = i;
        }
      SRowSwap (gtsd_p, sgtsd_p, i, i + 1);
      }
    }

  /* Figure out the number of atoms that are really still in the TSDs */
/* This is WRONG!
  for (i = Tsd_NumAtoms_Get (gtsd_p) - 1;
    i > 0 && Tsd_Atom_NeighborId_Get (gtsd_p, i, 0) == TSD_INVALID
    && Tsd_Atom_NeighborId_Get (sgtsd_p, i, 0) == TSD_INVALID; i--);
*/
  for (i = Tsd_NumAtoms_Get (gtsd_p);
    i > 0 && Tsd_Atom_NeighborId_Get (gtsd_p, i - 1, 0) == TSD_INVALID
    && Tsd_Atom_NeighborId_Get (sgtsd_p, i - 1, 0) == TSD_INVALID; i--);

  /* Allocate new sets of TsdRow_t's and and copy the old ones in and
     then destroy them.
  */
#ifdef _MIND_MEM_
  mind_malloc ("trow_tmp", "tsd{5}", &trow_tmp, TSDROWSIZE * i);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Rows in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (gtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (gtsd_p), 0, TSDROWSIZE * i);
    });

  mind_free ("Tsd_AtomHandle_Get(gtsd_p)", "tsd", Tsd_AtomHandle_Get (gtsd_p));

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_MatchCompress at %p",
    Tsd_AtomHandle_Get (gtsd_p)));

  Tsd_AtomHandle_Put (gtsd_p, trow_tmp);
  Tsd_NumAtoms_Put (gtsd_p, i);

  mind_malloc ("trow_tmp", "tsd{5a}", &trow_tmp, TSDROWSIZE * i);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Row allocation in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (sgtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (sgtsd_p), 0, TSDROWSIZE * i);
    });

  mind_free ("Tsd_AtomHandle_Get(sgtsd_p)", "tsd", Tsd_AtomHandle_Get (sgtsd_p));
#else
  Mem_Alloc (TsdRow_t *, trow_tmp, TSDROWSIZE * i, GLOBAL);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Rows in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (gtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (gtsd_p), 0, TSDROWSIZE * i);
    });

  Mem_Dealloc (Tsd_AtomHandle_Get (gtsd_p), TSDROWSIZE * Tsd_NumAtoms_Get (gtsd_p), GLOBAL);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_MatchCompress at %p",
    Tsd_AtomHandle_Get (gtsd_p)));

  Tsd_AtomHandle_Put (gtsd_p, trow_tmp);
  Tsd_NumAtoms_Put (gtsd_p, i);

  Mem_Alloc (TsdRow_t *, trow_tmp, TSDROWSIZE * i, GLOBAL);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Row allocation in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (sgtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (sgtsd_p), 0, TSDROWSIZE * i);
    });

  Mem_Dealloc (Tsd_AtomHandle_Get (sgtsd_p), TSDROWSIZE * Tsd_NumAtoms_Get (sgtsd_p), GLOBAL);
#endif

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_MatchCompress at %p",
    Tsd_AtomHandle_Get (sgtsd_p)));

  Tsd_AtomHandle_Put (sgtsd_p, trow_tmp);
  Tsd_NumAtoms_Put (sgtsd_p, i);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_PARAMS, (outbuf,
    "Entering Tsd_MatchCompress, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_MatchCompress_Fix
*
*    This routine compresses two TSDs of the same size.  The caller implies
*    they are related as goal and sub-goal.  There are several things to do:
*      1) compress each row (may have to remove or alter this for stereochemistry);
*      2) eliminate any unused rows (atoms) WITHOUT messing up the rest of the TSD;
*      3) look for unattached atoms in the goal pattern that interrupt the contiguous
*         parts and move these to the end.  (This was not accomplished successfully
*         in the modification of Tsd_MatchCompress, but that version isn't suitable
*         for its intended purpose anyway.)
*      4) actually reallocate the memory used by the rows down to the minimum size needed.
*
*  Used to be:
*
*    Tsd_MatchCompress:
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
*    Allocates memory
*    Deallocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
void Tsd_MatchCompress_Fix
  (
  Tsd_t         *gtsd_p,                    /* Goal TSD */
  Tsd_t         *sgtsd_p,                   /* Subgoal TSD (must match) */
  U32_t         *roots                      /* incomplete without this!!! */
  )
{
  U16_t          i, j, root_inx;         /* Counters */
  TsdRow_t      *trow_tmp;                  /* Temp for shuffling the atoms */
  Boolean_t     *lone_gpatom, *lone_sgpatom, sorted, lone_temp;

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_PARAMS, (outbuf,
    "Entering Tsd_MatchCompress_Fix, goal = %p, subgoal = %p", gtsd_p, sgtsd_p));

  ASSERT (gtsd_p != NULL && sgtsd_p != NULL
      && Tsd_NumAtoms_Get (gtsd_p) == Tsd_NumAtoms_Get (sgtsd_p),
    {
    DEBUG (R_TSD, DB_TSDMATCOMP, TL_ALWAYS, (outbuf,
      "TSDs not suited to compression in Tsd_MatchCompress_Fix"));

    IO_Exit_Error (R_TSD, X_SYNERR, "Assert failure");
    });

  /* 1) Compress each atom's representation in both the goal and sub-goal TSDs */

  for (i = 0; i < Tsd_NumAtoms_Get (gtsd_p); i++)
    {
    SRowCompress (gtsd_p, i);
    SRowCompress (sgtsd_p, i);
    }

  /* 2) Sort all the disconnected goal pattern atoms to the end of the TSD.
        If ALL neighbors are TSD_INVALID, then the atom (as represented by
        the row) is non-existent.
  */

#ifdef _MIND_MEM_
  mind_malloc ("lone_gpatom", "tsd{6}", &lone_gpatom, Tsd_NumAtoms_Get (gtsd_p) * sizeof (Boolean_t));
  mind_malloc ("lone_sgpatom", "tsd{6}", &lone_sgpatom, Tsd_NumAtoms_Get (gtsd_p) * sizeof (Boolean_t));
#else
  lone_gpatom = (Boolean_t *) malloc (Tsd_NumAtoms_Get (gtsd_p) * sizeof (Boolean_t));
  lone_sgpatom = (Boolean_t *) malloc (Tsd_NumAtoms_Get (gtsd_p) * sizeof (Boolean_t));
#endif

  for (i = 0; i < Tsd_NumAtoms_Get (gtsd_p); i++)
    for (j = 0, lone_gpatom[i] = lone_sgpatom[i] = TRUE; j < MX_NEIGHBORS && lone_gpatom[i]; j++)
    {
    if (Tsd_Atom_NeighborId_Get (gtsd_p, i, j) != TSD_INVALID) lone_gpatom[i] = FALSE;
    if (Tsd_Atom_NeighborId_Get (sgtsd_p, i, j) != TSD_INVALID) lone_sgpatom[i] = FALSE;
    }

  for (i = 0, sorted = FALSE; i < Tsd_NumAtoms_Get (gtsd_p) - 1 && !sorted; i++) if (lone_gpatom[i])
    for (j = i + 1, sorted = TRUE; j < Tsd_NumAtoms_Get (gtsd_p) && sorted; j++) if (!lone_gpatom[j])
    {
    sorted = FALSE;
    lone_temp = lone_gpatom[i];
    lone_gpatom[i] = lone_gpatom[j];
    lone_gpatom[j] = lone_temp;
    lone_temp = lone_sgpatom[i];
    lone_sgpatom[i] = lone_sgpatom[j];
    lone_sgpatom[j] = lone_temp;
    for (root_inx = 0; root_inx < MX_ROOTS && roots[root_inx] != REACT_NODE_INVALID; root_inx++)
      {
      if (roots[root_inx] == i) roots[root_inx] = j;
      else if (roots[root_inx] == j) roots[root_inx] = i;
      }
    SRowSwap (gtsd_p, sgtsd_p, i, j);
    }

  /* 3) Sort all the empty rows (eliminated atoms) to the end of the TSD.
        If ALL neighbors are TSD_INVALID, then the atom (as represented by
        the row) is non-existent.
  */

  for (i = 0, sorted = FALSE; i < Tsd_NumAtoms_Get (gtsd_p) - 1 && !sorted; i++) if (lone_gpatom[i] && lone_sgpatom[i])
    for (j = i + 1, sorted = TRUE; j < Tsd_NumAtoms_Get (gtsd_p) && sorted; j++) if (!lone_sgpatom[j])
    {
    sorted = FALSE;
    lone_temp = lone_gpatom[i];
    lone_gpatom[i] = lone_gpatom[j];
    lone_gpatom[j] = lone_temp;
    lone_temp = lone_sgpatom[i];
    lone_sgpatom[i] = lone_sgpatom[j];
    lone_sgpatom[j] = lone_temp;
    for (root_inx = 0; root_inx < MX_ROOTS && roots[root_inx] != REACT_NODE_INVALID; root_inx++)
      {
      if (roots[root_inx] == i) roots[root_inx] = j;
      else if (roots[root_inx] == j) roots[root_inx] = i;
      }
    SRowSwap (gtsd_p, sgtsd_p, i, j);
    }

  /* 4) Figure out the number of atoms that are really still in the TSDs - isn't this MUCH simpler AND less opaque? */

  for (i = Tsd_NumAtoms_Get (gtsd_p); i > 0 && lone_gpatom[i - 1] && lone_sgpatom[i - 1]; i--);

#ifdef _MIND_MEM_
  mind_free ("lone_gpatom", "tsd", lone_gpatom);
  mind_free ("lone_sgpatom", "tsd", lone_sgpatom);

  /* Allocate new sets of TsdRow_t's and and copy the old ones in and
     then destroy them.
  */
  mind_malloc ("trow_tmp", "tsd{6}", &trow_tmp, TSDROWSIZE * i);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Rows in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (gtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (gtsd_p), 0, TSDROWSIZE * i);
    });

  mind_free ("Tsd_AtomHandle_Get(gtsd_p)", "tsd", Tsd_AtomHandle_Get (gtsd_p));

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_MatchCompress at %p",
    Tsd_AtomHandle_Get (gtsd_p)));

  Tsd_AtomHandle_Put (gtsd_p, trow_tmp);
  Tsd_NumAtoms_Put (gtsd_p, i);

  mind_malloc ("trow_tmp", "tsd{6a}", &trow_tmp, TSDROWSIZE * i);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Row allocation in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (sgtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (sgtsd_p), 0, TSDROWSIZE * i);
    });

  mind_free ("Tsd_AtomHandle_Get(sgtsd_p)", "tsd", Tsd_AtomHandle_Get (sgtsd_p));
#else
  free (lone_gpatom);
  free (lone_sgpatom);

  /* Allocate new sets of TsdRow_t's and and copy the old ones in and
     then destroy them.
  */
  Mem_Alloc (TsdRow_t *, trow_tmp, TSDROWSIZE * i, GLOBAL);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Rows in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (gtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (gtsd_p), 0, TSDROWSIZE * i);
    });

  Mem_Dealloc (Tsd_AtomHandle_Get (gtsd_p), TSDROWSIZE * Tsd_NumAtoms_Get (gtsd_p), GLOBAL);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_MatchCompress at %p",
    Tsd_AtomHandle_Get (gtsd_p)));

  Tsd_AtomHandle_Put (gtsd_p, trow_tmp);
  Tsd_NumAtoms_Put (gtsd_p, i);

  Mem_Alloc (TsdRow_t *, trow_tmp, TSDROWSIZE * i, GLOBAL);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_MatchCompress at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Row allocation in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (sgtsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (sgtsd_p), 0, TSDROWSIZE * i);
    });

  Mem_Dealloc (Tsd_AtomHandle_Get (sgtsd_p), TSDROWSIZE * Tsd_NumAtoms_Get (sgtsd_p), GLOBAL);
#endif

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_MatchCompress at %p",
    Tsd_AtomHandle_Get (sgtsd_p)));

  Tsd_AtomHandle_Put (sgtsd_p, trow_tmp);
  Tsd_NumAtoms_Put (sgtsd_p, i);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_PARAMS, (outbuf,
    "Entering Tsd_MatchCompress, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Tsd_Strip_H
*
*    This routine strips hydrogens from symmetric or dont-care carbons in a TSD.
*
*  Used to be:
*
*    chkval:
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
void Tsd_Strip_H
  (
  Tsd_t         *tsd_p,                      /* Molecule to strip */
  Boolean_t      dont_care
  )
{
  TsdRow_t *trow_tmp, trow_b;
  int i, j, k, l, nbrid, temp, tindex, tneighbor;
  Boolean_t row_changed, tsd_changed;

  for (i = 0, tsd_changed = FALSE; i < Tsd_NumAtoms_Get (tsd_p); i++)
    {
    row_changed = FALSE;
    if (Tsd_Atomid_Get (tsd_p, i) == 6 /* C */ &&
       (dont_care ||
        !Tsd_AtomFlags_Asymmetry_Get (tsd_p, i) ||
         Tsd_AtomFlags_DontCare_Get (tsd_p, i)))
      for (j = 0; j < MX_NEIGHBORS; j++)
      {
      nbrid = Tsd_Atom_NeighborId_Get (tsd_p, i, j);
      if (nbrid != TSD_INVALID && Tsd_Atomid_Get (tsd_p, nbrid) == 1 /* H */)
        {
        row_changed = tsd_changed = TRUE;
        Tsd_Atomid_Put (tsd_p, nbrid, TSD_INVALID);
        Tsd_Atom_Disconnect (tsd_p, i, nbrid);
        }
      }
    if (row_changed) SRowCompress (tsd_p, i);
    }

  if (!tsd_changed) return;

  for (i = 0, j = Tsd_NumAtoms_Get (tsd_p); i < j; )
    {
    while (Tsd_Atomid_Get (tsd_p, i) != TSD_INVALID && i < Tsd_NumAtoms_Get (tsd_p))
      i++;
    do --j; while (Tsd_Atomid_Get (tsd_p, j) == TSD_INVALID && j > i);

    if (i < j) SRowSwap (tsd_p, NULL, i, j);
    }


  /* Figure out the number of atoms that are really still in the TSD */

  for (i = Tsd_NumAtoms_Get (tsd_p);
    i > 0 && Tsd_Atomid_Get (tsd_p, i - 1) == TSD_INVALID; i--);

  /* Allocate new set of TsdRow_t's and and copy the old ones in and
     then destroy them.
  */
#ifdef _MIND_MEM_
  mind_malloc ("trow_tmp", "tsd{7}", &trow_tmp, TSDROWSIZE * i);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_Strip_H at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Rows in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (tsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (tsd_p), 0, TSDROWSIZE * i);
    });

  mind_free ("Tsd_AtomHandle_Get(tsd_p)", "tsd", Tsd_AtomHandle_Get (tsd_p));
#else
  Mem_Alloc (TsdRow_t *, trow_tmp, TSDROWSIZE * i, GLOBAL);

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Allocated Tsd Rows in Tsd_Strip_H at %p", trow_tmp));

  if (trow_tmp == NULL)
    IO_Exit_Error (R_TSD, X_LIBCALL,
      "No memory for Tsd Rows in Tsd_MatchCompress");

  (void) memcpy (trow_tmp, Tsd_AtomHandle_Get (tsd_p), TSDROWSIZE * i);

  DEBUG_DO (DB_TSDMATCOMP, TL_MEMORY,
    {
    memset (Tsd_AtomHandle_Get (tsd_p), 0, TSDROWSIZE * i);
    });

  Mem_Dealloc (Tsd_AtomHandle_Get (tsd_p), TSDROWSIZE * Tsd_NumAtoms_Get (tsd_p), GLOBAL);
#endif

  DEBUG (R_TSD, DB_TSDMATCOMP, TL_MEMORY, (outbuf,
    "Deallocated Tsd Rows in Tsd_MatchCompress at %p",
    Tsd_AtomHandle_Get (tsd_p)));

  Tsd_AtomHandle_Put (tsd_p, trow_tmp);
  Tsd_NumAtoms_Put (tsd_p, i);
}

/****************************************************************************
*
*  Function Name:                 Tsd_Verify
*
*    This routine verifys that a TSD is consistent.
*
*  Used to be:
*
*    chkval:
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
void Tsd_Verify
  (
  Tsd_t         *tsd_p                      /* Molecule to look in */
  )
{
  U16_t          i;                         /* Counter */
  U16_t          val_tmp;                   /* Temp for valences */

  DEBUG (R_TSD, DB_TSDVERIFY, TL_PARAMS, (outbuf,
    "Entering Tsd_Verify, tsd = %p", tsd_p));

  if (tsd_p == NULL)
    return;

  for (i = 0; i < Tsd_NumAtoms_Get (tsd_p); i++)
    {
    val_tmp = Tsd_Atom_Valence_Get (tsd_p, i);
    if (!val_tmp)
      TRACE (R_TSD, DB_TSDVERIFY, TL_TRACE, (outbuf,
        "In TSD at %p, atom # %u is not connected to any node", tsd_p, i));

    if (val_tmp > Atomid_MaxValence (Tsd_Atomid_Get (tsd_p, i)))
      TRACE (R_TSD, DB_TSDVERIFY, TL_TRACE, (outbuf,
        "In TSD at %p, atom # %u with valence %u has exceeded its normal\
 valence.",
        tsd_p, i, val_tmp));
    }

  DEBUG (R_TSD, DB_TSDVERIFY, TL_PARAMS, (outbuf,
    "Leaving Tsd_Verify, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SNeighborSwitch
*
*    This routine switches two neighbors of an atom.  This means both their
*    ids and their bonds.
*
*  Used to be:
*
*    switch:
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
static void SNeighborSwitch
  (
  Tsd_t         *tsd_p,                     /* TSD we are looking at */
  U16_t          atom,                      /* Atom we are looking at */
  U8_t           neighbor1,                 /* Neighbor to switch */
  U8_t           neighbor2                  /* Neighbor to switch with */
  )
{
  U16_t          temp;                      /* For switching */

  temp = Tsd_Atom_NeighborBond_Get (tsd_p, atom, neighbor1);
  Tsd_Atom_NeighborBond_Put (tsd_p, atom, neighbor1,
    Tsd_Atom_NeighborBond_Get (tsd_p, atom, neighbor2));
  Tsd_Atom_NeighborBond_Put (tsd_p, atom, neighbor2, temp);

  temp = Tsd_Atom_NeighborId_Get (tsd_p, atom, neighbor1);
  Tsd_Atom_NeighborId_Put (tsd_p, atom, neighbor1,
    Tsd_Atom_NeighborId_Get (tsd_p, atom, neighbor2));
  Tsd_Atom_NeighborId_Put (tsd_p, atom, neighbor2, temp);

  return;
}

/****************************************************************************
*
*  Function Name:                 SRowCompress
*
*    This routine compresses a single row in a TSD.  It simply folds all
*    the neighbors to the front using SNeighborSwap of a lower numbered
*    empty neighbor with a higher numbered valid neighbor.
*
*  Used to be:
*
*    pack_row:
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
static void SRowCompress
  (
  Tsd_t         *tsd_p,                     /* Molecule we are looking at */
  U16_t          row                        /* Row to compress */
  )
{
  U8_t           start, end;                /* Counter */

/* FIRST! Get rid of zero-bonded neighbors!!! */
  for (start = 0; start < MX_NEIGHBORS; start++)
    if (Tsd_Atom_NeighborBond_Get (tsd_p, row, start) == BOND_NONE)
        Tsd_Atom_NeighborId_Put (tsd_p, row, start, TSD_INVALID);

  for (start = 0, end = MX_NEIGHBORS; start < end;)
    {
/*
    while (Tsd_Atom_NeighborId_Get (tsd_p, row, start) != TSD_INVALID)
      start++;
    while (Tsd_Atom_NeighborId_Get (tsd_p, row, end) == TSD_INVALID)
      end--;
*/
    while (Tsd_Atom_NeighborId_Get (tsd_p, row, start) != TSD_INVALID && start < MX_NEIGHBORS)
      start++;
    do --end; while (Tsd_Atom_NeighborId_Get (tsd_p, row, end) == TSD_INVALID && end > start);

    if (start < end)
      SNeighborSwitch (tsd_p, row, start, end);
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 SRowSwap
*
*    This routine swaps two rows in each of two TSDs, this is a utility
*    routine for Tsd_MatchCompress.  The same rows are swapped in each
*    TSD.  The difficult part is figuring out how to make sure all the atoms
*    that are neighbors of each other remain connected.
*
*    The algorithm is not currently deciphered ???
*
*    GAM (6/30/1999): Generalized - now it can be used with ONE, as well as TWO TSD's -
*                     just make the the second one NULL!
*
*  Used to be:
*
*    swap:
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
static void SRowSwap
  (
  Tsd_t         *tsd1_p,                    /* Molecule we are looking at */
  Tsd_t         *tsd2_p,                    /* Molecule we are looking at */
  U16_t          row1,                      /* Swap row1 and row2 */
  U16_t          row2                       /* Swap row1 and row2 */
  )
{
  TsdRow_t       trow_b;                    /* For swaping row entries */
  U16_t          k, l;                      /* Counters */
  U16_t          temp;                      /* Not sure ??? */
  U16_t          tindex;                    /* Temp for index of neighbor */
  U16_t          tneighbor;                 /* Temp for neighbor */

  /* Swap rows in TSD #1 and #2 */

  trow_b = tsd1_p->atoms[row1];
  tsd1_p->atoms[row1] = tsd1_p->atoms[row2];
  tsd1_p->atoms[row2] = trow_b;

  if (tsd2_p != NULL)
  {
    trow_b = tsd2_p->atoms[row1];
    tsd2_p->atoms[row1] = tsd2_p->atoms[row2];
    tsd2_p->atoms[row2] = trow_b;
  }

  temp = row1 + row2;

  /* Fix up TSD # 1.  For the two rows that were swapped, fix up their
     neighbors (including themselves) to point to the new correct
     atom indexes.
  */

  for (k = row1; k <= row2; k += row2 - row1)
    {
    for (l = 0; l < MX_NEIGHBORS; l++)
      {
      tneighbor = Tsd_Atom_NeighborId_Get (tsd1_p, k, l);
      if (tneighbor != TSD_INVALID && tneighbor != k)
        {
        tindex = Tsd_Atom_Index_Find (tsd1_p, tneighbor, temp - k);
        if (tindex != TSD_INVALID)
          Tsd_Atom_NeighborId_Put (tsd1_p, tneighbor, tindex, k);
        }
      else
        if (tneighbor != TSD_INVALID)
          {
          tindex = Tsd_Atom_Index_Find (tsd1_p, tneighbor, k);
          if (tindex != TSD_INVALID)
            Tsd_Atom_NeighborId_Put (tsd1_p, tneighbor, tindex, temp - k);
          }
      }
    }

  if (tsd2_p == NULL) return;

  /* Fix up TSD # 2.  For the two rows that were swapped, fix up their
     neighbors (including themselves) to point to the new correct
     atom indexes.
  */

  for (k = row1; k <= row2; k += row2 - row1)
    {
    for (l = 0; l < MX_NEIGHBORS; l++)
      {
      tneighbor = Tsd_Atom_NeighborId_Get (tsd2_p, k, l);
      if (tneighbor != TSD_INVALID && tneighbor != k)
        {
        tindex = Tsd_Atom_Index_Find (tsd2_p, tneighbor, temp - k);
        if (tindex != TSD_INVALID)
          Tsd_Atom_NeighborId_Put (tsd2_p, tneighbor, tindex, k);
        }
      else
        if (tneighbor != TSD_INVALID)
          {
          tindex = Tsd_Atom_Index_Find (tsd2_p, tneighbor, k);
          if (tindex != TSD_INVALID)
            Tsd_Atom_NeighborId_Put (tsd2_p, tneighbor, tindex, temp - k);
          }
      }
    }


  return;
}
/* End of SRowSwap */
/* End of TSD.C */
