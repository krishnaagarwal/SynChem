#ifndef _H_XTR_
#define _H_XTR_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     XTR.H
*
*    This module contains the abstraction for the Xtr data-structure.
*    It is the primary molecule description.  It allows Synchem to answer any
*    question it might ask about a molecule (with the help of its
*    sub-structures).
*
*    Routines can be found in XTR.C unless otherwise noted
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
* 09-Oct-96  Krebsbach  Reconciled #includes.
* 23-May-95  Cheung     Changed Xtr_Rings_NumRingSys_Get to function, added -
*			if ring_p == NULL, call Xtr_Rings_Set
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_AROMATICS_
#include "aromatics.h"
#endif

#ifndef _H_ATOMS_
#include "atoms.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_FUNCGROUPS_
#include "funcgroups.h"
#endif

#ifndef _H_NAME_
#include "name.h"
#endif

#ifndef _H_RINGS_
#include "rings.h"
#endif

/*** Literal Values ***/

#define XTR_VALID        (U16_t)1
#define XTR_INVALID      (U16_t)-202

/*** Data Structures ***/

/* This is modeled after the TSD row structure, in fact field names have been
   changed so that they are not gratuitously different from the TSD (but as a
   result they appear gratuitously different from the original XTR)

   The atoms are referred to by their index into the attributes array in the
   XTR of which the XtrRow_t is a descriptor.  The id field in the neighbors
   array contains the index of the neighbor in the attributes array.  The
   atomid field has the atomic number (ex. 6 for Carbon).  Num_neighbors has
   the number of valid neighbors to look at.
*/

typedef struct s_rxtr
  {
  U16_t         atomid;               /* Atomic number (was id) */
  U8_t          flags;                /* Flag byte */
  U8_t          num_neighbors;        /* Number of adjacent atoms */
  struct s_rxtr1
    {
    U16_t       id;                   /* Index of neighbor */
    U8_t        bond;                 /* Multiplicity of bond */
    U8_t        stereo;               /* Direction of each bond */
    } neighbors[MX_NEIGHBORS];
  } XtrRow_t;
#define XTRROWSIZE sizeof (XtrRow_t)

/* Flag mask values */

#define XtrM_DontCare        0x1
#define XtrM_Asymmetry       0x2
#define XtrM_Dot             0x4
#define XtrM_ResonantBonds   0x8

/** Field Access Macros for a XtrRow_t **/

/* Macro Prototypes
   Boolean_t Xtr_AttrFlags_Asymmetry_Get (Xtr_t *, U16_t);
   void      Xtr_AttrFlags_Asymmetry_Put (Xtr_t *, U16_t, Boolean_t);
   Boolean_t Xtr_AttrFlags_DontCare_Get  (Xtr_t *, U16_t);
   void      Xtr_AttrFlags_DontCare_Put  (Xtr_t *, U16_t, Boolean_t);
   Boolean_t Xtr_AttrFlags_Dot_Get       (Xtr_t *, U16_t);
   void      Xtr_AttrFlags_Dot_Put       (Xtr_t *, U16_t, Boolean_t);
   Boolean_t Xtr_AttrFlags_ResonantBonds_Get (Xtr_t *, U16_t);
   void      Xtr_AttrFlags_ResonantBonds_Put (Xtr_t *, U16_t, Boolean_t);
   U16_t     Xtr_Attr_Atomid_Get         (Xtr_t *, U16_t);
   void      Xtr_Attr_Atomid_Put         (Xtr_t *, U16_t, U16_t);
   Boolean_t Xtr_Attr_Flags_Get          (Xtr_t *, U16_t);
   void      Xtr_Attr_Flags_Put          (Xtr_t *, U16_t, U16_t);
   U8_t      Xtr_Attr_NumNeighbors_Get   (Xtr_t *, U16_t);
   void      Xtr_Attr_NumNeighbors_Put   (Xtr_t *, U16_t, U8_t);
   U16_t     Xtr_Attr_NeighborId_Get     (Xtr_t *, U16_t, U8_t);
   void      Xtr_Attr_NeighborId_Put     (Xtr_t *, U16_t, U8_t, U16_t);
   U8_t      Xtr_Attr_NeighborBond_Get   (Xtr_t *, U16_t, U8_t);
   void      Xtr_Attr_NeighborBond_Put   (Xtr_t *, U16_t, U8_t, U8_t);
   U8_t      Xtr_Attr_NeighborStereo_Get (Xtr_t *, U16_t, U8_t);
   void      Xtr_Attr_NeighborStereo_Put (Xtr_t *, U16_t, U8_t, U8_t);
*/

#ifndef XTR_DEBUG
/* Used to be asymtry: */

#define Xtr_AttrFlags_Asymmetry_Get(xtr_p, atom)\
  ((xtr_p)->attributes[atom].flags & XtrM_Asymmetry ? TRUE : FALSE)

#define Xtr_AttrFlags_Asymmetry_Put(xtr_p, atom, value)\
  if ((value) == TRUE)\
    (xtr_p)->attributes[atom].flags |= XtrM_Asymmetry;\
  else\
    (xtr_p)->attributes[atom].flags &= ~XtrM_Asymmetry

/* Used to be dntcare: */

#define Xtr_AttrFlags_DontCare_Get(xtr_p, atom)\
  ((xtr_p)->attributes[atom].flags & XtrM_DontCare ? TRUE : FALSE)

#define Xtr_AttrFlags_DontCare_Put(xtr_p, atom, value)\
  { if ((value) == TRUE)\
    (xtr_p)->attributes[atom].flags |= XtrM_DontCare;\
  else\
    (xtr_p)->attributes[atom].flags &= ~XtrM_DontCare; }

/* Used to be getdot: */

#define Xtr_AttrFlags_Dot_Get(xtr_p, atom)\
  ((xtr_p)->attributes[atom].flags & XtrM_Dot ? TRUE : FALSE)

#define Xtr_AttrFlags_Dot_Put(xtr_p, atom, value)\
  { if ((value) == TRUE)\
    (xtr_p)->attributes[atom].flags |= XtrM_Dot;\
  else\
    (xtr_p)->attributes[atom].flags &= ~XtrM_Dot; }

#define Xtr_AttrFlags_ResonantBonds_Get(xtr_p, atom)\
  ((xtr_p)->attributes[atom].flags & XtrM_ResonantBonds ? TRUE : FALSE)

#define Xtr_AttrFlags_ResonantBonds_Put(xtr_p, atom, value)\
  { if ((value) == TRUE)\
    (xtr_p)->attributes[atom].flags |= XtrM_ResonantBonds;\
  else\
    (xtr_p)->attributes[atom].flags &= ~XtrM_ResonantBonds; }

/* Used to be atom_id: */

#define Xtr_Attr_Atomid_Get(xtr_p, atom)\
  ((atom) == XTR_INVALID ? XTR_INVALID : (xtr_p)->attributes[atom].atomid)

/* Used to be $atm_id: */

#define Xtr_Attr_Atomid_Put(xtr_p, atom, value)\
  (xtr_p)->attributes[atom].atomid = (value)

#define Xtr_Attr_Flags_Get(xtr_p, atom)\
  (xtr_p)->attributes[atom].flags

#define Xtr_Attr_Flags_Put(xtr_p, atom, value)\
  (xtr_p)->attributes[atom].flags = (value)

/* Used to be degree: */

#define Xtr_Attr_NumNeighbors_Get(xtr_p, atom)\
  (xtr_p)->attributes[atom].num_neighbors

/* Used to be $degree: */

#define Xtr_Attr_NumNeighbors_Put(xtr_p, atom, value)\
  (xtr_p)->attributes[atom].num_neighbors = (value)

/* Used to be neighbr: */

#define Xtr_Attr_NeighborId_Get(xtr_p, atom, adj)\
  (xtr_p)->attributes[atom].neighbors[adj].id

/* Used to be $nghbr: */

#define Xtr_Attr_NeighborId_Put(xtr_p, atom, adj, value)\
  (xtr_p)->attributes[atom].neighbors[adj].id = (value)

/* Used to be bxmltr: && bxmult: (compute resonant bonds 1st) */
 
#define Xtr_Attr_NeighborBond_Get(xtr_p, atom, adj)\
  (xtr_p)->attributes[atom].neighbors[adj].bond

/* Used to be $bxmult: */

#define Xtr_Attr_NeighborBond_Put(xtr_p, atom, adj, value)\
  (xtr_p)->attributes[atom].neighbors[adj].bond = (value)

/* Used to be bxdrct: && rlocx: */

#define Xtr_Attr_NeighborStereo_Get(xtr_p, atom, adj)\
  (xtr_p)->attributes[atom].neighbors[adj].stereo

/* Used to be $stereo: */

#define Xtr_Attr_NeighborStereo_Put(xtr_p, atom, adj, value)\
  (xtr_p)->attributes[atom].neighbors[adj].stereo = (value)

#else
/** Debug versions of field access macros **/

/* Used to be asymtry: */

#define Xtr_AttrFlags_Asymmetry_Get(xtr_p, atom)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ? HALT :\
  (xtr_p)->attributes[atom].flags & XtrM_Asymmetry ? TRUE : FALSE)

#define Xtr_AttrFlags_Asymmetry_Put(xtr_p, atom, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms) HALT;\
  else if ((value) != FALSE) (xtr_p)->attributes[atom].flags |=\
  XtrM_Asymmetry; else (xtr_p)->attributes[atom].flags &= ~XtrM_Asymmetry; }

/* Used to be dntcare: */

#define Xtr_AttrFlags_DontCare_Get(xtr_p, atom)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ? HALT :\
  (xtr_p)->attributes[atom].flags & XtrM_DontCare ? TRUE : FALSE)

#define Xtr_AttrFlags_DontCare_Put(xtr_p, atom, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms) HALT;\
  else if ((value) != FALSE) (xtr_p)->attributes[atom].flags |= XtrM_DontCare;\
  else (xtr_p)->attributes[atom].flags &= ~XtrM_DontCare; }

/* Used to be getdot: */

#define Xtr_AttrFlags_Dot_Get(xtr_p, atom)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ? HALT :\
  (xtr_p)->attributes[atom].flags & XtrM_Dot ? TRUE : FALSE)

#define Xtr_AttrFlags_Dot_Put(xtr_p, atom, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms) HALT;\
  else if ((value) != FALSE) (xtr_p)->attributes[atom].flags |= XtrM_Dot;\
  else (xtr_p)->attributes[atom].flags &= ~XtrM_Dot; }

#define Xtr_AttrFlags_ResonantBonds_Get(xtr_p, atom)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ? HALT :\
  (xtr_p)->attributes[atom].flags & XtrM_ResonantBonds ? TRUE : FALSE)

#define Xtr_AttrFlags_ResonantBonds_Put(xtr_p, atom, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms) HALT;\
  else if ((value) != FALSE) (xtr_p)->attributes[atom].flags |=\
  XtrM_ResonantBonds; else (xtr_p)->attributes[atom].flags &=\
  ~XtrM_ResonantBonds; }

/* Used to be atom_id: */  /* Used to be XTR_INVALID instead of HALT */

#define Xtr_Attr_Atomid_Get(xtr_p, atom)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ? HALT :\
  (xtr_p)->attributes[atom].atomid)

/* Used to be $atm_id: */

#define Xtr_Attr_Atomid_Put(xtr_p, atom, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms) HALT;\
  else (xtr_p)->attributes[atom].atomid = (value); }

#define Xtr_Attr_Flags_Get(xtr_p, atom)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ? HALT :\
  (xtr_p)->attributes[atom].flags)

#define Xtr_Attr_Flags_Put(xtr_p, atom, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms) HALT;\
  else (xtr_p)->attributes[atom].flags = (value); }

/* Used to be degree: */

#define Xtr_Attr_NumNeighbors_Get(xtr_p, atom)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ? HALT :\
  (xtr_p)->attributes[atom].num_neighbors)

/* Used to be $degree: */

#define Xtr_Attr_NumNeighbors_Put(xtr_p, atom, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms) HALT;\
  else (xtr_p)->attributes[atom].num_neighbors = (value); }

/* Used to be neighbr: */

#define Xtr_Attr_NeighborId_Get(xtr_p, atom, adj)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms || (adj) < 0\
  || (adj) >= MX_NEIGHBORS ? HALT :\
  (xtr_p)->attributes[atom].neighbors[adj].id)

/* Used to be $nghbr: */

#define Xtr_Attr_NeighborId_Put(xtr_p, atom, adj, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ||\
  (adj) < 0 || (adj) >= MX_NEIGHBORS) HALT; else\
  (xtr_p)->attributes[atom].neighbors[adj].id = (value); }

/* Used to be bxmltr: && bxmult: (compute resonant bonds 1st) */
 
#define Xtr_Attr_NeighborBond_Get(xtr_p, atom, adj)\
  ((atom) == XTR_INVALID || (adj) == XTR_INVALID ? HALT :\
  (xtr_p)->attributes[atom].neighbors[adj].bond)
 

/* Used to be $bxmult: */

#define Xtr_Attr_NeighborBond_Put(xtr_p, atom, adj, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ||\
  (adj) < 0 || (adj) >= MX_NEIGHBORS) HALT; else\
  (xtr_p)->attributes[atom].neighbors[adj].bond = (value); }

/* Used to be bxdrct: && rlocx: */

#define Xtr_Attr_NeighborStereo_Get(xtr_p, atom, adj)\
  ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms || (adj) < 0\
  || (adj) >= MX_NEIGHBORS ? HALT :\
  (xtr_p)->attributes[atom].neighbors[adj].stereo)

/* Used to be $stereo: */

#define Xtr_Attr_NeighborStereo_Put(xtr_p, atom, adj, value)\
  { if ((xtr_p) < GBAddr || (atom) < 0 || (atom) >= (xtr_p)->num_atoms ||\
  (adj) < 0 || (adj) >= MX_NEIGHBORS) HALT; else\
  (xtr_p)->attributes[atom].neighbors[adj].stereo = (value); }
#endif

/** End of Field Access macros for a XtrRow_t **/

/* The XTR data-structure itself.  Note, resonant bonds are not computed 
   by the called routine/macro, so whichever routine creates the XTR should
   decide whether to call Xtr_ResonantBonds_Set.

   NOTE: struct name used to avoid circular include problem.
*/

typedef struct s_xtr
  {
  Aromatics_t  *aromatics;                 /* Aromatics structure */
  Atoms_t      *atoms;                     /* Atoms structure */
  XtrRow_t     *attributes;                /* Atom attributes */
  FuncGroups_t *functionalgroups;          /* Functional Group structure */
  Name_t       *name;                      /* Xtr canonical name */
  Rings_t      *rings;                     /* Rings structure */
  U16_t         num_atoms;                 /* Number of atoms in the XTR */
  } Xtr_t;
#define XTRSIZE sizeof (Xtr_t)

/** Field Access Macros for a Xtr_t **/

/* Macro Prototypes
   Aromatics_t  *Xtr_Aromatics_Get  (Xtr_t *);
   void          Xtr_Aromatics_Put  (Xtr_t *, Aromatics_t *);
   Atoms_t      *Xtr_Atoms_Get      (Xtr_t *);
   void          Xtr_Atoms_Put      (Xtr_t *, Atoms_t *);
   XtrRow_t     *Xtr_Attributes_Get (Xtr_t *);
   void          Xtr_Attributes_Put (Xtr_t *, XtrRow_t *);
   FuncGroups_t *Xtr_FuncGroups_Get (Xtr_t *);
   void          Xtr_FuncGroups_Put (Xtr_t *, FuncGroups_t *);
   Name_t       *Xtr_Name_Get       (Xtr_t *);
   void          Xtr_Name_Put       (Xtr_t *, Name_t *);
   U16_t         Xtr_NumAtoms_Get   (Xtr_t *);
   void          Xtr_NumAtoms_Put   (Xtr_t *, U16_t);
   Rings_t      *Xtr_Rings_Get      (Xtr_t *);
   void          Xtr_Rings_Put      (Xtr_t *, Rings_t *);
*/

#ifndef XTR_DEBUG
/* Used to be aaromtc: */

#define Xtr_Aromatics_Get(xtr_p)\
  (xtr_p)->aromatics

/* Used to be $aromtc: */

#define Xtr_Aromatics_Put(xtr_p, value)\
  (xtr_p)->aromatics = (value)

/* Used to be aatoms: */

#define Xtr_Atoms_Get(xtr_p)\
  (xtr_p)->atoms

/* Used to be atoms: */

#define Xtr_Atoms_Put(xtr_p, value)\
  (xtr_p)->atoms = (value)

#define Xtr_Attributes_Get(xtr_p)\
  (xtr_p)->attributes

#define Xtr_Attributes_Put(xtr_p, value)\
  (xtr_p)->attributes = (value)

/* Used to be afgroup: */

#define Xtr_FuncGroups_Get(xtr_p)\
  (xtr_p)->functionalgroups

/* Used to be $fgroup: */

#define Xtr_FuncGroups_Put(xtr_p, value)\
  (xtr_p)->functionalgroups = (value)

/* Used to be aname: */

#define Xtr_Name_Get(xtr_p)\
  (xtr_p)->name

/* Used to be $name: */

#define Xtr_Name_Put(xtr_p, value)\
  (xtr_p)->name = (value)

/* Used to be nmatoms: */

#define Xtr_NumAtoms_Get(xtr_p)\
  (xtr_p)->num_atoms

#define Xtr_NumAtoms_Put(xtr_p, value)\
  (xtr_p)->num_atoms = (value)

/* Used to be arings: */

#define Xtr_Rings_Get(xtr_p)\
  (xtr_p)->rings

/* Used to be $rings: */

#define Xtr_Rings_Put(xtr_p, value)\
  (xtr_p)->rings = (value)
#else
/* Used to be aaromtc: */

#define Xtr_Aromatics_Get(xtr_p)\
  ((xtr_p) < GBAddr ? HALT : (xtr_p)->aromatics)

/* Used to be $aromtc: */

#define Xtr_Aromatics_Put(xtr_p, value)\
  { if ((xtr_p) < GBAddr) HALT; else (xtr_p)->aromatics = (value); }

/* Used to be aatoms: */

#define Xtr_Atoms_Get(xtr_p)\
  ((xtr_p) < GBAddr ? HALT : (xtr_p)->atoms)

/* Used to be atoms: */

#define Xtr_Atoms_Put(xtr_p, value)\
  { if ((xtr_p) < GBAddr) HALT; else (xtr_p)->atoms = (value); }

#define Xtr_Attributes_Get(xtr_p)\
  ((xtr_p) < GBAddr ? HALT : (xtr_p)->attributes)

#define Xtr_Attributes_Put(xtr_p, value)\
  { if ((xtr_p) < GBAddr) HALT; else (xtr_p)->attributes = (value); }

/* Used to be afgroup: */

#define Xtr_FuncGroups_Get(xtr_p)\
  ((xtr_p) < GBAddr ? HALT : (xtr_p)->functionalgroups)

/* Used to be $fgroup: */

#define Xtr_FuncGroups_Put(xtr_p, value)\
  { if ((xtr_p) < GBAddr) HALT; else (xtr_p)->functionalgroups = (value); }

/* Used to be aname: */

#define Xtr_Name_Get(xtr_p)\
  ((xtr_p) < GBAddr ? HALT : (xtr_p)->name)

/* Used to be $name: */

#define Xtr_Name_Put(xtr_p, value)\
  { if ((xtr_p) < GBAddr) HALT; else (xtr_p)->name = (value); }

/* Used to be nmatoms: */

#define Xtr_NumAtoms_Get(xtr_p)\
  ((xtr_p) < GBAddr ? HALT : (xtr_p)->num_atoms)

#define Xtr_NumAtoms_Put(xtr_p, value)\
  { if ((xtr_p) < GBAddr) HALT; else (xtr_p)->num_atoms = (value); }

/* Used to be arings: */

#define Xtr_Rings_Get(xtr_p)\
  ((xtr_p) < GBAddr ? HALT : (xtr_p)->rings)

/* Used to be $rings: */

#define Xtr_Rings_Put(xtr_p, value)\
  { if ((xtr_p) < GBAddr) HALT; else (xtr_p)->rings = (value); }
#endif

/** End of Field Access Macros for Xtr_t **/

/*** Macros ***/

/* Macro Prototypes
   U16_t Xtr_FuncGrp_AnyInstance_Check    (Xtr_t *, U16_t);
   U16_t Xtr_FuncGrp_NumInstances_Get     (Xtr_t *, U16_t);
   U16_t Xtr_FuncGrp_NumSubstructures_Get (Xtr_t *);
   void  Xtr_FuncGrp_PreservableBonds_Set (Xtr_t *, Array_t *);
   U16_t Xtr_FuncGrp_SubstructureInstance_Get (Xtr_t *, U16_t, U16_t);
   U16_t Xtr_Rings_NumRingSys_Get         (Xtr_t *);
*/

#ifndef XTR_DEBUG
/* Used to be:
   anyinst: mainent=true, pres_ent = false
   anyinsp: mainent=false, pres_ent = true
   subanyi: mainent=false, pres_ent = false
*/

#define Xtr_FuncGrp_AnyInstance_Check(xtr_p, fgnum)\
  (FuncGroups_Substructure_Exists (Xtr_FuncGroups_Get (xtr_p), fgnum) != 0 ?\
  FuncGrp_SubstructureInstance_Get (Xtr_FuncGroups_Get (xtr_p), 1, fgnum) :\
  XTR_INVALID)

/* Used to be:
   nmsub: mainent=true, pres_ent=false
   nmsubp: mainent=false, pres_ent=true
   subnsub: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_NumInstances_Get(xtr_p, fgnum)\
  FuncGrp_SubstructureCount_Get (Xtr_FuncGroups_Get (xtr_p), fgnum)

/* Used to be:
   nmsub: mainent=true, pres_ent=false
   nmsubp: mainent=false, pres_ent=true
   subnsub: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_NumSubstructures_Get(xtr_p)\
  FuncGrp_NumSubstructures_Get (Xtr_FuncGroups_Get (xtr_p))

/* Used to be:
   prsvbnd: mainent=true, pres_ent=false
   prsbndp: mainent=false, pres_ent=true
   subpbnd: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_PreservableBonds_Set(xtr_p, bonds_p)\
  { if (xtr_p->functionalgroups == NULL) xtr_p->functionalgroups =\
  FuncGroups_Create (xtr_p);\
  Array_CopyContents (FuncGrp_Preservable_Get (Xtr_FuncGroups_Get (xtr_p)),\
  bonds_p);}

/* Used to be:
   instloc: mainent=true, pres_ent=false
   inslocp: mainent=false, pres_ent=true
   subiloc: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_SubstructureInstance_Get(xtr_p, fgnum, inst)\
  FuncGrp_SubstructureInstance_Get (Xtr_FuncGroups_Get (xtr_p), fgnum, inst)

#else
/* Debug versions of macros */

/* Used to be:
   anyinst: mainent=true, pres_ent = false
   anyinsp: mainent=false, pres_ent = true
   subanyi: mainent=false, pres_ent = false
*/

#define Xtr_FuncGrp_AnyInstance_Check(xtr_p, fgnum)\
  (FuncGroups_Substructure_Exists (Xtr_FuncGroups_Get (xtr_p), fgnum) != 0 ?\
  FuncGrp_SubstructureInstance_Get (Xtr_FuncGroups_Get (xtr_p), 1, fgnum) :\
  HALT)

/* Used to be:
   nmsub: mainent=true, pres_ent=false
   nmsubp: mainent=false, pres_ent=true
   subnsub: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_NumInstances_Get(xtr_p, fgnum)\
  FuncGrp_SubstructureCount_Get (Xtr_FuncGroups_Get (xtr_p), fgnum)

/* Used to be:
   nmsub: mainent=true, pres_ent=false
   nmsubp: mainent=false, pres_ent=true
   subnsub: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_NumSubstructures_Get(xtr_p)\
  FuncGrp_NumSubstructures_Get (Xtr_FuncGroups_Get (xtr_p))

/* Used to be:
   prsvbnd: mainent=true, pres_ent=false
   prsbndp: mainent=false, pres_ent=true
   subpbnd: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_PreservableBonds_Set(xtr_p, bonds_p)\
  { if (xtr_p->functionalgroups == NULL) xtr_p->functionalgroups =\
  FuncGroups_Create (xtr_p);\
  Array_CopyContents (FuncGrp_Preservable_Get (Xtr_FuncGroups_Get (xtr_p)),\
  bonds_p);}

/* Used to be:
   instloc: mainent=true, pres_ent=false
   inslocp: mainent=false, pres_ent=true
   subiloc: mainent=false, pres_ent=false
*/

#define Xtr_FuncGrp_SubstructureInstance_Get(xtr_p, fgnum, inst)\
  FuncGrp_SubstructureInstance_Get (Xtr_FuncGroups_Get (xtr_p), fgnum, inst)

#endif

/*** Routine Prototypes ***/

void          Xtr_Attr_Atoms_Link          (Xtr_t *, U16_t, U16_t, U8_t);
U8_t          Xtr_Attr_NeighborBond_Find   (Xtr_t *, U16_t, U16_t);
U8_t          Xtr_Attr_NeighborIndex_Find  (Xtr_t *, U16_t, U16_t);
U8_t          Xtr_Attr_NeighborStereo_Find (Xtr_t *, U16_t, U16_t);
U16_t         Xtr_Attr_NextNonHydrogen_Get (Xtr_t *, U16_t, U8_t);
U8_t          Xtr_Attr_NumNonHydrogen_Get  (Xtr_t *, U16_t);
U16_t	      Xtr_Attr_NonHydrogenNeighbor_Get (Xtr_t *, U16_t, U16_t);
void          Xtr_Attr_ResonantBonds_Set   (Xtr_t *);
void          Xtr_CompressRow              (Xtr_t *, U16_t);
U16_t         Xtr_ConnectedSize_Find       (Xtr_t *, U16_t);
Xtr_t        *Xtr_Copy                     (Xtr_t *);
Xtr_t        *Xtr_CopyExpand               (Xtr_t *, U16_t);
Xtr_t        *Xtr_CopySubset_Atom          (Xtr_t *, Array_t *, Array_t *);
Xtr_t        *Xtr_CopySubset               (Xtr_t *, Array_t *, Array_t *);
Xtr_t        *Xtr_Create                   (U16_t);
void          Xtr_Destroy                  (Xtr_t *);
void          Xtr_Dump                     (Xtr_t *, FileDsc_t *);
void	      Xtr_FuncGroups_Destroy       (Xtr_t *);
U16_t         Xtr_Rings_NumRingSys_Get     (Xtr_t *);
void          Xtr_Rings_Set                (Xtr_t *);
void          Xtr_Ringdef_Set              (Xtr_t *, U16_t);

/* AROMATICS.C */

Boolean_t     Xtr_Aromat_Bond_Get 	   (Xtr_t *, U16_t, U16_t);
Boolean_t     Xtr_Aromat_Node_Get 	   (Xtr_t *, U16_t);
void          Xtr_Aromat_FindBonds         (Xtr_t *, Array_t *);
void          Xtr_Aromat_Set               (Xtr_t *);
void          Aromat_FindBonds             (Xtr_t *, Array_t *);

/* ATOMS.C */

U16_t Atoms_Count_Find                     (Xtr_t *, U16_t);
U16_t Atoms_InstanceIndex_Get              (Xtr_t *, U16_t, U16_t);
void  Xtr_Atoms_Set                        (Xtr_t *);

/* CHEMISTRY.C */

Boolean_t Xtr_FuncGrp_Instance_IsValid	   (Xtr_t *, U16_t, U16_t);


/* FUNCGROUPS.C */

FuncGroups_t *FuncGroups_Create            (Xtr_t *);
Boolean_t     Xtr_FuncGrp_Instance_IsDiff  (Xtr_t *, U16_t, U16_t);
Boolean_t     Xtr_FuncGrps_Equiv           (Xtr_t *, Xtr_t *);

/* NAME.C */

Boolean_t Name_ConstitutionalEquivilance_Get (Xtr_t *, U16_t , U16_t);
Name_t   *Name_Create                      (U16_t, Sling_t);
Sling_t   Name_Sling_Get                   (Xtr_t *, Boolean_t);
Sling_t   Name_SlingMap_Get                (Xtr_t *, Boolean_t, Array_t *);
Boolean_t Name_StereochemicalEquivilance_Get (Xtr_t *, U16_t, U16_t);
void      Xtr_Name_Set                     (Xtr_t *, Array_t *);

/* REACTION.C */

Boolean_t React_Schema_IsOk                  (Xtr_t *, U32_t);

/* RESONANTBONDS.C */

void          ResonantBonds_Find           (Xtr_t *, Array_t *);
void          ResonantBonds_FindSpiro      (Xtr_t *, Array_t *);
void          ResonantBonds_Fix            (Xtr_t *, U16_t, U8_t, U8_t);

/* RINGDEFINITION.C */

Boolean_t     Ringdef_Carbocyclic_Find     (Xtr_t *, U16_t, U16_t);
void          Ringdef_CommonRing_Find      (Xtr_t *, U16_t, U16_t, U16_t,
  Array_t *);
void          Ringdef_CycleMember_Set      (Xtr_t *, U16_t, U16_t, Array_t *);
U16_t         Ringdef_CycleSize_Find       (Xtr_t *, U16_t, U16_t);
void          Ringdef_MinPrimaryRing_Find  (Xtr_t *, U16_t, U16_t, U16_t,
  U16_t *, U16_t *);
void          Ringdef_Min4PrimaryRing_Find (Xtr_t *, U16_t, U16_t, U16_t,
  U16_t, U16_t, U16_t *, U16_t *);
U16_t         Ringdef_NumPrimaryCycles_Find (Xtr_t *, U16_t);
void          Ringdef_PrimaryRingNodes_Get (Xtr_t *, U16_t, U16_t, Array_t *);
Ringdef_t    *Ringdef_PrimeCycles_Find     (Xtr_t *);

/* RINGS.C */

Boolean_t     Ring_AtomIn                  (Xtr_t *, U16_t);
Boolean_t     Ring_AtomInSpecific          (Xtr_t *, U16_t, U16_t);
void          Ring_Bonds_Set               (Xtr_t *, U16_t, Array_t *);
void          Ring_Systems_Find            (Xtr_t *, Rings_t *, Ringsys_t **,
  U16_t *);

/* RINGSYSTEMS.C */

Ringdef_t    *Ringsys_Ringdef_Find         (Xtr_t *, U16_t);
void          Ringsys_Ringdef_Insert       (Xtr_t *, U16_t, Ringdef_t *);

/*** Global Variables ***/

/* End of Xtr.H */
#endif
