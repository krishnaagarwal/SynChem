#ifndef _H_TSD_
#define _H_TSD_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     TSD.H
*
*    This module is the abstraction for the Tsd data-structure.  It is
*    a simpler form of molecule description than an XTR and it is
*    easier to get information from than a Sling.
*
*    Routines can be found in TSD.C unless otherwise noted
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
* 27-Feb-95  Krebsbach  Removed the restriction that the atom index be less
*                       than the number of atoms in the macros during 
*                       XTR_DEBUG mode:  Tsd_Atom_Flags_Put, Tsd_Atomid_Put, 
*                       Tsd_Atom_NeighborBond_Put, Tsd_AtomFlags_Dot_Put, 
*                       Tsd_Atom_NeighborId_Put, Tsd_AtomFlags_Asymmetry_Put
*                       (deleted :  || (atom) >= (tsd_p)->num_atoms).
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

/*** Literal Values ***/

#define PARITY_DONTCARE  (U8_t)-6
#define PARITY_INVALID1  (U8_t)-5
#define PARITY_INVALID2  (U8_t)-3
#define PARITY_TRIEVEN   (U8_t)-2
#define PARITY_TETRAEVEN (U8_t)-1
#define PARITY_INIT      (U8_t)0
#define PARITY_TETRAODD  (U8_t)1
#define PARITY_TRIODD    (U8_t)2
#define PARITY_UNSURE    (U8_t)5

#define BOND_DIR_UP      0
#define BOND_DIR_DOWN    1
#define BOND_DIR_LEFT    2
#define BOND_DIR_RIGHT   3
#define BOND_DIR_IN      4
#define BOND_DIR_OUT     5
#define BOND_DIR_INVALID (U8_t)-103

#define BOND_NONE        0
#define BOND_SINGLE      1
#define BOND_DOUBLE      2
#define BOND_TRIPLE      3
#define BOND_QUADRUPLE   4
#define BOND_VARIABLE    5
#define BOND_RESONANT    6

#define MX_BONDMULTIPLICITY 4

/* Mask for aromatic bonds, stored in higher bit positions beyond normal bond
   multiplicities.
*/

#define TSD_VALID           1
#define TSD_INVALID        (U16_t)-201

/*** Data Structures ***/

/* Each atom in a TSD is represented by a row.  The atom id, size of the bonds
   (single, double, triple), and the atom number (not id) on the other end of
   the bond are represented here.
*/

typedef struct s_tsdrow
  {
  struct tsdr1                            /* Array of neighbors */
    {
    U16_t       id;                       /* Atom # within the molecule */
    U8_t        bond;                     /* Bond type */
    } neighbors[MX_NEIGHBORS];
  U16_t         atomid;                   /* Atom identifier (atomic #) */
  U8_t          flags;                    /* Atom flags */
  } TsdRow_t;
#define TSDROWSIZE sizeof (TsdRow_t)

/* Flag literal values (derived by inspection from tsdxtr: code)
   These represent the known symmetry at a given atom.  I'm not sure
   exactly what 'dot' means.
*/

#define TsdM_DontCare      0x1
#define TsdM_Asymmetry     0x2
#define TsdM_Dot           0x4

/* TSD data structure.  This is the compact, no extraneous information,
   format for representing a molecule.
*/

typedef struct s_tsd
  {
  TsdRow_t     *atoms;                    /* One row per atom */
  U16_t         num_atoms;                /* Number of atoms (also rows) */
  } Tsd_t;
#define TSDSIZE sizeof (Tsd_t)

/** Field Access Macros for Tsd_t **/

/* Macro  Prototypes
   TsdRow_t *Tsd_AtomHandle_Get          (Tsd_t *);
   void      Tsd_AtomHandle_Put          (Tsd_t *, TsdRow_t *);
   U8_t      Tsd_Atom_Flags_Get          (Tsd_t *, U16_t);
   void      Tsd_Atom_Flags_Put          (Tsd_t *, U16_t, U8_t);
   Boolean_t Tsd_AtomFlags_Asymmetry_Get (Tsd_t *, U16_t);
   void      Tsd_AtomFlags_Asymmetry_Put (Tsd_t *, U16_t, Boolean_t);
   Boolean_t Tsd_AtomFlags_DontCare_Get  (Tsd_t *, U16_t);
   void      Tsd_AtomFlags_DontCare_Put  (Tsd_t *, U16_t, Boolean_t);
   Boolean_t Tsd_AtomFlags_Dot_Get       (Tsd_t *, U16_t);
   void      Tsd_AtomFlags_Dot_Put       (Tsd_t *, U16_t, Boolean_t);
   U8_t      Tsd_Atom_NeighborBond_Get   (Tsd_t *, U16_t, U8_t);
   void      Tsd_Atom_NeighborBond_Put   (Tsd_t *, U16_t, U8_t, U8_t);
   U16_t     Tsd_Atom_NeighborId_Get     (Tsd_t *, U16_t, U8_t);
   void      Tsd_Atom_NeighborId_Put     (Tsd_t *, U16_t, U8_t, U16_t);
   U16_t     Tsd_Atomid_Get              (Tsd_t *, U16_t);
   void      Tsd_Atomid_Put              (Tsd_t *, U16_t, U16_t);
   U16_t     Tsd_NumAtoms_Get            (Tsd_t *);
   void      Tsd_NumAtoms_Put            (Tsd_t *, U16_t);
*/

#ifndef XTR_DEBUG
#define Tsd_AtomHandle_Get(tsd_p)\
  (tsd_p)->atoms

#define Tsd_AtomHandle_Put(tsd_p, value)\
  (tsd_p)->atoms = (value)

/* Should have been $zdots: */

#define Tsd_Atom_Flags_Get(tsd_p, atom)\
  (tsd_p)->atoms[atom].flags

/* Used to be zdots: */

#define Tsd_Atom_Flags_Put(tsd_p, atom, value)\
  (tsd_p)->atoms[atom].flags = (value)

#define Tsd_AtomFlags_Asymmetry_Get(tsd_p, atom)\
  ((tsd_p)->atoms[atom].flags & TsdM_Asymmetry ? TRUE : FALSE)

#define Tsd_AtomFlags_Asymmetry_Put(tsd_p, atom, value)\
  { if ((value) == TRUE)\
    (tsd_p)->atoms[atom].flags |= TsdM_Asymmetry;\
  else\
    (tsd_p)->atoms[atom].flags &= ~TsdM_Asymmetry; }

#define Tsd_AtomFlags_DontCare_Get(tsd_p, atom)\
  ((tsd_p)->atoms[atom].flags & TsdM_DontCare ? TRUE : FALSE)

#define Tsd_AtomFlags_DontCare_Put(tsd_p, atom, value)\
  { if ((value) == TRUE)\
    (tsd_p)->atoms[atom].flags |= TsdM_DontCare;\
  else\
    (tsd_p)->atoms[atom].flags &= ~TsdM_DontCare; }

#define Tsd_AtomFlags_Dot_Get(tsd_p, atom)\
  ((tsd_p)->atoms[atom].flags & TsdM_Dot ? TRUE : FALSE)

#define Tsd_AtomFlags_Dot_Put(tsd_p, atom, value)\
  { if ((value) == TRUE)\
    (tsd_p)->atoms[atom].flags |= TsdM_Dot;\
  else\
    (tsd_p)->atoms[atom].flags &= ~TsdM_Dot; }

#define Tsd_Atom_NeighborBond_Get(tsd_p, atom, neighbor)\
  (tsd_p)->atoms[atom].neighbors[neighbor].bond

#define Tsd_Atom_NeighborBond_Put(tsd_p, atom, neighbor, value)\
  (tsd_p)->atoms[atom].neighbors[neighbor].bond = (value)

#define Tsd_Atom_NeighborId_Get(tsd_p, atom, neighbor)\
  (tsd_p)->atoms[atom].neighbors[neighbor].id

#define Tsd_Atom_NeighborId_Put(tsd_p, atom, neighbor, value)\
  (tsd_p)->atoms[atom].neighbors[neighbor].id = (value)

/* Used to be atomid: */

#define Tsd_Atomid_Get(tsd_p, row)\
  (tsd_p)->atoms[row].atomid

/* Used to be $atomid: */

#define Tsd_Atomid_Put(tsd_p, row, value)\
  (tsd_p)->atoms[row].atomid = (value)

/* Used to be n_atoms: */

#define Tsd_NumAtoms_Get(tsd_p)\
  (tsd_p)->num_atoms

/* Would have been $n_atoms: */

#define Tsd_NumAtoms_Put(tsd_p, value)\
  (tsd_p)->num_atoms = (value)

#else
#define Tsd_AtomHandle_Get(tsd_p)\
  ((tsd_p) < GBAddr ? ((Tsd_t *)HALTP)->atoms : (tsd_p)->atoms)

#define Tsd_AtomHandle_Put(tsd_p, value)\
  { if ((tsd_p) < GBAddr) HALT; else (tsd_p)->atoms = (value); }

/* Should have been $zdots: */

#define Tsd_Atom_Flags_Get(tsd_p, atom)\
  ((tsd_p) < GBAddr ? HALT : (tsd_p)->atoms[atom].flags)

/* Used to be zdots: */

#define Tsd_Atom_Flags_Put(tsd_p, atom, value)\
  { if ((tsd_p) < GBAddr || (atom) < 0) HALT;\
  else (tsd_p)->atoms[atom].flags = (value); }

#define Tsd_AtomFlags_Asymmetry_Get(tsd_p, atom)\
  ((tsd_p) < GBAddr || (atom) < 0 || (atom) >= (tsd_p)->num_atoms ? HALT :\
  (tsd_p)->atoms[atom].flags & TsdM_Asymmetry ? TRUE : FALSE)

#define Tsd_AtomFlags_Asymmetry_Put(tsd_p, atom, value)\
  { if ((tsd_p) < GBAddr || (atom) < 0) HALT;\
  else if ((value) == TRUE) (tsd_p)->atoms[atom].flags |= TsdM_Asymmetry;\
  else (tsd_p)->atoms[atom].flags &= ~TsdM_Asymmetry; }

#define Tsd_AtomFlags_DontCare_Get(tsd_p, atom)\
  ((tsd_p) < GBAddr || (atom) < 0 || (atom) >= (tsd_p)->num_atoms ? HALT :\
  (tsd_p)->atoms[atom].flags & TsdM_DontCare ? TRUE : FALSE)

#define Tsd_AtomFlags_DontCare_Put(tsd_p, atom, value)\
  { if ((tsd_p) < GBAddr || (atom) < 0) HALT;\
  else if ((value) == TRUE) (tsd_p)->atoms[atom].flags |= TsdM_DontCare;\
  else (tsd_p)->atoms[atom].flags &= ~TsdM_DontCare; }

#define Tsd_AtomFlags_Dot_Get(tsd_p, atom)\
  ((tsd_p) < GBAddr || (atom) < 0 || (atom) >= (tsd_p)->num_atoms ? HALT :\
  (tsd_p)->atoms[atom].flags & TsdM_Dot ? TRUE : FALSE)

#define Tsd_AtomFlags_Dot_Put(tsd_p, atom, value)\
  { if ((tsd_p) < GBAddr || (atom) < 0) HALT;\
  else if ((value) == TRUE) (tsd_p)->atoms[atom].flags |= TsdM_Dot;\
  else (tsd_p)->atoms[atom].flags &= ~TsdM_Dot; }

#define Tsd_Atom_NeighborBond_Get(tsd_p, atom, neighbor)\
  ((tsd_p) < GBAddr || (atom) < 0 || (atom) >= (tsd_p)->num_atoms ||\
  (neighbor) < 0 || (neighbor) >= MX_NEIGHBORS ? HALT :\
  (tsd_p)->atoms[atom].neighbors[neighbor].bond)

#define Tsd_Atom_NeighborBond_Put(tsd_p, atom, neighbor, value)\
  { if ((tsd_p) < GBAddr || (atom) < 0 ||\
  (neighbor) < 0 || (neighbor) >= MX_NEIGHBORS) HALT;\
  else (tsd_p)->atoms[atom].neighbors[neighbor].bond = (value); }

#define Tsd_Atom_NeighborId_Get(tsd_p, atom, neighbor)\
  ((tsd_p) < GBAddr || (atom) < 0 || (atom) >= (tsd_p)->num_atoms ||\
  (neighbor) < 0 || (neighbor) >= MX_NEIGHBORS ? HALT :\
  (tsd_p)->atoms[atom].neighbors[neighbor].id)

#define Tsd_Atom_NeighborId_Put(tsd_p, atom, neighbor, value)\
  { if ((tsd_p) < GBAddr || (atom) < 0 ||\
  (neighbor) < 0 || (neighbor) >= MX_NEIGHBORS) HALT;\
  (tsd_p)->atoms[atom].neighbors[neighbor].id = (value); }

/* Used to be atomid: */

#define Tsd_Atomid_Get(tsd_p, atom)\
  ((tsd_p) < GBAddr || (atom) < 0 || (atom) >= (tsd_p)->num_atoms ? HALT :\
  (tsd_p)->atoms[atom].atomid)

/* Used to be $atomid: */

#define Tsd_Atomid_Put(tsd_p, atom, value)\
  { if ((tsd_p) < GBAddr || (atom) < 0) HALT;\
  (tsd_p)->atoms[atom].atomid = (value); }

/* Used to be n_atoms: */

#define Tsd_NumAtoms_Get(tsd_p)\
  ((tsd_p) < GBAddr ? HALT : (tsd_p)->num_atoms)

/* Would have been $n_atoms: */

#define Tsd_NumAtoms_Put(tsd_p, value)\
  { if ((tsd_p) < GBAddr) HALT; else (tsd_p)->num_atoms = (value); }
#endif

/*** Macros ***/

/* Macro prototypes
   Boolean Tsd_Atom_AreConnected (Tsd_t *, U16_t, U16_t);
   U8_t    Tsd_Atom_BondSize_Get (Tsd_t *, U16_t, U16_t);
   void    Tsd_RowCopy           (Tsd_t *, U16_t, Tsd_t *, U16_t);
   Boolean Tsd_Validate          (Tsd_t *, U16_t);
*/

#ifndef XTR_DEBUG
/* Used to be connected: */

#define Tsd_Atom_AreConnected(tsd_p, atom1, atom2)\
  (Tsd_Atom_Index_Find (tsd_p, atom1, atom2) != TSD_INVALID ? TRUE : FALSE)

/* Used to be bond:, getbond: */

#define Tsd_Atom_BondSize_Get(tsd_p, atom1, atom2)\
  (tsd_p)->atoms[atom1].neighbors[\
  Tsd_Atom_Index_Find (tsd_p, atom1, atom2)].bond

#define Tsd_RowCopy(dst_p, drow, src_p, srow)\
  memcpy (&(dst_p)->atoms[drow], &(src_p)->atoms[srow], TSDROWSIZE)

/* Check to see if the row we want is beyond the end of the TSD */

#define Tsd_Validate(tsd_p, row)\
  ((tsd_p)->number >= row ? TRUE : FALSE)
#else
/* Used to be connected: */

#define Tsd_Atom_AreConnected(tsd_p, atom1, atom2)\
  ((tsd_p) < GBAddr || (atom1) < 0 || (atom1) >= (tsd_p)->num_atoms ||\
  (atom2) < 0 || (atom2) >= (tsd_p)->num_atoms ? HALT :\
  Tsd_Atom_Index_Find (tsd_p, atom1, atom2) != TSD_INVALID ? TRUE : FALSE)

/* Used to be bond:, getbond: */

#define Tsd_Atom_BondSize_Get(tsd_p, atom1, atom2)\
  ((tsd_p) < GBAddr || (atom1) < 0 || (atom1) >= (tsd_p)->num_atoms ||\
  (atom2) < 0 || (atom2) >= (tsd_p)->num_atoms ? HALT :\
  (tsd_p)->atoms[atom1].neighbors[\
  Tsd_Atom_Index_Find (tsd_p, atom1, atom2)].bond)

#define Tsd_RowCopy(dst_p, drow, src_p, srow)\
  { if ((dst_p) < GBAddr || (src_p) < GBAddr || (drow) < 0 || (drow) >=\
  (dst_p)->num_atoms || (srow) < 0 || (srow) >= (src_p)->num_atoms) HALT; else\
  memcpy (&(dst_p)->atoms[drow], &(src_p)->atoms[srow], TSDROWSIZE); }

/* Check to see if the row we want is beyond the end of the TSD */

#define Tsd_Validate(tsd_p, row)\
  ((tsd_p) < GBAddr ? HALT : (tsd_p)->num_atoms >= row ? TRUE : FALSE)
#endif

/*** Routine Prototypes ***/

void   Tsd_Atom_Bond_Change    (Tsd_t *, U16_t, U16_t, U8_t);
void   Tsd_Atom_Connect        (Tsd_t *, U16_t, U16_t, U8_t);
void   Tsd_Atom_ConnectChange  (Tsd_t *, U16_t, U16_t, U8_t);
void   Tsd_Atom_Disconnect     (Tsd_t *, U16_t, U16_t);
U16_t  Tsd_Atom_Empty_Find     (Tsd_t *);
U16_t  Tsd_Atom_Index_Find     (Tsd_t *, U16_t, U16_t);
void   Tsd_Atom_Invert         (Tsd_t *, U16_t);
void   Tsd_Atom_Neighbors_Get  (Tsd_t *, U16_t, Array_t *);
U8_t   Tsd_Atom_Valence_Get    (Tsd_t *, U16_t);
Tsd_t *Tsd_Copy                (Tsd_t *);
Tsd_t *Tsd_Create              (U16_t);
void   Tsd_Destroy             (Tsd_t *);
void   Tsd_Dump                (Tsd_t *, FileDsc_t *);
void   Tsd_Expand              (Tsd_t *, U16_t);
U16_t  Tsd_LastAtomid_Find     (Tsd_t *, U16_t);
void   Tsd_MatchCompress       (Tsd_t *, Tsd_t *, U32_t *);
void   Tsd_MatchCompress_Fix   (Tsd_t *, Tsd_t *, U32_t *);
void   Tsd_Strip_H             (Tsd_t *, Boolean_t);
void   Tsd_Verify              (Tsd_t *);

/* End of Tsd.H */
#endif
