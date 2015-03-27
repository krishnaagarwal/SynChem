/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RESONANTBONDS.C
*
*    This module contains the routines for finding resonant bonds in an XTR.
*
*  Routines:
*
*    ResonantBonds_Find
*    ResonantBonds_FindSpiro
*    ResonantBonds_Fix
*    SAllAtomsSingleDouble
*    SAtomValence_Get
*    SBonds_Fix
*    SBonds_Invert
*    SBonds_Mark
*    SBranch_Remove
*    SCalculate
*    SChain_Fix
*    SDuplicates_Prune
*    SFRBonds_Top
*    SGeneric_Compound_Find
*    SImpossible_Sequences_Remove
*    SImprove
*    SMappedTo
*    SNot_Worth_Checking
*    SParallel
*    SParallel_System_Find
*    SPaths_Find_Configs_Add
*    SPursue
*    SRelation
*    SRemaining_Huckelness_Find
*    SRingSystem_Huckelness_Find
*    SSpiro_Atom
*    SSpiro_Compound_Find
*    STables_Free
*    STraverse
*    SValence4_Compound_Find
*    SValence4_Ring_Find
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
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_RESONANTBONDS_
#include "resonantbonds.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/* Static Routine Prototypes */

static Boolean_t SAllAtomsSingleDouble  (Xtr_t *);
static void      SAnalyze_H_Subgoals    (Xtr_t *, U16_t, U8_t, U8_t, U8_t,
  ResbondsCB_t *);
static U8_t      SAtomValence_Get       (Xtr_t *, U16_t);
static Boolean_t SBonds_Fix             (U16_t, U16_t, Xtr_t *, Array_t *,
  Array_t *, U8_t);
static void      SBonds_Invert          (Array_t *, U32_t);
static void      SBonds_Mark            (Xtr_t *, Boolean_t, ResbondsCB_t *);
static void      SBranch_Remove         (Xtr_t *, U16_t, U8_t, Array_t *,
  U16_t *);
static Boolean_t SCalculate             (Array_t *, Array_t *, Xtr_t *);
static void      SChain_Fix             (Xtr_t *, U16_t, U16_t, Array_t *,
   U8_t);
static void      SDuplicates_Prune      (AtomArray_t *, List_t *, Xtr_t *);
static void      SFRBonds_Top           (ResbondsCB_t *);
static Xtr_t    *SGeneric_Compound_Find (Xtr_t *, ResbondsCB_t *);
static Xtr_t    *SImpossible_Sequences_Remove (Xtr_t *, U16_t *,
  ResbondsCB_t *);
static void      SImprove               (S16_t, S16_t, U16_t, Xtr_t *,
  Array_t *, Array_t *, Array_t *, Array_t *, Array_t *, Array_t *, Array_t *,
  ResbondsCB_t *);
static U16_t     SMappedTo              (U16_t, ResbondsCB_t *);
static Boolean_t SNot_Worth_Checking    (Xtr_t *, ResbondsCB_t *);
static Boolean_t SParallel              (U16_t, U16_t, Array_t *, Array_t *);
static void      SParallel_System_Find  (Xtr_t *, U16_t, U16_t, Array_t *,
  Array_t *, Array_t *, Array_t *, ResbondsCB_t *);
static void      SPaths_Find_Configs_Add (List_t *, AtomArray_t *, Array_t *,
  Array_t *, Boolean_t *, Array_t *, Array_t *, Xtr_t *);
static Boolean_t SPursue                (U16_t, U8_t, U16_t, List_t *,
  AtomArray_t *, Array_t *, Array_t *, Array_t *, Array_t *, U16_t, Array_t *,
  Xtr_t *);
static void      SRelation              (S16_t, S16_t, U16_t, Xtr_t *,
  Array_t *, Array_t *, Array_t *, Array_t *, Array_t *, Array_t *, Array_t *,
  ResbondsCB_t *);
static void      SRemaining_Huckelness_Find (Xtr_t *, ResbondsCB_t *);
static void      SRingSystem_Huckelness_Find (Xtr_t *, ResbondsCB_t *);
static Boolean_t SSpiro_Atom            (Xtr_t *, U16_t, U16_t);
static void      SSpiro_Compound_Find   (Xtr_t *, ResbondsCB_t *);
static void      STables_Free           (Array_t *, Array_t *, Array_t *,
  Array_t *, Array_t *, Array_t *, Array_t *);
static void      STraverse              (U16_t, U8_t, Xtr_t *, S16_t *,
  Array_t *, Array_t *, Array_t *, Array_t *, Array_t *, Array_t *, Array_t *);
static Xtr_t    *SValence4_Compound_Find (Xtr_t *, ResbondsCB_t *);
static void      SValence4_Ring_Find    (Xtr_t *, ResbondsCB_t *);

/****************************************************************************
*
*  Function Name:                 ResonantBonds_Find
*
*    This routine is a visible entrypoint to finding Resonant Bonds in a
*    molecule.  It sets up the module level static variables and then
*    calls the top level of the search/enumeration.
*
*  Used to be:
*
*    frbond:
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
void ResonantBonds_Find
  (
  Xtr_t         *inp_xtr_p,                  /* Molecule to look at */
  Array_t       *outbonds_p                  /* 2-d bit, resonant bonds */
  )
{
  ResbondsCB_t   rbcb;                       /* Control block for this call */

  DEBUG (R_XTR, DB_RESBONDSFIND, TL_PARAMS, (outbuf,
    "Entering Xtr_ResonantBonds_Find, Xtr = %p, bonds = %p",
    inp_xtr_p, outbonds_p));

  FILL (rbcb, 0);
  rbcb.xtr = inp_xtr_p;
  rbcb.owtput = outbonds_p;
  rbcb.spiro = FALSE;

  SFRBonds_Top (&rbcb);

  DEBUG (R_XTR, DB_RESBONDSFIND, TL_PARAMS, (outbuf,
    "Leaving Xtr_ResonantBonds_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 ResonantBonds_FindSpiro
*
*    Same as ResonantBonds_Find except that it works on Spiro atoms
*    as well.
*
*  Used to be:
*
*    sfrbond:
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
void ResonantBonds_FindSpiro
  (
  Xtr_t         *inp_xtr_p,                  /* Molecule to look at */
  Array_t       *outbonds_p                  /* 2-d bit, resonant bonds */
  )
{
  ResbondsCB_t   rbcb;                       /* Control block for this call */

  DEBUG (R_XTR, DB_RESBONDSFINDSPIRO, TL_PARAMS, (outbuf,
    "Entering Xtr_ResonantBonds_FindSpiro, Xtr = %p, bonds = %p",
    inp_xtr_p, outbonds_p));

  FILL (rbcb, 0);
  rbcb.xtr = inp_xtr_p;
  rbcb.owtput = outbonds_p;
  rbcb.spiro = TRUE;

  SFRBonds_Top (&rbcb);

  DEBUG (R_XTR, DB_RESBONDSFINDSPIRO, TL_PARAMS, (outbuf,
    "Leaving Xtr_ResonantBonds_FindSpiro, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 ResonantBonds_Fix
*
*    This routine modifies an XTR by fixing the resonant bonds into either
*    single or double bonds.  Which bonds get which values depends on the
*    starting atom and which bond is given which value.
*
*  Used to be:
*
*    unrbond:
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
void ResonantBonds_Fix
  (
  Xtr_t         *xtr_p,                      /* Molecule to fix the bonds of */
  U16_t          atom_idx,                   /* Atom to start at */
  U8_t           neigh_idx,                  /* Nbr whose bond is tobe set */
  U8_t           bondsize                    /* Value for first bond */
  )
{
  Xtr_t         *xtr_tmp;                    /* Subset w/ only resonant bnds */
  U16_t          atom;                       /* Counter */
  U16_t          atom1;                      /* Mapped versions of atom_idx */
  U16_t          atom2;                      /* Mapped version of neigh_idx */
  U16_t          num_atoms;                  /* Number of atoms in m'cule */
  U16_t          num_atoms_tmp;              /* # atoms in subset of m'cule */
  U16_t          neigh;                      /* Counter */
  U8_t           num_neighbors;              /* Compiler bug */
  Array_t        bonds;                      /* 2-d bit, scratch */
  Array_t        map;                        /* 1-d word, mapping of rbonds */
  Array_t        conformation;               /* 2-d byte, new bond size */
  Array_t        resbonds;                   /* 2-d bit, scratch */

  DEBUG (R_XTR, DB_RESBONDSFIX, TL_PARAMS, (outbuf,
    "Entering ResonantBonds_Fix, Xtr = %p, start atom = %u,"
    " start neighbor = %hu, bondsize = %hu", xtr_p, atom_idx, neigh_idx, 
    bondsize));

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "resonantbond{1}s", &bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
  mind_Array_1d_Create ("&map", "resonantbond{1}s", &map, num_atoms, WORDSIZE);

  for (atom = 0; atom < num_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = 0; neigh < num_neighbors; neigh++)
      if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) == BOND_RESONANT)
        Array_2d1_Put (&bonds, atom, neigh, TRUE);
      else
        Array_2d1_Put (&bonds, atom, neigh, FALSE);
    }

  xtr_tmp = Xtr_CopySubset (xtr_p, &bonds, &map);
  mind_Array_Destroy ("&bonds", "resonantbonds", &bonds);
  num_atoms_tmp = Xtr_NumAtoms_Get (xtr_tmp);

  mind_Array_2d_Create ("&conformation", "resonantbonds{1}", &conformation, num_atoms_tmp, num_atoms_tmp, BYTESIZE);
  mind_Array_2d_Create ("&resbonds", "resonantbonds{1}", &resbonds, num_atoms_tmp, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
  Array_1d_Create (&map, num_atoms, WORDSIZE);

  for (atom = 0; atom < num_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = 0; neigh < num_neighbors; neigh++)
      if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) == BOND_RESONANT)
        Array_2d1_Put (&bonds, atom, neigh, TRUE);
      else
        Array_2d1_Put (&bonds, atom, neigh, FALSE);
    }

  xtr_tmp = Xtr_CopySubset (xtr_p, &bonds, &map);
  Array_Destroy (&bonds);
  num_atoms_tmp = Xtr_NumAtoms_Get (xtr_tmp);

  Array_2d_Create (&conformation, num_atoms_tmp, num_atoms_tmp, BYTESIZE);
  Array_2d_Create (&resbonds, num_atoms_tmp, MX_NEIGHBORS, BITSIZE);
#endif
  Array_Set (&conformation, BOND_NONE);

  /* Find mapping in new XTR of original atom index and atom index of
     neighbor
  */

  for (atom = 0; Array_1d16_Get (&map, atom) != atom_idx && atom <
       num_atoms_tmp; atom++)
    /* Empty loop body */ ;

  atom1 = atom;
  atom2 = Xtr_Attr_NeighborId_Get (xtr_p, atom_idx, neigh_idx);

  for (atom = 0; Array_1d16_Get (&map, atom) != atom2 && atom < num_atoms_tmp;
       atom++)
    /* Empty loop body */ ;

  atom2 = atom;

  (void) SBonds_Fix (atom1, atom2, xtr_tmp, &conformation, &resbonds,
    bondsize);

  Xtr_Destroy (xtr_tmp);

  for (atom = 0; atom < num_atoms_tmp; atom++)
    for (neigh = 0; neigh < num_atoms_tmp; neigh++)
      {
      TRACE_DO (DB_RESBONDSFIX, TL_MAJOR,
        {
        TRACE (R_XTR, DB_RESBONDSFIX, TL_TRACE, (outbuf,
          "Before setting bond in Resbonds_Fix, index1 = %u, index2 = %u,"
          " map (first) = %u, map (second) = %u, conformation (1, 2) = %hu",
          atom, neigh, Array_1d16_Get (&map, atom), Array_1d16_Get (&map,
          neigh), Array_2d8_Get (&conformation, atom, neigh)));
        });

      atom1 = Array_1d16_Get (&map, atom);
      atom2 = Array_1d16_Get (&map, neigh);
      if (Array_2d8_Get (&conformation, atom, neigh) != BOND_NONE)
        Xtr_Attr_NeighborBond_Put (xtr_p, atom1, Xtr_Attr_NeighborIndex_Find (
          xtr_p, atom1, atom2), Array_2d8_Get (&conformation, atom,
          neigh));
      }

  TRACE_DO (DB_RESBONDSFIX, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In ResonantBonds_Fix");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&map", "resonantbonds", &map);
  mind_Array_Destroy ("&resbonds", "resonantbonds", &resbonds);
  mind_Array_Destroy ("&conformation", "resonantbonds", &conformation);
#else
  Array_Destroy (&map);
  Array_Destroy (&resbonds);
  Array_Destroy (&conformation);
#endif

  DEBUG (R_XTR, DB_RESBONDSFIX, TL_PARAMS, (outbuf,
    "Leaving ResonantBonds_Fix, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SAllAtomsSingleDouble
*
*    This function checks to make sure that all atoms are have both single
*    and double bonds.  This is necessary for atoms to be part of ABCs,
*    Alternating Bond Cycles.  See Boivie's thesis.
*
*  Used to be:
*
*    allatomsingleanddouble:
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
*    True - the are
*    False - they aren't
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SAllAtomsSingleDouble
  (
  Xtr_t         *xtr_p                       /* Molecule to check */
  )
{
  U16_t          atom;                       /* Counter */
  U8_t           neigh;                      /* Counter */
  U8_t           found;                      /* Flag */
  U8_t           num_neighbors;              /* Compiler bug */

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SAllAtomsSingleDouble");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  for (atom = 0; atom < Xtr_NumAtoms_Get (xtr_p); atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = found = 0; neigh < num_neighbors && found != 0x3; neigh++)
      {
      if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) == BOND_DOUBLE)
        found |= 0x2;
      else
        if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) == BOND_SINGLE)
          found |= 0x1;
      }

    if (found != 0x3)
      return FALSE;
    }

   return TRUE;
}

/****************************************************************************
*
*  Function Name:                 SAnalyze_H_Subgoals
*
*    This routine analyzes hetero-atom subgoals.  It removes branches
*    based on the bond size that is passed in and then it calls
*    SGeneric_Compound_Find to check the remaining portion of the molecule.
*    This search is just for ABC bonds, not resonant bonds.
*
*  Used to be:
*
*    analyze_h_subgoals:
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
static void SAnalyze_H_Subgoals
  (
  Xtr_t         *xtr_p,                      /* Molecule to check */
  U16_t          atom,                       /* Which atom are we after */
  U8_t           bondsize,                   /* Strength of bonds */
  U8_t           num_bonds,                  /* Number of bonds */
  U8_t           num_neighbors,              /* Number of neighbors */
  ResbondsCB_t  *rbcb_p                      /* Control block */
  )
{
  Xtr_t         *txtr_p;                     /* For recursion */
  AtomArray_t   *bondarray_p;                /* Temp. bond array */
  AtomArray_t   *map_p;                      /* New mapping array */
  Array_t       *array_p;                    /* Temporary */
  U16_t          num_atoms;                  /* Number of atoms in m'cule */
  U16_t          i, count;                   /* Counters */
  U16_t          atoms_removed;              /* Number of atoms removed */
  U8_t           last_bond;                  /* Last bond index */
  U8_t           neigh_index;                /* Counter */

  if (SNot_Worth_Checking (xtr_p, rbcb_p))
    return;

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SAnalyze_H_Subgoals, Xtr = %p, atom = %u, bondsize = %hu,"
    " # bonds = %hu, # neighbors = %hu", xtr_p, atom, bondsize, num_bonds,
    num_neighbors));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SAnalyze_H_Subgoals");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  num_atoms = Xtr_NumAtoms_Get (xtr_p);

  bondarray_p = AtmArr_Create (num_atoms, ATMARR_BONDS_BIT, FALSE);
  array_p = AtmArr_Array_Get (bondarray_p);
  Stack_PushAdd (RsbndCB_Analyze_Get (rbcb_p), bondarray_p);

  /* Look at all the bonds of this atom.  Need zero for the possible first
     use of last_bond, so set to -1 so increment rolls it over.
  */

  for (i = 0, last_bond = -1; i < num_bonds; i++)
    {
    count = num_atoms;

    /* Look for next bond of the correct size.  We need the neighbor index */

    for (neigh_index = last_bond + 1; (neigh_index < num_neighbors) &&
        (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh_index) != bondsize);
         neigh_index++)
      /* Empty loop body */ ;

    last_bond = neigh_index;
    Array_Set (array_p, TRUE);

    /* Mark earlier bonds of the same size as not included.  Make sure the
       selected bond is included.  Remove all atoms that can't be reached.
    */

    for (neigh_index = 0; neigh_index < num_neighbors; neigh_index++)
      if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh_index) == bondsize)
        Array_2d1_Put (array_p, atom, neigh_index, FALSE);

    Array_2d1_Put (array_p, atom, last_bond, TRUE);
    for (neigh_index = 0; neigh_index < num_neighbors; neigh_index++)
      if (Array_2d1_Get (array_p, atom, neigh_index) == FALSE)
        {
        SBranch_Remove (xtr_p, atom, neigh_index, array_p, &atoms_removed);
        count -= atoms_removed;
        }

    map_p = AtmArr_Create (count, ATMARR_NOBONDS_WORD, FALSE);
    Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
    txtr_p = Xtr_CopySubset (xtr_p, array_p, AtmArr_Array_Get (map_p));
    txtr_p = SGeneric_Compound_Find (txtr_p, rbcb_p);
    Xtr_Destroy (txtr_p);
    Stack_Pop (RsbndCB_Map_Get (rbcb_p));
    }                                        /* End of num-bonds loop */

  Stack_Pop (RsbndCB_Analyze_Get (rbcb_p));

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving Analyze_H_Subgoals, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SAtomValence_Get
*
*    This function returns the total of all the bond multiplicities of the
*    given atom.  This is the valence of the atom.
*
*  Used to be:
*
*    valence:
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
*    Total valence of the atom
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U8_t SAtomValence_Get
  (
  Xtr_t         *xtr_p,                      /* Molecule to check */
  U16_t          atom                        /* Which atom are we after */
  )
{
  U8_t           neigh;                      /* Counter */
  U8_t           sum;                        /* Counter */
  U8_t           bondsize;                   /* Bond strength from atom */
  U8_t           num_neighbors;              /* Compiler bug */

  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
  for (neigh = 0, sum = 0; neigh < num_neighbors; neigh++)
    {
    bondsize = Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh);

    ASSERT (bondsize < BOND_TRIPLE,
      {
      IO_Exit_Error (R_XTR, X_SYNERR,
        "Bond multiplicity too large in SValence in ResonantBonds");
      });

    sum += bondsize;
    }

  return sum;
}

/****************************************************************************
*
*  Function Name:                 SBonds_Fix
*
*    This routine fixes the bonds according to the parameters.
*
*  Used to be:
*
*    fix_bonds:
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
*    Flag for whether we are completely done (unwind recursion stack)
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SBonds_Fix
  (
  U16_t          atom1,                      /* Atom to start fixing at */
  U16_t          atom2,                      /* Neighbor at other end */
  Xtr_t         *xtr_p,                      /* Subset molecule */
  Array_t       *conformation_p,             /* 2-d byte, new bond values */
  Array_t       *resbonds_p,                 /* 1-d bit, ResonantBonds_Find */
  U8_t           bondsize                    /* Bond multiplicity to set */
  )
{
  Xtr_t         *xtr_tmp;                    /* Copy of XTR */
  U16_t          neighid;                    /* Atom index of neighbor */
  U16_t          atom;                       /* Counter */
  U16_t          num_atoms;                  /* Temporary */
  U16_t          neigh;                      /* Counter */
  U8_t           num_neighbors;              /* Compiler bug */
  Boolean_t      notallres;                  /* Flag */
  Boolean_t      done;                       /* Status flag from recursion */
  Array_t        newconform;                 /* For recursion */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SBonds_Fix, atom1 = %u, atom2 = %u, conformation = %p,"
    " bonds = %hu", atom1, atom2, conformation_p, bondsize));

  SChain_Fix (xtr_p, atom1, atom2, conformation_p, bondsize);
  SChain_Fix (xtr_p, atom2, atom1, conformation_p, bondsize);

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
  for (atom = 0, notallres = FALSE; (atom < num_atoms) && (notallres == FALSE);
       atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = 0; (neigh < num_neighbors) && (notallres == FALSE); neigh++)
      {
      neighid = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);
      if (Array_2d8_Get (conformation_p, atom, neighid) == BOND_NONE)
        notallres = TRUE;
      }
    }

  if (notallres == FALSE)
    {
    xtr_tmp = Xtr_CopyExpand (xtr_p, 0);
    for (atom = 0; atom < num_atoms; atom++)
      for (neigh = 0; neigh < num_atoms; neigh++)
        if (Array_2d8_Get (conformation_p, atom, neigh) != BOND_NONE)
          Xtr_Attr_NeighborBond_Put (xtr_tmp, atom,
            Xtr_Attr_NeighborIndex_Find (xtr_tmp, atom, neigh), 
            Array_2d8_Get (conformation_p, atom, neigh));

    Xtr_Rings_Set (xtr_tmp);
    ResonantBonds_Find (xtr_tmp, resbonds_p);

    for (atom = 0; atom < num_atoms; atom++)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_tmp, atom);
      for (neigh = 0; neigh < num_neighbors; neigh++)
        if (Array_2d1_Get (resbonds_p, atom, neigh) == FALSE)
          {
          Xtr_Destroy (xtr_tmp);
          return FALSE;
          }
      }

    Xtr_Destroy (xtr_tmp);

    /* Need to return conformation array all the way back to the original
       call site.  See below for mechanism.
    */

    return TRUE;
    }                             /* End of if-!notallres */

#ifdef _MIND_MEM_
  mind_Array_Copy ("&newconform", "resonantbonds{2}", conformation_p, &newconform);
  atom--;
  done = SBonds_Fix (atom, neighid, xtr_p, &newconform, resbonds_p,
    BOND_SINGLE);

  if (done == FALSE)
    {
    mind_Array_Destroy ("&newconform", "resonantbonds", &newconform);
    done = SBonds_Fix (atom, neighid, xtr_p, conformation_p, resbonds_p,
      BOND_DOUBLE);
    }
  else
    {
    /* In case where TRUE is returned, the conformation array at that time
       must be returned all the way up the stack
    */

    mind_Array_Destroy ("conformation_p", "resonantbonds", conformation_p);
    *conformation_p = newconform;
    }
#else
  Array_Copy (conformation_p, &newconform);
  atom--;
  done = SBonds_Fix (atom, neighid, xtr_p, &newconform, resbonds_p,
    BOND_SINGLE);

  if (done == FALSE)
    {
    Array_Destroy (&newconform);
    done = SBonds_Fix (atom, neighid, xtr_p, conformation_p, resbonds_p,
      BOND_DOUBLE);
    }
  else
    {
    /* In case where TRUE is returned, the conformation array at that time
       must be returned all the way up the stack
    */

    Array_Destroy (conformation_p);
    *conformation_p = newconform;
    }
#endif

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SBonds_Fix, status = <void>"));

  return done;
}

/****************************************************************************
*
*  Function Name:                 SBonds_Invert
*
*    This function inverts the bond mask value from true to false and
*    vice-versa.
*
*  Used to be:
*
*    inverbond_array_t:
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
static void SBonds_Invert
  (
  Array_t       *bonds_p,                   /* Array to invert */
  U32_t          num_atoms                  /* Size of array */
  )
{
  U32_t          i;
  U16_t          j;                      /* Counters */

  for (i = 0; i < num_atoms; i++)
    for (j = 0; j < MX_NEIGHBORS; j++)
      Array_2d1_Put (bonds_p, i, j, (Array_2d1_Get (bonds_p, i, j) == TRUE) ?
        FALSE : TRUE);

  return;
}

/****************************************************************************
*
*  Function Name:                 SBonds_Mark
*
*    This function marks bonds in the specified molecule subset as being
*    ABC bonds in the original molecule and the flag tells whether
*    they should be marked in the output as resonant bonds or not.
*
*  Used to be:
*
*    mark_bonds:
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
static void SBonds_Mark
  (
  Xtr_t         *xtr_p,                     /* Molecule to mark up */
  Boolean_t      resonance,                 /* Resonant bond flag */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  U16_t          atom;                      /* Counter */
  U16_t          neigh_id;                  /* Neighbor id */
  U16_t          orig_atom;                 /* Original atom index */
  U8_t           num_neighbors;             /* Compiler bug */
  U8_t           neigh;                     /* Counter */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SBonds_Mark, Xtr = %p, resonance = %hu", xtr_p, resonance));

  for (atom = 0; atom < Xtr_NumAtoms_Get (xtr_p); atom++)
    {
    orig_atom = SMappedTo (atom, rbcb_p);
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = 0; neigh < num_neighbors; neigh++)
      {
      neigh_id = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);
      neigh_id = SMappedTo (neigh_id, rbcb_p);
      Array_2d1_Put (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
        Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p), orig_atom,
        neigh_id), TRUE);
      Array_2d1_Put (RsbndCB_ABCbonds_Get (rbcb_p), neigh_id,
        Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p), neigh_id,
        orig_atom), TRUE);

      if (resonance == TRUE)
        {
        Array_2d1_Put (RsbndCB_Output_Get (rbcb_p), orig_atom,
          Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p), orig_atom,
          neigh_id), TRUE);
        Array_2d1_Put (RsbndCB_Output_Get (rbcb_p), neigh_id,
          Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p), neigh_id,
          orig_atom), TRUE);
        }
      }
    }

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SBonds_Mark, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SBranch_Remove
*
*    This routine removes an atom and all atoms chained off it and reachable
*    in no other way which means those with only two neighbors.  The last
*    one in the chain disappears during the call.
*
*  Used to be:
*
*    remove_branch:
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
static void SBranch_Remove
  (
  Xtr_t         *xtr_p,                      /* Molecule to check */
  U16_t          atom,                       /* Branch root to remove */
  U8_t           neigh,                      /* Which neighbor index */
  Array_t       *bonds_p,                    /* 2-d bit, bond mask */
  U16_t         *atoms_removed_p             /* # atoms removed */
  )
{
  U16_t          next;                       /* Neighbor id to look at */
  U16_t          last;                       /* Atom index of previous node */
  U16_t          temp;                       /* For swapping prev and cur. */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SBranch_Remove, Xtr = %p, atom = %u, neigh = %hu, bonds = %p",
    xtr_p, atom, neigh, bonds_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MINOR,
    {
    IO_Put_Trace (R_XTR, "In SBranch_Remove");
    Xtr_Dump (xtr_p, &GTraceFile);
    Array_Dump (bonds_p, &GTraceFile);
    });

  Array_2d1_Put (bonds_p, atom, neigh, FALSE);
  last = atom;
  next = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);
  *atoms_removed_p = 0;

   /* Loop looking for atoms that have further descendants in the chain from
     the root atom that is being removed.  Remove them as well.  If the
     final atom has only one neighbor then we already removed it on our
     way in, otherwise it has more so it can be left intact.
  */

  while ((Xtr_Attr_NumNeighbors_Get (xtr_p, next) == 2)
         && (Array_2d1_Get (bonds_p, next, 0) == TRUE))
    {
    (*atoms_removed_p)++;
    Array_2d1_Put (bonds_p, next, 0, FALSE);
    Array_2d1_Put (bonds_p, next, 1, FALSE);

    temp = next;
    if (Xtr_Attr_NeighborId_Get (xtr_p, next, 0) == last)
      next = Xtr_Attr_NeighborId_Get (xtr_p, next, 1);
    else
      next = Xtr_Attr_NeighborId_Get (xtr_p, next, 0);

    last = temp;
    }

  Array_2d1_Put (bonds_p, next, Xtr_Attr_NeighborIndex_Find (xtr_p,
    next, last), FALSE);

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SBranch_Remove, atoms removed = %u", *atoms_removed_p));

  return;
}

/****************************************************************************
*
*  Function Name:                 SCalculate
*
*    This routine calculates whether all ABC bonds in a given molecule are 
*    resonant which implys the chain/ring is resonant.
*
*  Used to be:
*
*    calculate:
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
static Boolean_t SCalculate
  (
  Array_t       *end1_p,                    /* 2-d word, one end of branch */
  Array_t       *resonant_p,                /* 2-d bit, resonant bonds */
  Xtr_t         *xtr_p                      /* Molecule to search */
  )
{
  U16_t          atom;                      /* Counter */
  U32_t          num_atoms;                 /* # atom in m'cule */
  U8_t           num_neighbors;             /* Temp */
  U8_t           neigh;                     /* Counter */
  Boolean_t      flag;                      /* Hack aversion */
  Boolean_t      all_resonant;              /* Output value */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SCalculate, end1 = %p, resonant = %p, Xtr = %p",
    end1_p, resonant_p, xtr_p));

  num_atoms = Array_Rows_Get (end1_p);
  for (atom = 0, flag = FALSE, all_resonant = TRUE; atom < num_atoms &&
       flag == FALSE; atom++)
    {
    if (Array_2d16_Get (end1_p, atom , 0) != XTR_INVALID)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0; neigh < num_neighbors && !flag; neigh++)
        {
        if (Array_2d1_Get (resonant_p, atom, neigh) == FALSE)
          {
          all_resonant = FALSE;
          flag = TRUE;
          }
        }
      }
    }

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SCalculate, flag all bonds are resonant = %hu", all_resonant));

  return all_resonant;
}

/****************************************************************************
*
*  Function Name:                 SChain_Fix
*
*    This routine sets a chain of resonant bonds according to the first bond.
*    It uses the required alternating single/double bonds that resonant bonds
*    are equivilant to.
*
*  Used to be:
*
*    fix_chain:
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
static void SChain_Fix
  (
  Xtr_t         *xtr_p,                      /* Subsetted molecule from top */
  U16_t          atom1,                      /* Atom to start at */
  U16_t          atom2,                      /* Neighbor to start at */
  Array_t       *conformation_p,             /* Conformation array */
  U8_t           bondsize                    /* Size of bond to start */
  )
{
  U16_t          neighid;                    /* Atom index of neighbor */
  U16_t          node;                       /* For cycle walking */
  U16_t          last;                       /* For cycle walking */
  U8_t           neigh;                      /* Counter */
  U8_t           nextbond;                   /* Next bond size */
  U8_t           num_bonds;                  /* Number of bond from atom */
  U8_t           num_edges_left;             /* Number of unfixed edges */
  U8_t           num_neighbors;              /* Compiler bug */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SChain_Fix, atom1 = %u, atom2 = %u, conformation = %p,"
    " bondsize = %hu", atom1, atom2, conformation_p, bondsize));

  Array_2d8_Put (conformation_p, atom1, atom2, bondsize);
  Array_2d8_Put (conformation_p, atom2, atom1, bondsize);

  node = atom2;
  last = atom1;

  if (bondsize == BOND_SINGLE)
    nextbond = BOND_DOUBLE;
  else
    nextbond = BOND_SINGLE;

  /* Fix all bonds in the cycle */

  while (Xtr_Attr_NumNeighbors_Get (xtr_p, node) == 2)
    {
    if (Xtr_Attr_NeighborId_Get (xtr_p, node, 0) == last)
      {
      last = node;
      node = Xtr_Attr_NeighborId_Get (xtr_p, node, 1);
      }
    else
      {
      last = node;
      node = Xtr_Attr_NeighborId_Get (xtr_p, node, 0);
      }

    if (Array_2d8_Get (conformation_p, last, node) != BOND_NONE)
      return;

    Array_2d8_Put (conformation_p, last, node, nextbond);
    Array_2d8_Put (conformation_p, node, last, nextbond);

    if (nextbond == BOND_SINGLE)
      nextbond = BOND_DOUBLE;
    else
      nextbond = BOND_SINGLE;
    }

  if (Xtr_Attr_Atomid_Get (xtr_p, node) != CARBON)
    return;

  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, node);
  for (neigh = 0, num_edges_left = 0, num_bonds = 0; neigh < num_neighbors;
       neigh++)
    {
    neighid = Xtr_Attr_NeighborId_Get (xtr_p, node, neigh);
    if (Array_2d8_Get (conformation_p, node, neighid) == BOND_NONE)
      num_edges_left++;
    else
      num_bonds += Array_2d8_Get (conformation_p, node, neighid);
    }

  num_bonds = 4 - num_bonds;
  if (num_bonds == num_edges_left)
    nextbond = BOND_SINGLE;
  else
    if (num_bonds == num_edges_left << 1)
      nextbond = BOND_DOUBLE;
    else
      return;

  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, node);
  for (neigh = 0; neigh < num_neighbors; neigh++)
    {
     neighid = Xtr_Attr_NeighborId_Get (xtr_p, node, neigh);
     if (Array_2d8_Get (conformation_p, neighid, node) == BOND_NONE)
       SChain_Fix (xtr_p, node, neighid, conformation_p, nextbond);
     }

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SChain_Fix, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SDuplicates_Prune
*
*    This routine trims the stacks of information eliminating duplicate
*    entries.  It does this by comparing two atom arrays for equality.
*    Two flags are used, flag 1 is true when the two are NOT duplicates,
*    ans so one should NOT be destroyed.  Flag2 is true if is destroyed.
*
*  Used to be:
*
*    prune_duplicates:
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
static void SDuplicates_Prune
  (
  AtomArray_t   *prune_p,                   /* Point to prune from */
  List_t        *list_p,                    /* List of configurations */
  Xtr_t         *xtr_p                      /* M'cule to check */
  )
{
  AtomArray_t   *front_p;                   /* For the front of the stack */
  AtomArray_t   *tail_p;                    /* For the tail of the stack */
  AtomArray_t   *prevtail_p;                /* For trailing the tail */
  U16_t          atom;                      /* Counter */
  U16_t          num_atoms;                 /* # atoms in m'cule */
  U8_t           num_neighbors, neigh;      /* Counters */
  Boolean_t      configs_equal;             /* Configs equal, ie delete */
  Boolean_t      config_deleted;            /* Config deleted, ie no need
    to advance comparator */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SDuplicates_Prune, prune = %p, config list = %p, Xtr = %p",
    prune_p, list_p, xtr_p));

  /* Loop through list up to latest round insertion point, the prune point.
     Check to see that any of the "newer" configurations, ie those after the
     prune point, are in fact duplicates.  If so, excise them from the list.
  */

  num_atoms = Xtr_NumAtoms_Get (xtr_p);

  if (prune_p == (AtomArray_t *)List_Front_Get (list_p))
    prevtail_p = NULL;
  else
    prevtail_p = prune_p;

  for (tail_p = AtmArr_Next_Get (prune_p); tail_p != NULL; )
    {
    /* Loop through entire configuration list */

    for (config_deleted = FALSE, front_p = (AtomArray_t *)List_Front_Get (
         list_p); front_p != prune_p && !config_deleted; )
      {
      for (atom = 0, configs_equal = TRUE;  atom < num_atoms &&
           configs_equal == TRUE; atom++)
        {
        num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
        for (neigh = 0; neigh < num_neighbors; neigh++)
          if (Array_2d1_Get (AtmArr_Array_Get (front_p), atom, neigh)
              != Array_2d1_Get (AtmArr_Array_Get (tail_p), atom, neigh))
            configs_equal = FALSE;
        }

      /* If the arrays are equal, then destroy the one at the head of the
         "tail" linked list.
      */

      if (configs_equal == TRUE)
        {
        List_Remove (list_p, prevtail_p);
        tail_p = AtmArr_Next_Get (prevtail_p);
        config_deleted = TRUE;
        }
      else
        front_p = AtmArr_Next_Get (front_p);
      }                                     /* End for-front_p loop */

    /* If the array wasn't destroyed it wasn't equal to any of the config
       arrays, so check the next "tail" array.
    */

    if (config_deleted == FALSE)
      {
      prune_p = prevtail_p = tail_p;
      tail_p = AtmArr_Next_Get (tail_p);
      }
    }                                       /* End for-tail_p loop */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SDuplicates_Prune, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SFRBonds_Top
*
*    This routine is the top-level of the search of the molecule for
*    resonant bonds.
*
*  Used to be:
*
*    N/A (frbond: sfrbond:):
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
static void SFRBonds_Top
  (
  ResbondsCB_t  *rbcb_p                      /* Control block */
  )
{
  Xtr_t         *xtr_tmp;                    /* Xtr trimmed to 1/2 bonds */
  Xtr_t         *rxtr_p;                     /* Copy of XTR */
  AtomArray_t   *map_p;                      /* Temporary for mapping array */
  U16_t          num_atoms;                  /* Number of atoms in molecule */
  U16_t          i, j;                       /* Counters */
  U8_t           num_neighbors;              /* Number of neighbors of atom */
  Array_t        tbonds;                     /* Temp. bonds array */

  Array_Set (RsbndCB_Output_Get (rbcb_p), FALSE);
  if (Xtr_Rings_NumRingSys_Get (RsbndCB_Xtr_Get (rbcb_p)) == 0)
    return;

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SFRBonds_Top, control block = %p", rbcb_p));

  num_atoms = Xtr_NumAtoms_Get (RsbndCB_Xtr_Get (rbcb_p));
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("RsbndCB_ABCbonds_Get(rbcb_p)", "resonantbonds{3}", RsbndCB_ABCbonds_Get (rbcb_p), num_atoms, MX_NEIGHBORS,
    BITSIZE);
#else
  Array_2d_Create (RsbndCB_ABCbonds_Get (rbcb_p), num_atoms, MX_NEIGHBORS,
    BITSIZE);
#endif
  Array_Set (RsbndCB_ABCbonds_Get (rbcb_p), FALSE);
  RsbndCB_Analyze_Put (rbcb_p, Stack_Create (STACK_ATMARR));
  RsbndCB_GenComp_Put (rbcb_p, Stack_Create (STACK_ATMARR));
  RsbndCB_Map_Put (rbcb_p, Stack_Create (STACK_ATMARR));
  RsbndCB_ParaSys_Put (rbcb_p, Stack_Create (STACK_ATMARR));
  RsbndCB_Val4Comp_Put (rbcb_p, Stack_Create (STACK_ATMARR));
  RsbndCB_Val4Ring_Put (rbcb_p, Stack_Create (STACK_ATMARR));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    if (RsbndCB_Spiro_Get (rbcb_p) == FALSE)
      IO_Put_Trace (R_XTR, "Xtr_ResonantBonds_Find called");
    else
      IO_Put_Trace (R_XTR, "Xtr_ResonantBonds_FindSpiro called");

    Xtr_Dump (RsbndCB_Xtr_Get (rbcb_p), &GTraceFile);
    });

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&tbonds", "resonantbonds{3}", &tbonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&tbonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  Array_Set (&tbonds, TRUE);

  /* Remove all bonds with a multiplicity greater than 2, they can not
     be associated with a resonant structure.
  */

  for (i = 0; i < num_atoms; i++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (RsbndCB_Xtr_Get (rbcb_p), i);
    for (j = 0; j < num_neighbors; j++)
      if (Xtr_Attr_NeighborBond_Get (RsbndCB_Xtr_Get (rbcb_p), i, j) >
          BOND_DOUBLE)
        Array_2d1_Put (&tbonds, i, j, FALSE);
    }

  map_p = AtmArr_Create (num_atoms, ATMARR_NOBONDS_WORD, FALSE);
  Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
  xtr_tmp = Xtr_CopySubset (RsbndCB_Xtr_Get (rbcb_p), &tbonds,
    AtmArr_Array_Get (map_p));

  if (RsbndCB_Spiro_Get (rbcb_p) == FALSE)
    {
    /* Copying the XTR is necessary since SGeneric_Compound_Find modifies
       the xtr which it is passed
    */

    rxtr_p = Xtr_Copy (xtr_tmp);
    xtr_tmp = SGeneric_Compound_Find (xtr_tmp, rbcb_p);
    SRemaining_Huckelness_Find (rxtr_p, rbcb_p);
    Xtr_Destroy (rxtr_p);
    }
  else
    SSpiro_Compound_Find (xtr_tmp, rbcb_p);

  Xtr_Destroy (xtr_tmp);
  Stack_Pop (RsbndCB_Map_Get (rbcb_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In ResonantBonds SFRBonds_Top");
    Array_Dump (RsbndCB_Output_Get (rbcb_p), &GTraceFile);
    });

  Stack_Destroy (RsbndCB_Analyze_Get (rbcb_p));
  Stack_Destroy (RsbndCB_GenComp_Get (rbcb_p));
  Stack_Destroy (RsbndCB_Map_Get (rbcb_p));
  Stack_Destroy (RsbndCB_ParaSys_Get (rbcb_p));
  Stack_Destroy (RsbndCB_Val4Comp_Get (rbcb_p));
  Stack_Destroy (RsbndCB_Val4Ring_Get (rbcb_p));

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&tbonds", "resonantbonds", &tbonds);
  mind_Array_Destroy ("RsbndCB_ABCbonds_Get(rbcb_p)", "resonantbonds", RsbndCB_ABCbonds_Get (rbcb_p));
#else
  Array_Destroy (&tbonds);
  Array_Destroy (RsbndCB_ABCbonds_Get (rbcb_p));
#endif

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SFRBonds_Top, status = <void>"));
  
  return;
}

/****************************************************************************
*
*  Function Name:                 SGeneric_Compound_Find
*
*    This routine looks for a subset of the input compound that has only
*    ABC bonds.  This handles generic structures, ie rings and more.
*
*    It starts by eliminating all atoms that can't have ABC bonds.  Then
*    create the ring systems information.  Check all ring systems, one at
*    a time by creating an Xtr with only the one ring system.  Check for
*    all atoms to have single and double bonds, if not, recurse to Generic
*    Compound Find.  Otherwise, look for a hetero atom, if none found, then
*    call Valence-4 Ring Find.  If a hetero atom was found, then must check
*    number of double bonds, and call Analyze Hetero-Subgoals with
*    the right type of bonds.  At end of loop for all ring systems, clean up
*    the set of module/task local stacks.
*
*  Used to be:
*
*    findr_gen_comp:
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
static Xtr_t *SGeneric_Compound_Find
  (
  Xtr_t         *xtr_p,                      /* Molecule to search */
  ResbondsCB_t  *rbcb_p                      /* Control block */
  )
{
  AtomArray_t   *bondarray_p;                /* Current bond mask */
  AtomArray_t   *map_p;                      /* Current mapping */
  Xtr_t         *rxtr_p;                     /* For 1 ring's resonant bonds */
  U16_t          num_atoms;                  /* # atoms in m'cule */
  U16_t          num_bonds;                  /* # bonds in m'cule */
  U16_t          atom, neigh;                /* Counters */
  U16_t          ringsys;                    /* Counter */
  U16_t          stunted_nodes;              /* # useless permutations */
  U8_t           num_neighbors;              /* Compiler bug */

  if (SNot_Worth_Checking (xtr_p, rbcb_p) == TRUE)
    return xtr_p;

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SGeneric_CompoundFind, Xtr = %p", xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SGeneric_Compound_Find");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  xtr_p = SImpossible_Sequences_Remove (xtr_p, &stunted_nodes, rbcb_p);
  num_atoms = Xtr_NumAtoms_Get (xtr_p);

  bondarray_p = AtmArr_Create (num_atoms, ATMARR_BONDS_BIT, FALSE);
  Stack_PushAdd (RsbndCB_GenComp_Get (rbcb_p), bondarray_p);

  /* Need to know ring system information, so must generate it */

  Xtr_Rings_Set (xtr_p);
  for (ringsys = 0; ringsys < Xtr_Rings_NumRingSys_Get (xtr_p); ringsys++)
    {
    /* Get the bonds just for this one ring system and create an XTR with only
       the ring system in it.
    */

    Ring_Bonds_Set (xtr_p, ringsys, AtmArr_Array_Get (bondarray_p));
    map_p = AtmArr_Create (num_atoms, ATMARR_NOBONDS_WORD, FALSE);
    Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
    rxtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bondarray_p), 
      AtmArr_Array_Get (map_p));

    if (SAllAtomsSingleDouble (rxtr_p) == FALSE)
      rxtr_p = SGeneric_Compound_Find (rxtr_p, rbcb_p);
    else
      {
      for (atom = 0; ((atom < Xtr_NumAtoms_Get (rxtr_p))
           && (SAtomValence_Get (rxtr_p, atom) <= 4)); atom++)
        /* Empty loop body */ ;

      /* See if a hetero atom was found, if yes, then can't look for a ring
         directly.
      */

      if (atom >= Xtr_NumAtoms_Get (rxtr_p))
        SValence4_Ring_Find (rxtr_p, rbcb_p);
      else
        {
        /* Count all double bonds and analyze all the compounds just
           considering them and the hetero atom.
        */

        num_neighbors = Xtr_Attr_NumNeighbors_Get (rxtr_p, atom);
        for (neigh = 0, num_bonds = 0; neigh < num_neighbors; neigh++)
          if (Xtr_Attr_NeighborBond_Get (rxtr_p, atom, neigh) == BOND_DOUBLE)
            num_bonds++;

        if (num_bonds > 1)
          SAnalyze_H_Subgoals (rxtr_p, atom, BOND_DOUBLE, num_bonds,
            num_neighbors, rbcb_p);
        else
          {
          /* Do the same, but for single bonds if there is only one double
             bond.
          */

          for (neigh = 0, num_bonds = 0; neigh < num_neighbors; neigh++)
            if (Xtr_Attr_NeighborBond_Get (rxtr_p, atom, neigh) == BOND_SINGLE)
              num_bonds++;

          SAnalyze_H_Subgoals (rxtr_p, atom, BOND_SINGLE, num_bonds,
            num_neighbors, rbcb_p);
          }
        }                                    /* End else-atom < Xtr_NumAtoms */
      }                                      /* End else-!SAllSingleDouble */

    Xtr_Destroy (rxtr_p);
    Stack_Pop (RsbndCB_Map_Get (rbcb_p));
    }                                        /* End for-ring systems loop */

  /* Get rid of all the mappings that have been created, but were skipped
     because they wouldn't be legal resonant molecules.  Get rid of the
     current generation "generic" compound bond mask.
  */

  for (neigh = 0; neigh < stunted_nodes; neigh++)
    Stack_Pop (RsbndCB_Map_Get (rbcb_p));

  Stack_Pop (RsbndCB_GenComp_Get (rbcb_p));

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SGeneric_Compound_Find, Xtr = %p", xtr_p));

  return xtr_p;
}

/****************************************************************************
*
*  Function Name:                 SImpossible_Sequences_Remove
*
*    This function trims the sub-goal tree of impossible sequences.  It checks
*    all atoms to make sure they have both a single and double bond, and if not
*    then the branch of atoms rooted at that atom is removed.  After each pass
*    over the whole molecule that generates a bad (improper bonds) atom, a new
*    Xtr is generated.
*
*  Used to be:
*
*    remove_impossible_sequences:
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
*    Address of modified XTR
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Xtr_t *SImpossible_Sequences_Remove
  (
  Xtr_t         *xtr_p,                      /* Molecule to check */
  U16_t         *number_p,                   /* # permutations clipped */
  ResbondsCB_t  *rbcb_p                      /* Control block */
  )
{
  Xtr_t         *curxtr_p;                   /* Current Xtr */
  AtomArray_t   *map_p;                      /* New mapping */
  U16_t          num_atoms;                  /* # atoms in m'cule */
  U16_t          atom, neigh;                /* Counters */
  U16_t          atoms_removed;              /* Counter */
  U16_t          count;                      /* Counter */
  U8_t           num_neighbors;              /* Compiler bug */
  U8_t           cur_atom_value;             /* Was this atom removed? */
  Boolean_t      bad_atoms_found;            /* Flag */
  Boolean_t      found;                      /* Flag */
  Array_t        tbonds;                     /* 2-d bit, bond mask */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SImpossible_Sequences_Remove, Xtr = %p", xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SImpossible_Sequences_Remove");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  curxtr_p = xtr_p;
  *number_p = 0;
  count = num_atoms = Xtr_NumAtoms_Get (curxtr_p);

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&tbonds", "resonantbonds{4}", &tbonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&tbonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  bad_atoms_found = TRUE;

  /* While there exist atoms without both single and double bonds
     remove them & their bonds.  If an molecule is permuted such that an atom
     ends up without both single and double bonds, then it will not be able
     to have resonant bonds so this permutation should be eliminated.
  */ 

  while (bad_atoms_found == TRUE)
    {
    bad_atoms_found = FALSE;
    Array_Set (&tbonds, TRUE);

    for (atom = 0; atom < num_atoms; atom++)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (curxtr_p, atom);
      for (neigh = 0, found = 0; ((neigh < num_neighbors) && (found != 0x3));
           neigh++)
        {
        if (Xtr_Attr_NeighborBond_Get (curxtr_p, atom, neigh) == BOND_DOUBLE)
          found |= 0x2;
        else
          if (Xtr_Attr_NeighborBond_Get (curxtr_p, atom, neigh) == BOND_SINGLE)
            found |= 0x1;
        }

      if (found != 0x3)
        {
        num_neighbors = Xtr_Attr_NumNeighbors_Get (curxtr_p, atom);
        for (neigh = 0, cur_atom_value = 0; neigh < num_neighbors; neigh++)
          if (Array_2d1_Get (&tbonds, atom, neigh))
            {
            SBranch_Remove (curxtr_p, atom, neigh, &tbonds, &atoms_removed);
            count -= atoms_removed;
            cur_atom_value = 1;
            }

        count -= cur_atom_value;
        bad_atoms_found = TRUE;
        }
      }                                      /* End for-atom */

      /* Clip out the bad atoms, and keep track of new mapping.  Count the
         number of atoms removed, but not ones caused to be no longer part
         of the molecule by their removal.
      */

      if (bad_atoms_found == TRUE)
        {
        xtr_p = curxtr_p;
        map_p = AtmArr_Create (count, ATMARR_NOBONDS_WORD, FALSE);
        Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
        (*number_p)++;
        curxtr_p = Xtr_CopySubset (xtr_p, &tbonds, AtmArr_Array_Get (map_p));
        num_atoms = Xtr_NumAtoms_Get (curxtr_p);
        Xtr_Destroy (xtr_p);
        }
    }                                        /* End while-bad_atoms_found */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&tbonds", "resonantbonds", &tbonds);
#else
  Array_Destroy (&tbonds);
#endif

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SImpossible_Sequences_Remove, return Xtr = %p, # useless maps"
    " generated = %u", curxtr_p, *number_p));

  return curxtr_p;
}

/****************************************************************************
*
*  Function Name:                 SImprove
*
*    This routine trys to find the "best" node to expand a Valence 4 ring into
*    subgoals with.
*
*  Used to be:
*
*    improve:
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
static void SImprove
  (
  S16_t          good_branch,               /* Good branch index */
  S16_t          bad_branch,                /* Bad branch index */
  U16_t          atom,                      /* Atom at one end of branches */
  Xtr_t         *xtr_p,                     /* Molecule we are checking */
  Array_t       *end1_p,                    /* 1-d word, end of branch atoms */
  Array_t       *index_p,                   /* 1-d byte, neighbor index of last
    bond in branch */
  Array_t       *other_p,                   /* 1-d word, = bondsize branches */
  Array_t       *common_node_p,             /* 1-d word, common atom of two
    branches */
  Array_t       *numnonabc_p,               /* 1-d word, # unknown abc-bonds */
  Array_t       *numabc_p,                  /* 1-d word, # known abc-bonds */
  Array_t       *up_ptr_p,                  /* 1-d word, good branch index
    for bad branch */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  U16_t          bad_end1;                  /* Atom index - end1 - bad */
  U16_t          orig_badend1;              /* Temp */
  U16_t          orig_last_neighid;         /* Temp */
  U8_t           last_neigh;                /* Last neighbor index - bad */
  U8_t           abc, nonabc;               /* Flags */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SImprove, good branch = %d, bad branch = %d, atom = %u,"
    " Xtr = %p, end1 = %p, index = %p, other = %p, common node = %p,"
    " numnonabc = %p, numabc = %p, up_ptr = %p",good_branch, bad_branch, 
    atom, xtr_p, end1_p, index_p, other_p, common_node_p, numnonabc_p, 
    numabc_p, up_ptr_p));

  bad_end1 = Array_1d16_Get (end1_p, bad_branch);
  last_neigh = Array_1d8_Get (index_p, bad_branch);
  Array_1d16_Put (up_ptr_p, bad_branch, good_branch);

  if (Array_1d16_Get (other_p, bad_branch) == INVALID_BRANCH)
    {
    Array_1d16_Put (other_p, bad_branch, -good_branch);
    Array_1d16_Put (common_node_p, bad_branch, atom);
    }

  while ((good_branch != bad_branch) && (Array_1d16_Get (up_ptr_p, good_branch)
         != INVALID_BRANCH))
    good_branch = Array_1d16_Get (up_ptr_p, good_branch);

  orig_badend1 = SMappedTo (bad_end1, rbcb_p);
  orig_last_neighid = SMappedTo (Xtr_Attr_NeighborId_Get (xtr_p, bad_end1,
    last_neigh), rbcb_p);
  if (Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_badend1,
      Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p), orig_badend1,
      orig_last_neighid)))
    {
    abc = 1;
    nonabc = 0;
    }
  else
    {
    abc = 0;
    nonabc = 1;
    }

  Array_1d16_Put (numnonabc_p, good_branch, Array_1d16_Get (numnonabc_p,
    good_branch) + Array_1d16_Get (numnonabc_p, bad_branch) + nonabc);
  Array_1d16_Put (numabc_p, good_branch, Array_1d16_Get (numabc_p,
    good_branch) + Array_1d16_Get (numabc_p, bad_branch) + abc);

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SImprove, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SMappedTo
*
*    This function returns the atom index in the original XTR of the given
*    atom index.  It uses the stack of mapping arrays to do a closure of the
*    reverse mapping.
*
*  Used to be:
*
*    mapped:
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
*    
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static U16_t SMappedTo
  (
  U16_t          target,                    /* Atom to look up */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  U16_t          temp_atom;                 /* Return value */
  AtomArray_t   *tmp;                       /* For stack traversal */

  for (tmp = (AtomArray_t *) Stack_TopAdd (RsbndCB_Map_Get (rbcb_p)), 
       temp_atom = target; tmp != NULL; tmp = AtmArr_Next_Get (tmp))
    temp_atom = Array_1d16_Get (AtmArr_Array_Get (tmp), temp_atom);

  return temp_atom;
}

/****************************************************************************
*
*  Function Name:                 SNot_Worth_Checking
*
*    This function tests a predicate to see if this node can be pruned
*    from the search, or rather summarily ignored.  The decision is based on
*    whether all ABC bonds are known to be resonant bonds.
*
*  Used to be:
*
*    noworth_checking_t:
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
static Boolean_t SNot_Worth_Checking
  (
  Xtr_t         *xtr_p,                     /* Molecule to evaluate */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  U16_t          atom, neigh;               /* Counters */
  U16_t          orig_atom;                 /* Original atom index */
  U16_t          orig_neighid;              /* Original neighbor atom index */
  U8_t           num_neighbors;             /* Compiler bug */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SNot_Worth_Checking, Xtr = %p", xtr_p));

  for (atom = 0; atom < Xtr_NumAtoms_Get (xtr_p); atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = 0; neigh < num_neighbors; neigh++)
      {
      orig_atom = SMappedTo (atom, rbcb_p);
      orig_neighid = SMappedTo (Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh),
        rbcb_p);
      if (Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
          Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p), orig_atom,
            orig_neighid)) == FALSE)
        {
        DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
          "Leaving SNot_Worth_Checking, status = FALSE"));

        return FALSE;
        }
      }
    }

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SNot_Worth_Checking, status = TRUE"));

  return TRUE;
}

/****************************************************************************
*
*  Function Name:                 SParallel
*
*    This function determines if two branches are linked in parallel.
*    If they share the same atoms at their ends then they are in parallel.
*
*  Used to be:
*
*    parallel:
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
*    TRUE  - they are parallel
*    FALSE - they aren't
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SParallel
  (
  U16_t          branch1,                   /* One branch index */
  U16_t          branch2,                   /* The other branch index */
  Array_t       *end1_p,                    /* 1-d word, one end of branch */
  Array_t       *end2_p                     /* 1-d word, other end of branch */
  )
{
  if (Array_1d16_Get (end1_p, branch1) == Array_1d16_Get (end1_p, branch2))
    return (Array_1d16_Get (end2_p, branch1) == Array_1d16_Get (end2_p,
      branch2));
  else
    if (Array_1d16_Get (end1_p, branch1) == Array_1d16_Get (end2_p, branch2))
      return (Array_1d16_Get (end2_p, branch1) == Array_1d16_Get (end1_p,
        branch2))
      ;
  else
    return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SParallel_System_Find
*
*    This routine finds ABC bonds in parallel components of Valence 4 Rings.
*    Each possible combination of branch types (one, two, even) are handled
*    differently.  See Boivie's thesis for details.
*
*  Used to be:
*
*    findr_parallel_sys:
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
static void SParallel_System_Find
  (
  Xtr_t         *xtr_p,                     /* Molecule to deal with */
  U16_t          branch1,                   /* One branch index to check */
  U16_t          branch2,                   /* Other branch index */
  Array_t       *end1_p,                    /* 1-d word, one end of branch */
  Array_t       *index_p,                   /* 1-d byte, neigh index of branch
    bond */
  Array_t       *type_p,                    /* 1-d byte, type of branch */
  Array_t       *length_p,                  /* 1-d word, length of branch */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  Xtr_t         *txtr_p;                    /* For recursion */
  AtomArray_t   *bonds_p;                   /* Modified bond mask */
  AtomArray_t   *map_p;                     /* Current mapping */
  U16_t          atoms_removed;             /* From SRemove_Branch */
  U16_t          orig_atom, orig_neigh_idx; /* Original information */
  U16_t          type;                      /* What sort of system */
  U16_t          branch;                    /* Counter */
  U16_t          num_atoms;                 /* # atoms in m'cule */
  Boolean_t      save1, save2;              /* Temps */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SParallel_System_Find, Xtr = %p, branch1 = %u, branch2 = %u,"
    " end1 = %p, index = %p, type = %p, length = %p", xtr_p, branch1, branch2,
    end1_p, index_p, type_p, length_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SParallel_System_Find");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  type = 1;
  num_atoms = Xtr_NumAtoms_Get (xtr_p);

  bonds_p = AtmArr_Create (num_atoms, ATMARR_BONDS_BIT, TRUE);
  Stack_PushAdd (RsbndCB_ParaSys_Get (rbcb_p), bonds_p);

  /* Want to execute this loop exactly twice */

  for (branch = branch1; branch <= branch2; branch += branch2 - branch1)
    {
    if (Array_1d8_Get (type_p, branch) == ONEBRANCH)
      type *= 2;
    else
      if (Array_1d8_Get (type_p, branch) == EVENBRANCH)
        type *= 3;
    else
      if (Array_1d8_Get (type_p, branch) == TWOBRANCH)
        type *= 5;
   }

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_MAJOR, (outbuf,
    "In SParallel_System_Find, type = %hu", type));

  switch (type)
    {
    case 4:                                 /* 2 1-branches in parallel */

      orig_atom = SMappedTo (Array_1d16_Get (end1_p, branch2), rbcb_p);
      type = SMappedTo (Xtr_Attr_NeighborId_Get (xtr_p, Array_1d16_Get (
        end1_p, branch2), Array_1d8_Get (index_p, branch2)), rbcb_p);
      orig_neigh_idx = Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p),
        orig_atom, type);
      save1 = Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
        orig_neigh_idx);
      save2 = Array_2d1_Get (RsbndCB_Output_Get (rbcb_p), orig_atom,
        orig_neigh_idx);
      Array_2d1_Put (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom, orig_neigh_idx,
        FALSE);
      Array_2d1_Put (RsbndCB_Output_Get (rbcb_p), orig_atom, orig_neigh_idx,
        FALSE);

      SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch1), Array_1d8_Get (
        index_p, branch1), AtmArr_Array_Get (bonds_p), &atoms_removed);
      map_p = AtmArr_Create (num_atoms - atoms_removed, ATMARR_NOBONDS_WORD,
        FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      SValence4_Ring_Find (txtr_p, rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));

      if (Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
          orig_neigh_idx))
        {
        SBonds_Invert (AtmArr_Array_Get (bonds_p), AtmArr_NumAtoms_Get (
          bonds_p));
        map_p = AtmArr_Create (atoms_removed + 2, ATMARR_NOBONDS_WORD, FALSE);
        Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
        txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
          AtmArr_Array_Get (map_p));
        SBonds_Mark (txtr_p, ((Array_1d16_Get (length_p, branch1) ==
          Array_1d16_Get (length_p, branch2)) && Array_2d1_Get (
          RsbndCB_Output_Get (rbcb_p), orig_atom, orig_neigh_idx)) ||
          ((Array_1d16_Get (length_p, branch1) != Array_1d16_Get (length_p,
          branch2)) && !Array_2d1_Get (RsbndCB_Output_Get (rbcb_p), orig_atom,
          orig_neigh_idx)), rbcb_p);
        Xtr_Destroy (txtr_p);
        Stack_Pop (RsbndCB_Map_Get (rbcb_p));
        }

      Array_2d1_Put (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom, orig_neigh_idx,
        (save1 || Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
        orig_neigh_idx)));
      Array_2d1_Put (RsbndCB_Output_Get (rbcb_p), orig_atom, orig_neigh_idx,
        (save2 || Array_2d1_Get (RsbndCB_Output_Get (rbcb_p), orig_atom,
        orig_neigh_idx)));
      break;

    case 10:                                /* 1-branch & 2-branch in || */

      if (Array_1d8_Get (type_p, branch1) == ONEBRANCH)
        SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch1),
          Array_1d8_Get (index_p, branch1), AtmArr_Array_Get (bonds_p),
          &atoms_removed);
      else
        SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch2),
          Array_1d8_Get (index_p, branch2), AtmArr_Array_Get (bonds_p),
          &atoms_removed);

      map_p = AtmArr_Create (num_atoms - atoms_removed, ATMARR_NOBONDS_WORD,
        FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      SValence4_Ring_Find (txtr_p, rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));

      SBonds_Invert (AtmArr_Array_Get (bonds_p), AtmArr_NumAtoms_Get (
        bonds_p));
      map_p = AtmArr_Create (atoms_removed + 2, ATMARR_NOBONDS_WORD, FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      SBonds_Mark (txtr_p, Array_1d16_Get (length_p, branch1) ==
        Array_1d16_Get (length_p, branch2), rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));

      Array_Set (AtmArr_Array_Get (bonds_p), TRUE);
      if (Array_1d8_Get (type_p, branch1) == ONEBRANCH)
        SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch2),
          Array_1d8_Get (index_p, branch2), AtmArr_Array_Get (bonds_p),
          &atoms_removed);
      else
        SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch1),
          Array_1d8_Get (index_p, branch1), AtmArr_Array_Get (bonds_p),
          &atoms_removed);

      SBonds_Invert (AtmArr_Array_Get (bonds_p), AtmArr_NumAtoms_Get (
        bonds_p));
      map_p = AtmArr_Create (atoms_removed + 2, ATMARR_NOBONDS_WORD, FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p =Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      SBonds_Mark (txtr_p, Array_1d16_Get (length_p, branch1) ==
        Array_1d16_Get (length_p, branch2), rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));
      break;

    case 6:                                 /* 1-branch & e-branch in || */

      if (Array_1d8_Get (type_p, branch1) == ONEBRANCH)
        SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch1),
          Array_1d8_Get (index_p, branch1), AtmArr_Array_Get (bonds_p),
          &atoms_removed);
      else
        SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch2),
          Array_1d8_Get (index_p, branch2), AtmArr_Array_Get (bonds_p),
          &atoms_removed);
      map_p = AtmArr_Create (num_atoms - atoms_removed, ATMARR_NOBONDS_WORD,
        FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      SValence4_Ring_Find (txtr_p, rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));
      break;

    case 9:                                 /* 2 e-branches in parallel */

      SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch1), Array_1d8_Get (
        index_p, branch1), AtmArr_Array_Get (bonds_p), &atoms_removed);
      type = atoms_removed;
      SBranch_Remove (xtr_p, Array_1d16_Get (end1_p, branch2), Array_1d8_Get (
        index_p, branch2), AtmArr_Array_Get (bonds_p), &atoms_removed);
      map_p = AtmArr_Create (num_atoms - atoms_removed - type,
        ATMARR_NOBONDS_WORD, FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      txtr_p = SValence4_Compound_Find (txtr_p, rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));
      SBonds_Invert (AtmArr_Array_Get (bonds_p), AtmArr_NumAtoms_Get (
        bonds_p));
      map_p = AtmArr_Create (atoms_removed + type + 2,
        ATMARR_NOBONDS_WORD, FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      SBonds_Mark (txtr_p, Array_1d16_Get (length_p, branch1) !=
        Array_1d16_Get (length_p, branch2), rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));
      break;

    default:

      ASSERT (FALSE,
        {
        IO_Exit_Error (R_XTR, X_SYNERR,
          "SParallel_Systems_Find: unexpected branches in parallel");
        });
      break;
    }                                       /* End of switch-type */

  Stack_Pop (RsbndCB_ParaSys_Get (rbcb_p));

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SParallel_System_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPaths_Find_Configs_Add
*
*    This routine looks for ABCycles of size 4n+2 (Huckel's Rule) in sets of
*    known ABC bonds.  It deals with shifting the bonds to determine new
*    conformations.
*
*  Used to be:
*
*    find_paths_and_add_configs:
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
static void SPaths_Find_Configs_Add
  (
  List_t        *list_p,                    /* List of configurations */
  AtomArray_t   *cur_config_p,              /* Current config to expand */
  Array_t       *end1_p,                    /* 2-d word, one end of branch */
  Array_t       *resonant_p,                /* 2-d bit, resonance flags */
  Boolean_t     *all_resonant_p,            /* Flag for resonance */
  Array_t       *end2_p,                    /* 2-d byte, neigh. index of end */
  Array_t       *length_p,                  /* 2-d word, length of branch */
  Xtr_t         *xtr_p                      /* Molecule to check */
  )
{
  AtomArray_t   *prune_p;                   /* Newest insertion point */
  U32_t          num_atoms;
  U16_t          atom;                      /* Counters */
  U16_t          num_neighbors;             /* Temporary */
  U8_t           neigh;                     /* Counter */
  Boolean_t      flag;                      /* Hack avoidance */
  Array_t        visited;                   /* 2-d bit, bond mask */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SPaths_Find_Configs_Add, list = %p, current config = %p, end1 =\
 %p, resonant = %p, end2 = %p, length = %p, Xtr = %p",
    list_p, cur_config_p, end1_p, resonant_p, end2_p, length_p, xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SPaths_Find_Configs_Add");
    Array_Dump (AtmArr_Array_Get (cur_config_p), &GTraceFile);
    });

  num_atoms = Array_Rows_Get (length_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&visited", "resonantbonds{5}", &visited, num_atoms, BITSIZE);
#else
  Array_1d_Create (&visited, num_atoms, BITSIZE);
#endif
  Array_Set (&visited, FALSE);

  for (atom = 0, flag = FALSE; atom < num_atoms && flag == FALSE; atom++)
    {
    if (Array_2d16_Get (end1_p, atom, 0) != XTR_INVALID)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0; neigh < num_neighbors && !flag; neigh++)
        {
        prune_p = (AtomArray_t *)List_Tail_Get (list_p);
        if (Array_2d1_Get (AtmArr_Array_Get (cur_config_p), atom, neigh) ==
            TRUE)
          if (SPursue (atom, neigh, 0, list_p, cur_config_p, end1_p, end2_p,
              length_p, resonant_p, atom, &visited, xtr_p) == TRUE)
            {
            *all_resonant_p = SCalculate (end1_p, resonant_p, xtr_p);
            if (*all_resonant_p == TRUE)
              flag = TRUE;
            else
              SDuplicates_Prune (prune_p, list_p, xtr_p);
            }
        }                                   /* End for-neigh loop */
      }                                     /* End if-end1_p == 0 block */

    Array_1d1_Put (&visited, atom, TRUE);
    }                                       /* End for-atom loop */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&visited", "resonantbonds", &visited);
#else
  Array_Destroy (&visited);
#endif

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SPaths_Find_Configs_Add, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPursue
*
*    This routine uses a depth-first search to find ABCycles of size 4n+2,
*    it helps out SPaths_Find_Configs_Add.
*
*  Used to be:
*
*    pursue:
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
*    TRUE  - haven't reached end of chain yet
*    FALSE - have reached end of chain
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SPursue
  (
  U16_t          atom,                      /* Atom to check out */
  U8_t           neigh,                     /* Neighbor index to check out */
  U16_t          length_in,                 /* Length to start looking for */
  List_t        *list_p,                    /* List of configurations */
  AtomArray_t   *cur_config_p,              /* Current configuration */
  Array_t       *end1_p,                    /* 2-d word, one end of branch */
  Array_t       *end2_p,                    /* 2-d byte, other end neigh.
    index */
  Array_t       *length_p,                  /* 1-d word, length of branch */
  Array_t       *resonant_p,                /* 2-d bit, resonant bond mask */
  U16_t          goal_atom,                 /* Recursion protection */
  Array_t       *visited_p,                 /* 1-d bit, atoms seen already */
  Xtr_t         *xtr_p                      /* Molecule to search */
  )
{
  AtomArray_t   *new_config_p;              /* New configuration */
  U16_t          length_out;                /* New length to look at */
  U16_t          neighid;                   /* Temporary */
  U8_t           num_neighbors;             /* # neighbors of atom */
  U8_t           other_end_idx;             /* Other end neighbor index */
  U8_t           neigh_idx;                 /* Counter */
  Boolean_t      found, bond;               /* Flags */

  neighid = Array_2d16_Get (end1_p, atom, neigh);
  if (Array_1d1_Get (visited_p, neighid))
    return FALSE;

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SPursue, atom = %u, neigh idx = %hu, length_in = %u,"
    " list = %p, current config = %p, end #1 = %p, end #2 = %p,"
    " length = %p, resonant = %p, goal atom = %u, visited = %p, Xtr = %p",
    atom, neigh, length_in, list_p, cur_config_p, end1_p, end2_p, length_p,
    resonant_p, goal_atom, visited_p, xtr_p));

  other_end_idx = Array_2d8_Get (end2_p, atom, neigh);
  length_out = length_in + Array_2d16_Get (length_p, atom, neigh);

  /* Check for Huckel length rings */

  if ((neighid == goal_atom) && (length_out % 4) == 2)
    {
    new_config_p = AtmArr_Copy (cur_config_p);
    List_InsertAdd (list_p, List_Tail_Get (list_p), new_config_p);

    Array_2d1_Put (AtmArr_Array_Get (new_config_p), atom, neigh,
      !Array_2d1_Get (AtmArr_Array_Get (new_config_p), atom, neigh));
    Array_2d1_Put (AtmArr_Array_Get (new_config_p), neighid, other_end_idx,
      !Array_2d1_Get (AtmArr_Array_Get (new_config_p), neighid,
      other_end_idx));
    Array_2d1_Put (resonant_p, atom, neigh, TRUE);
    Array_2d1_Put (resonant_p, neighid, other_end_idx, TRUE);

    DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
      "Leaving SPursue, status = FALSE"));

    return TRUE;
    }
  else
    new_config_p = (AtomArray_t *)List_Tail_Get (list_p);

  if (neighid == goal_atom)
    {
    DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
      "Leaving SPursue, status = FALSE"));

    return FALSE;
    }

  bond = Array_2d1_Get (AtmArr_Array_Get (cur_config_p), neighid,
    other_end_idx);
  Array_1d1_Put (visited_p, neighid, TRUE);
  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, neighid);
  for (neigh_idx = 0, found = FALSE; neigh_idx < num_neighbors; neigh_idx++)
    {
    if (Array_2d1_Get (AtmArr_Array_Get (cur_config_p), neighid,
        neigh_idx) != bond)
      if (SPursue (neighid, neigh_idx, length_out, list_p, cur_config_p,
          end1_p, end2_p, length_p, resonant_p, goal_atom, visited_p, xtr_p))
        found = TRUE;
    }

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SPursue, status = %hu", found));

  /* Mark atom as no longer seen, and invert the configuration bonds */

  Array_1d1_Put (visited_p, neighid, FALSE);
  if (found == TRUE)
    {
    for (new_config_p = AtmArr_Next_Get (new_config_p); new_config_p != NULL;
         new_config_p = AtmArr_Next_Get (new_config_p))
      {
      Array_2d1_Put (AtmArr_Array_Get (new_config_p), atom, neigh,
        !Array_2d1_Get (AtmArr_Array_Get (new_config_p), atom, neigh));
      Array_2d1_Put (AtmArr_Array_Get (new_config_p), neighid, other_end_idx,
        !Array_2d1_Get (AtmArr_Array_Get (new_config_p), neighid,
        other_end_idx));
      }

    Array_2d1_Put (resonant_p, atom, neigh, TRUE);
    Array_2d1_Put (resonant_p, neighid, other_end_idx, TRUE);
    return TRUE;
    }
  else
    return FALSE;
}

/****************************************************************************
*
*  Function Name:                 SRelation
*
*    This routine determines how two branches are related.  This is to help
*    determine which branch should be removed in generating the first sub-goal
*    of a Valence-4 Ring.
*
*  Used to be:
*
*    relation:
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
static void SRelation
  (
  S16_t          branch1,                   /* Branch index #1 */
  S16_t          branch2,                   /* Branch index #2 */
  U16_t          atom,                      /* End atom id */
  Xtr_t         *xtr_p,                     /* Molecule we are checking */
  Array_t       *end1_p,                    /* 1-d word, one end of branch */
  Array_t       *index_p,                   /* 1-d byte, neighbor index of last
    bond in branch */
  Array_t       *other_p,                   /* 1-d word, = bondsize branches */
  Array_t       *common_node_p,             /* 1-d word, common atom between
    branches */
  Array_t       *numnonabc_p,               /* 1-d word, # unknown abc-bonds */
  Array_t       *numabc_p,                  /* 1-d word, # known abc-bonds */
  Array_t       *up_ptr_p,                  /* 1-d word, good branch above
    bad branch */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  U8_t           bond1, bond2;              /* Temps */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SRelation, branch1 = %d, branch2 = %d, atom = %u, Xtr = %p,"
    " end1 = %p, index = %p, other = %p, common_node = %p, numnonabc = %p,"
    " numabc = %p, up_ptr = %p", branch1, branch2, atom, xtr_p, end1_p, 
    index_p, other_p, common_node_p, numnonabc_p, numabc_p, up_ptr_p));

  /* Swap bond sizes, so can reverse bond size due to cycle rotation */

  bond1 = Xtr_Attr_NeighborBond_Get (xtr_p, Array_1d16_Get (end1_p, branch1),
    Array_1d8_Get (index_p, branch1));
  if (Array_1d16_Get (end1_p, branch1) != atom)
    if (bond1 == BOND_SINGLE)
      bond1 = BOND_DOUBLE;
    else
      bond1 = BOND_SINGLE;

  bond2 = Xtr_Attr_NeighborBond_Get (xtr_p, Array_1d16_Get (end1_p, branch2),
    Array_1d8_Get (index_p, branch2));
  if (Array_1d16_Get (end1_p, branch2) != atom)
    {
    if (bond2 == BOND_SINGLE)
      bond2 = BOND_DOUBLE;
    else bond2 = BOND_SINGLE;
    }

  if (bond1 > bond2)
    SImprove (branch1, branch2, atom, xtr_p, end1_p, index_p, other_p,
      common_node_p, numnonabc_p, numabc_p, up_ptr_p, rbcb_p);
  else if (bond2 > bond1)
    SImprove (branch2, branch1, atom, xtr_p, end1_p, index_p, other_p,
      common_node_p, numnonabc_p, numabc_p, up_ptr_p, rbcb_p);
  else
    {
    Array_1d16_Put (other_p, branch1, branch2);
    Array_1d16_Put (other_p, branch2, branch1);
    }

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SRelation, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SRemaining_Huckelness_Find
*
*    This routine looks for Huckel rings after all ABC bonds have been found.
*    In finding ABC bonds, some of the resonance may have been found, but
*    not necessarily all of it.
*
*  Used to be:
*
*    find_remaining_huckelness:
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
static void SRemaining_Huckelness_Find
  (
  Xtr_t         *xtr_p,                     /* Molecule to search */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  Xtr_t         *txtr_p;                    /* Temp. XTR copy */
  AtomArray_t   *map_p;                     /* Temp. mapping array */
  U16_t          num_atoms;                 /* # atoms in m'cule */
  U16_t          num_neighbors;             /* # neighbors of atom */
  U16_t          atom, ringsys;             /* Counters */
  U16_t          orig_atom;                 /* Original atom id */
  U16_t          unchecked_atoms;           /* # atoms not yet checked fully */
  U16_t          orig_neighid;              /* Original neighbor id */
  U8_t           orig_neigh_idx;            /* Original neighbor index */
  U8_t           neigh;                     /* Counter */
  Boolean_t      all_resonant, found;       /* Flags */
  Array_t        bonds;                     /* For bond manipulation */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SRemaining_Huckelness_Find, Xtr = %p", xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR,
      "In SRemaining_Huckelness_Find, Xtr, ABC bonds, Output");
    Xtr_Dump (xtr_p, &GTraceFile);
    Array_Dump (RsbndCB_ABCbonds_Get (rbcb_p), &GTraceFile);
    Array_Dump (RsbndCB_Output_Get (rbcb_p), &GTraceFile);
    });

  num_atoms = Xtr_NumAtoms_Get (xtr_p);

  /* Check to see if there are any resonant bonds that have been found, but
     not yet marked in the output.
  */

  for (atom = 0, found = FALSE; atom < num_atoms && found == FALSE; atom++)
    {
    orig_atom = SMappedTo (atom, rbcb_p);
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = 0; neigh < num_neighbors && !found; neigh++)
      {
      orig_neighid = SMappedTo (Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh),
        rbcb_p);
      orig_neigh_idx = Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p),
        orig_atom, orig_neighid);
      if (Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
          orig_neigh_idx) != Array_2d1_Get (RsbndCB_Output_Get (rbcb_p),
          orig_atom, orig_neigh_idx))
        found = TRUE;
      }
    }

  /* None found, so all known resonance has been found. */

  if (found == FALSE)
    {
    DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
      "Leaving SRemaining_Huckelness_Find, status = <void>"));

    return;
    }

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "resonantbonds{6}", &bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  Xtr_Rings_Set (xtr_p);
  for (ringsys = 0; ringsys < Xtr_Rings_NumRingSys_Get (xtr_p); ringsys++)
    {
    Ring_Bonds_Set (xtr_p, ringsys, &bonds);
    for (atom = 0, unchecked_atoms = 0, all_resonant = TRUE; atom < num_atoms;
         atom++)
      {
      orig_atom = SMappedTo (atom, rbcb_p);
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0, found = FALSE; neigh < num_neighbors; neigh++)
        if (Array_2d1_Get (&bonds, atom, neigh) == TRUE)
          {
          orig_neighid = SMappedTo  (Xtr_Attr_NeighborId_Get (xtr_p, atom,
            neigh), rbcb_p);
          orig_neigh_idx = Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (
            rbcb_p), orig_atom, orig_neighid);

          if (Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
              orig_neigh_idx) != Array_2d1_Get (RsbndCB_Output_Get (rbcb_p),
              orig_atom, orig_neigh_idx))
            all_resonant = FALSE;

          if (Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), orig_atom,
              orig_neigh_idx) == FALSE)
            {
            Array_2d1_Put (&bonds, atom, neigh, FALSE);
            }
          else
            found = TRUE;
          }

      if (found == TRUE)
        unchecked_atoms++;
      }                                     /* End of for-atom loop */

    if (all_resonant == FALSE)
      {
      map_p = AtmArr_Create (unchecked_atoms, ATMARR_NOBONDS_WORD, FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset  (xtr_p, &bonds, AtmArr_Array_Get (map_p));
      SRingSystem_Huckelness_Find (txtr_p, rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));
      }
    }                                       /* End for-ringsys loop */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bonds", "resonantbonds", &bonds);
#else
  Array_Destroy (&bonds);
#endif

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SRemaining_Huckelness_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SRingSystem_Huckelness_Find
*
*    This function looks for Huckelness in ring systems of ABC bonds.
*
*  Used to be:
*
*    find_huck_in_r_sys:
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
static void SRingSystem_Huckelness_Find
  (
  Xtr_t         *xtr_p,                     /* Molecule to search */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  Xtr_t         *txtr_p;                    /* Xtr for recursion */
  AtomArray_t   *config_p;                  /* Current configuration */
  AtomArray_t   *map_p;                     /* Current mapping */
  List_t        *list_p;                    /* List for configurations */
  U16_t          atom;                      /* Counter */
  U16_t          num_atoms;                 /* # atoms in m'cule */
  U16_t          num_neighbors;             /* # neighbors of cur. atom */
  U16_t          atoms_removed;             /* # atoms removed per call */
  U16_t          removed_sum;               /* Sum of repeated calls */
  U16_t          neighid, last, temp;       /* Temporaries */
  U16_t          branch_len;                /* Branch being looked at */
  U8_t           neigh;                     /* Counter */
  U8_t           neigh_idx;                 /* Temporary */
  Boolean_t      all_resonant;              /* Flag */
  Boolean_t      large_degree_found;        /* Flag for node with 3 neigh's */
  Array_t        bonds;                     /* For bond manipulation */
  Array_t        end1;                      /* 2-d word, one end of branch */
  Array_t        length;                    /* 2-d word, length of branch by
    atom and neighbor indices */
  Array_t        end2;                      /* 2-d byte, other end of branch */
  Array_t        resonance;                 /* 2-d bit, resonant bond flags */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SRingSystem_Huckelness_Find, Xtr = %p", xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR,
      "In SRingSystem_Huckelness_Find, Xtr, ABC bonds, Output");
    Xtr_Dump (xtr_p, &GTraceFile);
    Array_Dump (RsbndCB_ABCbonds_Get (rbcb_p), &GTraceFile);
    Array_Dump (RsbndCB_Output_Get (rbcb_p), &GTraceFile);
    });

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&end1", "resonantbonds{7}", &end1, num_atoms, MX_NEIGHBORS, WORDSIZE);
  mind_Array_2d_Create ("&end2", "resonantbonds{7}", &end2, num_atoms, MX_NEIGHBORS, BYTESIZE);
  mind_Array_2d_Create ("&length", "resonantbonds{7}", &length, num_atoms, MX_NEIGHBORS, WORDSIZE);
  mind_Array_2d_Create ("&resonance", "resonantbonds{7}", &resonance, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&end1, num_atoms, MX_NEIGHBORS, WORDSIZE);
  Array_2d_Create (&end2, num_atoms, MX_NEIGHBORS, BYTESIZE);
  Array_2d_Create (&length, num_atoms, MX_NEIGHBORS, WORDSIZE);
  Array_2d_Create (&resonance, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  Array_Set (&resonance, FALSE);
  Array_Set (&end1, XTR_INVALID);

  /* Allocate configuration */

  config_p = AtmArr_Create (num_atoms, ATMARR_BONDS_BIT, FALSE);
  list_p = List_Create (LIST_ATMARR);
  List_InsertAdd (list_p, NULL, config_p);

  for (atom = 0, large_degree_found = FALSE; atom < num_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    if (num_neighbors > 2)
      {
      large_degree_found = TRUE;
      for (neigh = 0; neigh < num_neighbors; neigh++)
        {
        if (Array_2d16_Get (&end1, atom, neigh) == XTR_INVALID)
          {
          neighid = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);
          last = atom;
          for (branch_len = 1; Xtr_Attr_NumNeighbors_Get (xtr_p, neighid) == 2;
               branch_len++)
            {
            temp = neighid;
            if (Xtr_Attr_NeighborId_Get (xtr_p, neighid, 0) == last)
              neighid = Xtr_Attr_NeighborId_Get (xtr_p, neighid, 1);
            else
              neighid = Xtr_Attr_NeighborId_Get (xtr_p, neighid, 0);
            last = temp;
            }

          neigh_idx = Xtr_Attr_NeighborIndex_Find (xtr_p, neighid, last);
          Array_2d16_Put (&end1, atom, neigh, neighid);
          Array_2d16_Put (&end1, neighid, neigh_idx, atom);
          Array_2d8_Put (&end2, atom, neigh, neigh_idx);
          Array_2d8_Put (&end2, neighid, neigh_idx, neigh);
          Array_2d16_Put (&length, atom, neigh, branch_len);
          Array_2d16_Put (&length, neighid, neigh_idx, branch_len);
          Array_2d1_Put (AtmArr_Array_Get (config_p), atom, neigh,
            (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) == BOND_DOUBLE));
          Array_2d1_Put (AtmArr_Array_Get (config_p), neighid, neigh_idx,
            (Xtr_Attr_NeighborBond_Get  (xtr_p, neighid, neigh_idx)
            == BOND_DOUBLE));
          }                                 /* If end1 [] == 0 block */
        }                                   /* End for-neigh loop */
      }                                     /* End if num_neighbors > 2 */
    }                                       /* End for-atom loop */

  /* If there is an expansion point, then this should be simplified before
     looking for Ring System Huckelness.
  */

  if (large_degree_found == FALSE)
    {  /* changed AtmArr_Destroy (config_p) to List_Destroy (list_p) (DK) */
    List_Destroy (list_p);   
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&end1", "resonantbonds", &end1);
    mind_Array_Destroy ("&end2", "resonantbonds", &end2);
    mind_Array_Destroy ("&length", "resonantbonds", &length);
    mind_Array_Destroy ("&resonance", "resonantbonds", &resonance);
#else
    Array_Destroy (&end1);
    Array_Destroy (&end2);
    Array_Destroy (&length);
    Array_Destroy (&resonance);
#endif

    DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
      "Leaving SRingSystem_Huckelness_Find, status = <void>"));

    return;
    }

  for (all_resonant = FALSE; config_p != NULL && !all_resonant;
       config_p = AtmArr_Next_Get (config_p))
    SPaths_Find_Configs_Add (list_p, config_p, &end1, &resonance,
      &all_resonant, &end2, &length, xtr_p);

  /* Free configuration list */

  List_Destroy (list_p);

  /* Mark resonant bond array for resonant branches */

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "resonantbonds{7}", &bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  Array_Set (&bonds, TRUE);

  for (atom = 0, removed_sum = 0; atom < num_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    if (num_neighbors > 2)
      {
      for (neigh = 0; neigh < num_neighbors; neigh++)
        if (Array_2d1_Get (&resonance, atom, neigh) == FALSE)
          {
          Array_2d1_Put (&resonance, Array_2d16_Get (&end1, atom, neigh),
            Array_2d8_Get (&end2, atom, neigh), TRUE);
          SBranch_Remove (xtr_p, atom, neigh, &bonds, &atoms_removed);
          removed_sum += atoms_removed;
          }
      }
    }

  map_p = AtmArr_Create (num_atoms - removed_sum, ATMARR_NOBONDS_WORD, FALSE);
  Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
  txtr_p = Xtr_CopySubset (xtr_p, &bonds, AtmArr_Array_Get (map_p));
  SBonds_Mark (txtr_p, TRUE, rbcb_p);
  Xtr_Destroy (txtr_p);
  Stack_Pop (RsbndCB_Map_Get (rbcb_p));

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bonds", "resonantbonds", &bonds);
  mind_Array_Destroy ("&end1", "resonantbonds", &end1);
  mind_Array_Destroy ("&length", "resonantbonds", &length);
  mind_Array_Destroy ("&end2", "resonantbonds", &end2);
  mind_Array_Destroy ("&resonance", "resonantbonds", &resonance);
#else
  Array_Destroy (&bonds);
  Array_Destroy (&end1);
  Array_Destroy (&length);
  Array_Destroy (&end2);
  Array_Destroy (&resonance);
#endif

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SRingSystem_Huckelness_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SSpiro_Atom
*
*    This function tests to see if an atom is a Spiro atom.
*
*  Used to be:
*
*    spiro_atom:
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
*    True - is a Spiro atom
*    False - is not
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SSpiro_Atom
  (
  Xtr_t         *xtr_p,                     /* Molecule to search */
  U16_t          atom,                      /* Atom to test */
  U16_t          num_atoms                  /* # atoms to consider */
  )
{
  U16_t          num_ring_systems;          /* # rings sys. in m'cule */
  U16_t          num_neighbors;             /* # neighbors of atom */
  U16_t          ringsys;                   /* Counter */
  U16_t          single_bonds;              /* Counter */
  U16_t          double_bonds;              /* Counter */
  U8_t           neigh;                     /* Counter */
  Boolean_t      spiro_bits;                /* Flag */
  Array_t        bonds;                     /* For bond manipulation */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SSpiro_Atom, Xtr = %p, atom = %u, # atoms = %u",
    xtr_p, atom, num_atoms));

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("&bonds", "resonantbonds{8}", &bonds, num_atoms, MX_NEIGHBORS, BYTESIZE);
#else
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BYTESIZE);
#endif
  num_ring_systems = Xtr_Rings_NumRingSys_Get (xtr_p);
  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
  single_bonds = 0;
  double_bonds = 0;

  for (ringsys = 0, spiro_bits = 0;
      ((ringsys < num_ring_systems) && (spiro_bits != 0x3)); ringsys++)
    {
    if (Ring_AtomInSpecific (xtr_p, ringsys, atom) == TRUE)
      {
      Ring_Bonds_Set (xtr_p, ringsys, &bonds);
      for (neigh = 0; neigh < num_neighbors; neigh++)
        {
        if (Array_2d8_Get (&bonds, atom, neigh))
          if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh)
              == BOND_SINGLE)
            single_bonds++;
          else
            double_bonds++;
        }

      if (single_bonds > 1)
        spiro_bits |= 0x1;
      if (double_bonds > 1)
        spiro_bits |= 0x2;
      }
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&bonds", "resonantbonds", &bonds);
#else
  Array_Destroy (&bonds);
#endif

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SSpiro_Atom, status = %hu", spiro_bits == 0x3));

  return spiro_bits == 0x3;
}

/****************************************************************************
*
*  Function Name:                 SSpiro_Compound_Find
*
*    This routine finds ABC bonds in spiro rings.
*
*  Used to be:
*
*    findr_spiro_comp:
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
static void SSpiro_Compound_Find
  (
  Xtr_t         *xtr_p,                     /* Molecule to search */
  ResbondsCB_t  *rbcb_p                     /* Control block */
  )
{
  Xtr_t         *txtr_p;                    /* Temp. XTR copy */
  Xtr_t         *rxtr_p;                    /* Temp. XTR copy */
  AtomArray_t   *map_p;                     /* Temp. mapping array */
  U16_t          num_atoms;                 /* # atoms in m'cule */
  U16_t          num_psuedo_atoms;          /* # additional atoms */
  U16_t          new_neigh_id;              /* Temporary */
  U16_t          next_pseudo_atom;          /* Counter */
  U16_t          next_index;                /* Counter */
  U16_t          atom, neigh;               /* Counters */
  U16_t          num_neighbors;             /* For manip. of m'cule */
  U16_t          new_neigh_cnt;             /* # new neighbors */
  U16_t          double_bond_cnt;           /* # double bonds for an atom */
  U16_t          neigh_id;                  /* Temporary */
  U16_t          bondsize;                  /* Temporary */
  U16_t          k, l;                      /* Counters */
  U16_t          last_double_bond;          /* Counter */
  Array_t        num_2bonds;                /* Array to count double bonds */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SSpiro_Compound_Find, Xtr = %p", xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SSpiro_Compound_Find");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  if (Xtr_Rings_NumRingSys_Get (xtr_p) == 0)
    return;

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
  num_psuedo_atoms = 0;
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&num_2bonds", "resonantbonds{9}", &num_2bonds, num_atoms, BYTESIZE);
#else
  Array_1d_Create (&num_2bonds, num_atoms, BYTESIZE);
#endif
  Array_Set (&num_2bonds, 0);

  for (atom = 0; atom < num_atoms; atom++)
    {
    if (SSpiro_Atom (xtr_p, atom, num_atoms) == TRUE)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0, double_bond_cnt = 0; neigh < num_neighbors;
           neigh++)
        if (Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh) == 2)
          double_bond_cnt++;

      Array_1d8_Put (&num_2bonds, atom, double_bond_cnt);
      num_psuedo_atoms += double_bond_cnt - 1;
      }
    }

  map_p = AtmArr_Create (num_atoms + num_psuedo_atoms, ATMARR_NOBONDS_WORD,
    FALSE);
  Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
  for (atom = 0; atom < num_atoms; atom++)
    Array_1d16_Put (AtmArr_Array_Get (map_p), atom, atom);

  /* whole section below is ripe for off-by-one errors */

  txtr_p = Xtr_CopyExpand (xtr_p, num_psuedo_atoms);
  next_pseudo_atom = num_atoms;
  for (atom = 0; atom < num_atoms; atom++)
    {
    if (Array_1d8_Get (&num_2bonds, atom) > 0)
      {                 /* the atom is a spiro atom */
      num_neighbors = Xtr_Attr_NumNeighbors_Get (txtr_p, atom);
      double_bond_cnt = Array_1d8_Get (&num_2bonds, atom);
      new_neigh_cnt = num_neighbors - double_bond_cnt + 1;
      new_neigh_id = Xtr_Attr_Atomid_Get (txtr_p, atom);
      last_double_bond = 0;
      for (k = 2; k <= double_bond_cnt; k++)
        {
        for (neigh = last_double_bond + 1;
            ((neigh < num_neighbors)
             && (Xtr_Attr_NeighborBond_Get (txtr_p, atom, neigh) != 2));
             neigh++)
          /* Empty loop body */ ;

        last_double_bond = neigh;
        Array_1d16_Put (AtmArr_Array_Get (map_p), next_pseudo_atom, atom);
        Xtr_Attr_Atomid_Put (txtr_p, next_pseudo_atom, new_neigh_id);
        Xtr_Attr_NumNeighbors_Put (txtr_p, next_pseudo_atom, new_neigh_cnt);
        for (neigh = 0, next_index = 0;
             neigh < num_neighbors; neigh++)
          {
          if ((neigh == last_double_bond)
             || (Xtr_Attr_NeighborBond_Get (txtr_p, atom, neigh)
             != BOND_DOUBLE))
            {
            neigh_id = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);
            bondsize = Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh);
            Xtr_Attr_NeighborId_Put (txtr_p, next_pseudo_atom, next_index,
              neigh_id);
            Xtr_Attr_NeighborBond_Put (txtr_p, next_pseudo_atom, next_index,
              bondsize);
            if (bondsize == BOND_DOUBLE)
              {
              for (l = 0; ((l < MX_NEIGHBORS) &&
                  (Xtr_Attr_NeighborId_Get (txtr_p, neigh_id, l) != atom));
                  l++)
                /* Empty loop body */ ;

              Xtr_Attr_NeighborId_Put (txtr_p, neigh_id, l, next_pseudo_atom);
              Xtr_Attr_NeighborBond_Put (txtr_p, neigh_id, l, BOND_DOUBLE);
              }
            else
              {
              l = Xtr_Attr_NumNeighbors_Get (txtr_p, neigh_id) + 1;

              ASSERT (l <= MX_NEIGHBORS,
                {
                IO_Exit_Error (R_XTR, X_SYNERR,
                  "Neighbor index too big in SSpiro_Compound_Find in\
 Xtr_ResonantBonds_Set");
                });

              Xtr_Attr_NeighborId_Put (txtr_p, neigh_id, l, next_pseudo_atom);
              Xtr_Attr_NeighborBond_Put (txtr_p, neigh_id, l, bondsize);
              Xtr_Attr_NumNeighbors_Put (txtr_p, neigh_id, l);
              }
            next_index++;
            }              /* End of if-neigh == last_double_bond */
          }                /* End for-neigh loop */
        next_pseudo_atom++;
        }                  /* End of for-j < last_double_bond loop */

      for (neigh = 0, next_index = 0; neigh < num_neighbors; neigh++)
        {
        if ((neigh > last_double_bond) ||
           (Xtr_Attr_NeighborBond_Get (txtr_p, atom, neigh) != 2))
          {
          Xtr_Attr_NeighborId_Put (txtr_p, atom, next_index,
            Xtr_Attr_NeighborId_Get (txtr_p, atom, neigh));
          Xtr_Attr_NeighborBond_Put (txtr_p, atom, next_index,
            Xtr_Attr_NeighborBond_Get (txtr_p, atom, neigh));
          next_index++;
          }
        }
      Xtr_Attr_NumNeighbors_Put (txtr_p, atom, new_neigh_cnt);
      }               /* End of if-spiro atom block */
    }                 /* End for-atom loop */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&num_2bonds", "resonantbonds", &num_2bonds);
#else
  Array_Destroy (&num_2bonds);
#endif
  rxtr_p = Xtr_Copy (txtr_p);
  txtr_p = SGeneric_Compound_Find (txtr_p, rbcb_p);
  Xtr_Destroy (txtr_p);
  SRemaining_Huckelness_Find (rxtr_p, rbcb_p);
  Xtr_Destroy (rxtr_p);
  Stack_Pop (RsbndCB_Map_Get (rbcb_p));

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SSpiro_Compound_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 STables_Free
*
*    This routine simply frees up a number of arrays.
*
*  Used to be:
*
*    free_tables:
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
static void STables_Free
  (
  Array_t       *a1,                         /* Array to free */
  Array_t       *a2,                         /* Array to free */
  Array_t       *a3,                         /* Array to free */
  Array_t       *a4,                         /* Array to free */
  Array_t       *a5,                         /* Array to free */
  Array_t       *a6,                         /* Array to free */
  Array_t       *a7                          /* Array to free */
  )
{
#ifdef _MIND_MEM_
  mind_Array_Destroy ("a1", "resonantbonds", a1);
  mind_Array_Destroy ("a2", "resonantbonds", a2);
  mind_Array_Destroy ("a3", "resonantbonds", a3);
  mind_Array_Destroy ("a4", "resonantbonds", a4);
  mind_Array_Destroy ("a5", "resonantbonds", a5);
  mind_Array_Destroy ("a6", "resonantbonds", a6);
  mind_Array_Destroy ("a7", "resonantbonds", a7);
#else
  Array_Destroy (a1);
  Array_Destroy (a2);
  Array_Destroy (a3);
  Array_Destroy (a4);
  Array_Destroy (a5);
  Array_Destroy (a6);
  Array_Destroy (a7);
#endif

  return;
}

/****************************************************************************
*
*  Function Name:                 STraverse
*
*    This routine traverses from the specified expansion point (node with
*    three neighbors) the specified branch.  The point is from the atom, and
*    the particular branch is defined by the neighbor index of the bond.
*    A branch is complete when another expansion point is found, and branches
*    are only counted in one direction.  For the branch the length, type, end
*    point and neighbor index of the first bond are recorded for later 
*    reconstructing the branch.
*
*  Used to be:
*
*    traverse:
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
static void STraverse
  (
  U16_t          atom,                      /* Atom to start at */
  U8_t           neigh,                     /* Neighbor index to start at */
  Xtr_t         *xtr_p,                     /* Current Xtr */
  S16_t         *last_branch_p,             /* Branch index being processed */
  Array_t       *end1_p,                    /* 1-d word, branch end atom id */
  Array_t       *traversed_p,               /* 2-d bit, bonds traversed */
  Array_t       *three_neigh_p,             /* 1-d bit, three neighbor flags */
  Array_t       *end2_p,                    /* 1-d word, other branch end id */
  Array_t       *index_p,                   /* 1-d byte, neigh index of first
    bond in branch */
  Array_t       *length_p,                  /* 1-d word, length of branch */
  Array_t       *type_p                     /* 1-d byte, type of branch */
  )
{
  U16_t          neighid;                   /* Neighbor atom id */
  U16_t          temp, last;                /* Neighbor ids for list trav. */
  U16_t          length;                    /* Counter */
  U16_t          cur_branch;                /* Save derefs. */
  U8_t           bond1, bond2;              /* Bonds between atom and neigh */
  U8_t           last_idx;                  /* Neigh. index of last bond in 
    branch */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering STraverse, atom = %u, neigh = %hu, Xtr = %p, last branch = %d,"
    " end1 = %p, traversed = %p, three neighbors = %p, n2 = %p, index = %p,"
    " length = %p, type = %p", atom, neigh, xtr_p, *last_branch_p, end1_p, 
    traversed_p, three_neigh_p, end2_p, index_p, length_p, type_p));

  cur_branch = ++(*last_branch_p);
  Array_1d16_Put (end1_p, cur_branch, atom);
  Array_2d1_Put (traversed_p, atom, neigh, TRUE);
  bond1 = Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh);
  neighid = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh);

  /* Traverse the branch to find where we meet root atom again */

  for (length = 1, last = atom; Array_1d1_Get (three_neigh_p, neighid) ==
       FALSE; length++)
    {
    temp = neighid;
    if (Xtr_Attr_NeighborId_Get (xtr_p, neighid, 0) == last)
      neighid = Xtr_Attr_NeighborId_Get (xtr_p, neighid, 1);
    else
      neighid = Xtr_Attr_NeighborId_Get (xtr_p, neighid, 0);
    last = temp;
    }

  /* For Huckel's Rule, take length mod 4 (should be 2) */

  length %= 4;
  last_idx = Xtr_Attr_NeighborIndex_Find (xtr_p, neighid, last);
  bond2 = Xtr_Attr_NeighborBond_Get (xtr_p, neighid, last_idx);
  Array_2d1_Put (traversed_p, neighid, last_idx, TRUE);
  Array_1d16_Put (end2_p, cur_branch, neighid);
  Array_1d8_Put (index_p, cur_branch, neigh);
  Array_1d16_Put (length_p, cur_branch, length);

  if (bond1 + bond2 == 2 * BOND_SINGLE)
    {
    Array_1d8_Put (type_p, cur_branch, ONEBRANCH);
    }
  else
    if (bond1 + bond2 == BOND_SINGLE + BOND_DOUBLE)
      {
      Array_1d8_Put (type_p, cur_branch, EVENBRANCH);
      }
  else
    if (bond1 + bond2 == 2 * BOND_DOUBLE)
      Array_1d8_Put (type_p, cur_branch, TWOBRANCH);

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving STraverse, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SValence4_Compound_Find
*
*    This routine finds ABC bonds in Valence-4 compounds (not necessarily
*    ring systems).
*
*  Used to be:
*
*    findr_val4_comp:
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
static Xtr_t *SValence4_Compound_Find
  (
  Xtr_t         *xtr_p,                      /* Molecule to check */
  ResbondsCB_t  *rbcb_p                      /* Control block */
  )
{
  Xtr_t         *rxtr_p;                     /* Molecule to check */
  AtomArray_t   *map_p;                      /* Temp. mapping */
  U16_t          num_atoms;                  /* # of atoms in m'cule */
  U16_t          num_ringsys;                /* # ring systems in m'cule */
  U16_t          ring;                       /* Counter */
  U16_t          stunted_nodes;              /* Nodes to punt */
  AtomArray_t   *bonds_p;                    /* Bond array temp. */

  if (SNot_Worth_Checking (xtr_p, rbcb_p) == TRUE)
    return xtr_p;

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SValence4_Compound_Find, Xtr = %p", xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SValence4_Compound_Find");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  xtr_p = SImpossible_Sequences_Remove (xtr_p, &stunted_nodes, rbcb_p);
  num_atoms = Xtr_NumAtoms_Get (xtr_p);
  Xtr_Rings_Set (xtr_p);
  num_ringsys = Xtr_Rings_NumRingSys_Get (xtr_p);

  bonds_p = AtmArr_Create (num_atoms, ATMARR_BONDS_BIT, FALSE);
  Stack_PushAdd (RsbndCB_Val4Comp_Get (rbcb_p), bonds_p);

  for (ring = 0; ring < num_ringsys; ring++)
    {
    Ring_Bonds_Set (xtr_p, ring, AtmArr_Array_Get (bonds_p));
    map_p = AtmArr_Create (num_atoms, ATMARR_NOBONDS_WORD, FALSE);
    Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
    rxtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
      AtmArr_Array_Get (map_p));

    if (SAllAtomsSingleDouble (rxtr_p) == FALSE)
      rxtr_p = SValence4_Compound_Find (rxtr_p, rbcb_p);
    else 
      SValence4_Ring_Find (rxtr_p, rbcb_p);

    Xtr_Destroy (rxtr_p);
    Stack_Pop (RsbndCB_Map_Get (rbcb_p));
    }

  Stack_Pop (RsbndCB_Val4Comp_Get (rbcb_p));
  for (ring = 0; ring < stunted_nodes; ring++)
    Stack_Pop (RsbndCB_Map_Get (rbcb_p));

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SValence4_Compound_Find, Xtr = %p", xtr_p));

  return xtr_p;
}

/****************************************************************************
*
*  Function Name:                 SValence4_Ring_Find
*
*    This routine finds ABC bonds in Valence-4 rings.  If they have no
*    valences higher than 4 then they are strictly carbocyclic, ie no
*    hetero atoms.
*
*    Loop through all the atoms.  Note the ones with three neighbors, they
*    are candidates for expansion nodes, ie for splitting the molecule up into
*    simpler "subgoals" to search for resonant bonds in.
*
*  Used to be:
*
*    findr_val4_ring:
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
static void SValence4_Ring_Find
  (
  Xtr_t         *xtr_p,                      /* Molecule to check */
  ResbondsCB_t  *rbcb_p                      /* Control block */
  )
{
  U32_t          tmp;                        /* For finding max-good branch */
  U32_t          max;                        /* For finding max-good branch */
  Xtr_t         *txtr_p;                     /* Temp. for recursion */
  AtomArray_t   *bonds_p;                    /* Current bond mask */
  AtomArray_t   *map_p;                      /* Current mapping array */
  U16_t          num_atoms;                  /* # atoms of m'cule */
  U16_t          num_neighbors;              /* # neighbors of atom */
  S16_t          num_branches;               /* # branches/exp. points */
  U16_t          atom;                       /* Counter */
  S16_t          branch, branch1, branch2;   /* Counters */
  U16_t          neighid;                    /* Neighbor atom id */
  U16_t          stunted_nodes;              /* # useless mapping perm. */
  S16_t          atom_map;                   /* Original atom index */
  U16_t          neigh_map;                  /* Original neigh. index */
  U16_t          count;                      /* Counter */
  S16_t          branch_max;                 /* "Best" branch from SRelation */
  S16_t          last_branch;                /* Branch index being processed */
  S16_t          branch_idx;                 /* For branch work */
  U8_t           neigh;                      /* Counter */
  Boolean_t      found;                      /* Flag for expansion point */
  Boolean_t      non_abc_bond_found;         /* Flag for non-ABC bond */
  Array_t        three_neigh;                /* 1-d bit, three neighbor flag */
  Array_t        end1;                       /* 1-d word, atom index at end of
                                                branch */
  Array_t        end2;                       /* 1-d word, atom idx, othr end */
  Array_t        index;                      /* 1-d byte, neigh idx of first
                                                bond traversed in branch */
  Array_t        length;                     /* 1-d word, length of branch */
  Array_t        traversed;                  /* 2-d bit, traversed bond flag */
  Array_t        type;                       /* 1-d byte, type of branch */
  Array_t        up_ptr;                     /* 1-d word, good branch above bad
                                                branch */
  Array_t        other;                      /* 1-d word, =bondsize branches */
  Array_t        numnonabc;                  /* 1-d word, # unknown abc-bond */
  Array_t        numabc;                     /* 1-d word, # known abc-bonds */
  Array_t        common_node;                /* 1-d word, common atom between
                                                two branches */
  Array_t        visited;                    /* 1-d bit, visited atom flags */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Entering SValence4_Ring_Find, Xtr = %p", xtr_p));

  TRACE_DO (DB_RESBONDSTATIC, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In SValence4_Ring_Find");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
  bonds_p = AtmArr_Create (num_atoms, ATMARR_BONDS_BIT, TRUE);
  Stack_PushAdd (RsbndCB_Val4Ring_Get (rbcb_p), bonds_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&three_neigh", "resonantbonds{10}", &three_neigh, num_atoms, BITSIZE);
#else
  Array_1d_Create (&three_neigh, num_atoms, BITSIZE);
#endif
  Array_Set (&three_neigh, FALSE);

  /* Check for expansion points and to make sure all atoms are joined by
     potentially resonant bonds.
  */

  for (atom = 0, num_branches = 0, found = FALSE, non_abc_bond_found = FALSE;
       atom < num_atoms; atom++)
    {
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    if (num_neighbors == 3)
      {
      Array_1d1_Put (&three_neigh, atom, TRUE);
      found = TRUE;
      num_branches++;
      }

    for (neigh = 0; neigh < num_neighbors; neigh++)
      {
      atom_map = SMappedTo (atom, rbcb_p);
      neigh_map = SMappedTo (Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh),
        rbcb_p);
      if (Array_2d1_Get (RsbndCB_ABCbonds_Get (rbcb_p), atom_map,
          Xtr_Attr_NeighborIndex_Find (RsbndCB_Xtr_Get (rbcb_p), atom_map,
          neigh_map)) == FALSE)
        non_abc_bond_found = TRUE;
      }
    }

  /* If we found a non-abc bond we are done. */

  if (non_abc_bond_found == FALSE)
    {
    /* Since all bonds have previously been found to be abc-bonds, undo our
       saved information and return.
    */

    Stack_Pop (RsbndCB_Val4Ring_Get (rbcb_p));
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&three_neigh", "resonantbonds", &three_neigh);
#else
    Array_Destroy (&three_neigh);
#endif

    DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
      "Leaving SValence4_Ring_Find, No undiscovered ABC bonds, status = <void>"));

    return;
    }

  /* If we didn't find an expansion point, ...
     And return.
  */

  if (found == FALSE)
    {
    SBonds_Mark (xtr_p, ((num_atoms % 4) == 2), rbcb_p);

    Stack_Pop (RsbndCB_Val4Ring_Get (rbcb_p));
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&three_neigh", "resonantbonds", &three_neigh);
#else
    Array_Destroy (&three_neigh);
#endif

    DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
      "Leaving SValence4_Ring_Find, No expansion points, status = <void>"));

    return;
    }

  /* Num_branches now equals the number of branches (was # expansion points).
     Prepare to calculate branch information.
  */

  num_branches = (3 * num_branches) / 2;

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_MAJOR, (outbuf,
    "In SValence4_Ring_Find, # of branches = %d", num_branches));

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&end1", "resonantbonds{10}", &end1, num_branches, WORDSIZE);
  mind_Array_1d_Create ("&end2", "resonantbonds{10}", &end2, num_branches, WORDSIZE);
  mind_Array_1d_Create ("&type", "resonantbonds{10}", &type, num_branches, BYTESIZE);
  mind_Array_1d_Create ("&index", "resonantbonds{10}", &index, num_branches, BYTESIZE);
  mind_Array_1d_Create ("&length", "resonantbonds{10}", &length, num_branches, WORDSIZE);
  mind_Array_2d_Create ("&traversed", "resonantbonds{10}", &traversed, num_atoms, MX_NEIGHBORS, BITSIZE);
#else
  Array_1d_Create (&end1, num_branches, WORDSIZE);
  Array_1d_Create (&end2, num_branches, WORDSIZE);
  Array_1d_Create (&type, num_branches, BYTESIZE);
  Array_1d_Create (&index, num_branches, BYTESIZE);
  Array_1d_Create (&length, num_branches, WORDSIZE);
  Array_2d_Create (&traversed, num_atoms, MX_NEIGHBORS, BITSIZE);
#endif
  Array_Set (&traversed, FALSE);

  /* Type check all the branches.  Loop over all the atoms.
  */

  for (atom = 0, last_branch = -1; atom < num_atoms; atom++)
    if (Array_1d1_Get (&three_neigh, atom) == TRUE)
      {
      num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
      for (neigh = 0; neigh < num_neighbors; neigh++)
        {
        if (Array_2d1_Get (&traversed, atom, neigh) == FALSE)
          STraverse (atom, neigh, xtr_p, &last_branch, &end1, &traversed,
            &three_neigh, &end2, &index, &length, &type);

        for (branch = 0; branch < last_branch; branch++)
          if (SParallel (branch, last_branch, &end1, &end2) == TRUE)
            {
            DEBUG (R_XTR, DB_RESBONDSTATIC, TL_MAJOR, (outbuf,
              "In SValence4_Ring_Find, SParallel (xtr_p, %d, %d, end1,"
              " end2) = FALSE", branch, last_branch));

            SParallel_System_Find (xtr_p, branch, last_branch, &end1, &index,
              &type, &length, rbcb_p);
            STables_Free (&three_neigh, &end1, &end2, &type, &index,
              &length, &traversed);
            Stack_Pop (RsbndCB_Val4Ring_Get (rbcb_p));

            DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
              "Leaving SValence4_Ring_Find, Parallel System investigated,\
 status = <void>"));

            return;
            }
        }                                    /* End of for-neigh loop */
      }                                      /* If three_neigh block */

  /* Count types of branches found so far */

  for (branch = 0, branch1 = 0, branch2 = 0; branch < num_branches; branch++)
    {
    if (Array_1d8_Get (&type, branch) == TWOBRANCH)
      branch2++;
    else
      if (Array_1d8_Get (&type, branch) == ONEBRANCH)
        branch1++;
    }

  if (branch2 == 0)
    {
    /* There are no 2 branches */

    DEBUG (R_XTR, DB_RESBONDSTATIC, TL_MAJOR, (outbuf,
      "In SValence4_Ring_Find, no 2 branches found"));

    Array_Set (AtmArr_Array_Get (bonds_p), TRUE);
    for (branch = 0, count = 0; branch < num_branches; branch++)
      {
      if (Array_1d8_Get (&type, branch) == ONEBRANCH)
        {
        SBranch_Remove (xtr_p, Array_1d16_Get (&end1, branch),
          Array_1d8_Get (&index, branch), AtmArr_Array_Get (bonds_p),
          &stunted_nodes);
        count += stunted_nodes;
        }
      }

    map_p = AtmArr_Create (num_atoms - count, ATMARR_NOBONDS_WORD, FALSE);
    Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
    txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
      AtmArr_Array_Get (map_p));
    txtr_p = SValence4_Compound_Find (txtr_p, rbcb_p);
    Xtr_Destroy (txtr_p);
    Stack_Pop (RsbndCB_Map_Get (rbcb_p));

    STables_Free (&three_neigh, &end1, &end2, &type, &index, &length,
      &traversed);
    Stack_Pop (RsbndCB_Val4Ring_Get (rbcb_p));
    }
  else
    if (branch1 == 2 * branch2)
      {
      /* # 2-branches & 2 # 1-branches */

      DEBUG (R_XTR, DB_RESBONDSTATIC, TL_MAJOR, (outbuf,
        "In SValence4_Ring_Find, # 2 branches = %d", branch2));

      SBonds_Mark (xtr_p, FALSE, rbcb_p);
      STables_Free (&three_neigh, &end1, &end2, &type, &index, &length,
        &traversed);
      Stack_Pop (RsbndCB_Val4Ring_Get (rbcb_p));
      }
    else
      {
      /* Expand subgoals */

#ifdef _MIND_MEM_
      mind_Array_1d_Create ("&other", "resonantbonds{10}", &other, num_branches, WORDSIZE);
      mind_Array_1d_Create ("&numnonabc", "resonantbonds{10}", &numnonabc, num_branches, WORDSIZE);
      mind_Array_1d_Create ("&up_ptr", "resonantbonds{10}", &up_ptr, num_branches, WORDSIZE);
      mind_Array_1d_Create ("&numabc", "resonantbonds{10}", &numabc, num_branches, WORDSIZE);
      mind_Array_1d_Create ("&common_node", "resonantbonds{10}", &common_node, num_branches, WORDSIZE);
      mind_Array_1d_Create ("&visited", "resonantbonds{10}", &visited, num_atoms, BITSIZE);
#else
      Array_1d_Create (&other, num_branches, WORDSIZE);
      Array_1d_Create (&numnonabc, num_branches, WORDSIZE);
      Array_1d_Create (&up_ptr, num_branches, WORDSIZE);
      Array_1d_Create (&numabc, num_branches, WORDSIZE);
      Array_1d_Create (&common_node, num_branches, WORDSIZE);
      Array_1d_Create (&visited, num_atoms, BITSIZE);
#endif
      Array_Set (&other, INVALID_BRANCH);
      Array_Set (&numnonabc, 0);
      Array_Set (&up_ptr, INVALID_BRANCH);
      Array_Set (&numabc, 0);
      Array_Set (&common_node, INVALID_BRANCH);
      Array_Set (&visited, FALSE);

      DEBUG (R_XTR, DB_RESBONDSTATIC, TL_MAJOR, (outbuf,
        "In SValence4_Ring_Find, subgoal expansion"));

      for (branch = 0; branch < num_branches; branch++)
        {
        if (Array_1d8_Get (&type, branch) == EVENBRANCH)
          {
          branch_max = branch;

          /* Execute loop twice, exactly */

          for (neighid = Array_1d16_Get (&end1, branch); neighid <=
               Array_1d16_Get (&end2, branch); neighid += Array_1d16_Get (
               &end2, branch) - Array_1d16_Get (&end1, branch))
            if (Array_1d1_Get (&visited, neighid) == FALSE)
              {
              Array_1d1_Put (&visited, neighid, TRUE);
              for (atom_map = branch + 1; atom_map < num_branches; atom_map++)
                if (((Array_1d16_Get (&end1, atom_map) == neighid)
                    || (Array_1d16_Get (&end2, atom_map) == neighid))
                    && (Array_1d8_Get (&type, atom_map) == EVENBRANCH))
                  SRelation (branch, atom_map, neighid, xtr_p, &end1, &index,
                    &other, &common_node, &numnonabc, &numabc, &up_ptr,
                    rbcb_p);
              }                              /* If-visited block */
          }                                  /* If-type block */
        }                                    /* end for-branch loop */

      /* Looking for branch with maximum value from SRelation.  PL/1 code
         uses bit-string concatenation here just to make life difficult.
         Not entirely sure about which direction to do this in.
      */

      for (branch = 0, max = 0; branch < num_branches; branch++)
        {
        tmp = (Array_1d16_Get (&numabc, branch) + Array_1d16_Get (&numnonabc,
          branch)) << 16;
        if (tmp > max)
          {
          max = tmp;
          branch_max = branch;
          }
        }

      SBranch_Remove (xtr_p, Array_1d16_Get (&end1, branch_max),
        Array_1d8_Get (&index, branch_max), AtmArr_Array_Get (bonds_p),
        &stunted_nodes);
      map_p = AtmArr_Create (num_atoms - stunted_nodes, ATMARR_NOBONDS_WORD,
        FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      txtr_p = SValence4_Compound_Find (txtr_p, rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));

      Array_Set (AtmArr_Array_Get (bonds_p), TRUE);
      branch_idx = Array_1d16_Get (&other, branch_max);

      if (branch_idx < 0)
        {
        branch_idx = -branch_idx;
        atom_map = Array_1d16_Get (&common_node, branch_max);

        /* Find the third branch which has node k as one of its ends.
           this is the one to remove next since branch_idx has its double bond
           end at k
        */

        for (branch = 0; ((branch < num_branches) && (branch == branch_max ||
             branch == branch_idx || (Array_1d16_Get (&end1, branch) !=
             atom_map && Array_1d16_Get (&end2, branch) != atom_map)));
             branch++)
          /* Empty loop body */ ;

        branch_idx = branch;
        }

      if (branch_idx == INVALID_BRANCH)
        {
        if (Xtr_Attr_NeighborBond_Get (xtr_p, Array_1d16_Get (&end1,
            branch_max), Array_1d8_Get (&index, branch_max)) == BOND_SINGLE)
          atom_map = Array_1d16_Get (&end1, branch_max);
        else
          atom_map = Array_1d16_Get (&end2, branch_max);

        for (branch = 0; ((branch < num_branches) && ((Array_1d16_Get (&end1,
             branch) != atom_map) && (Array_1d16_Get (&end2, branch) !=
             atom_map))); branch++)
          /* Empty loop body */ ;

        if (((Array_1d16_Get (&end1, branch) == atom_map) &&
            (Xtr_Attr_NeighborBond_Get (xtr_p, Array_1d16_Get (&end1, branch),
            Array_1d8_Get (&index, branch)) == BOND_SINGLE))
            || (Array_1d8_Get (&type, branch) == ONEBRANCH)
            || ((Array_1d8_Get (&type, branch) == EVENBRANCH)
            && (Xtr_Attr_NeighborBond_Get (xtr_p, Array_1d16_Get (&end1,
            branch), Array_1d8_Get (&index, branch) == BOND_DOUBLE))))
          branch_idx = branch;
        else
          for (branch_idx = branch + 1; ((branch_idx < num_branches) &&
               ((Array_1d16_Get (&end1, branch_idx) != atom_map) &&
               (Array_1d16_Get (&end2, branch_idx) != atom_map)));
               branch_idx++)
            /* Empty loop body */ ;
        }                                    /* End if-branch_idx =
                                                INVALID_BRANCH */

      SBranch_Remove (xtr_p, Array_1d16_Get (&end1, branch_idx),
        Array_1d8_Get (&index, branch_idx), AtmArr_Array_Get (bonds_p),
        &stunted_nodes);
      map_p = AtmArr_Create (num_atoms - stunted_nodes, ATMARR_NOBONDS_WORD,
        FALSE);
      Stack_PushAdd (RsbndCB_Map_Get (rbcb_p), map_p);
      txtr_p = Xtr_CopySubset (xtr_p, AtmArr_Array_Get (bonds_p),
        AtmArr_Array_Get (map_p));
      txtr_p = SValence4_Compound_Find (txtr_p, rbcb_p);
      Xtr_Destroy (txtr_p);
      Stack_Pop (RsbndCB_Map_Get (rbcb_p));

      STables_Free (&three_neigh, &end1, &end2, &type, &index, &length,
        &traversed);
      Stack_Pop (RsbndCB_Val4Ring_Get (rbcb_p));
#ifdef _MIND_MEM_
      mind_Array_Destroy ("&other", "resonantbonds", &other);
      mind_Array_Destroy ("&numnonabc", "resonantbonds", &numnonabc);
      mind_Array_Destroy ("&up_ptr", "resonantbonds", &up_ptr);
      mind_Array_Destroy ("&numabc", "resonantbonds", &numabc);
      mind_Array_Destroy ("&common_node", "resonantbonds", &common_node);
      mind_Array_Destroy ("&visited", "resonantbonds", &visited);
#else
      Array_Destroy (&other);
      Array_Destroy (&numnonabc);
      Array_Destroy (&up_ptr);
      Array_Destroy (&numabc);
      Array_Destroy (&common_node);
      Array_Destroy (&visited);
#endif
      }                                      /* End else-if not covered branch
                                                type */

  DEBUG (R_XTR, DB_RESBONDSTATIC, TL_PARAMS, (outbuf,
    "Leaving SValence4_Ring_Find, status = <void>"));

  return;
}
/* End of SValence4_Ring_Find */
/* End of RESONANTBONDS.C */
