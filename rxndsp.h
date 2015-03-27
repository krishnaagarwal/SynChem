#ifndef _H_RXNDSP_
#define _H_RXNDSP_

/******************************************************************************
*
*  Copyright (C) 1999, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RXNDSP.H
*
*    This module contains the definions of atoms, bonds and molecules
*    for displaying purposes (in DSP.H), and extends the bond definitions
*    to mask in reaction pattern-dependent values of bond order and
*    stereochemistry, as well as atom and bond selection information.
*    
*  Creation Date:
*
*    24-Feb-1999
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita	xxx
*
******************************************************************************/

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#define MAXROOTS /* 4 */ MX_ROOTS
#define MX_NAMES_PER_FG 10

#define NEITHER -1
#define GOAL 0
#define SUBG 1
#define BOTH 2

#define LOWORD_MASK (unsigned)0xffff
#define HIWORD_MASK (unsigned)0xffff0000

#define LONYBL_MASK (U8_t)0xf
#define HINYBL_MASK (U8_t)0xf0

typedef struct root_syn_s
  {
  int x;
  int y;
  int syntheme;
  } Root_Syn_t;

typedef struct rxndsp_molecule_s
  {
  Dsp_Molecule_t  both_dm;
  Dsp_Molecule_t *goal_dm_p;
  Dsp_Molecule_t *subgoal_dm_p;
  Root_Syn_t      root_syn[MAXROOTS];
  int             num_roots;
  } RxnDsp_Molecule_t;
#define RXNDSP_MOLECULE_SIZE (sizeof (RxnDsp_Molecule_t))

typedef Dsp_Atom_t RxnDsp_Atom_t;
#define RXNDSP_ATOM_SIZE DSP_ATOM_SIZE

typedef Dsp_Bond_t RxnDsp_Bond_t;
#define RXNDSP_BOND_SIZE DSP_BOND_SIZE

/*****************************************************************
Macro prototypes for rxndsp.h extensions to dsp.h

RxnDsp_Atom_t *RxnDsp_AtomThere_Get  (RxnDsp_Molecule_t *, int, int, int);

RxnDsp_Atom_t *RxnDsp_AtomIntrfr_Get (RxnDsp_Molecule_t *, int, int, int, int);

RxnDsp_Bond_t *RxnDsp_BondThere_Get  (RxnDsp_Molecule_t *, int, int, int,
                                      int, int, int);

RxnDsp_Atom_t *RxnDsp_AtomPtr_Get    (RxnDsp_Molecule_t *, int);
void           RxnDsp_AtomPtr_Put    (RxnDsp_Molecule_t *, int,
                                      RxnDsp_Atom_t *);

RxnDsp_Bond_t *RxnDsp_BondPtr_Get    (RxnDsp_Molecule_t *, int);
void           RxnDsp_BondPtr_Put    (RxnDsp_Molecule_t *, int,
                                      RxnDsp_Bond_t *);

RxnDsp_Atom_t *RxnDsp_RxncPtr_Get    (RxnDsp_Molecule_t *);
void           RxnDsp_RxncPtr_Put    (RxnDsp_Molecule_t *, RxnDsp_Atom_t *);

Root_Syn_t    *RxnDsp_RootPtr_Get    (RxnDsp_Molecule_t *, int);

int            RxnDsp_RootCnt_Get    (RxnDsp_Molecule_t *);
void           RxnDsp_RootCnt_Put    (RxnDsp_Molecule_t *, int);

Boolean_t      RxnDsp_AtomIsRoot_Get (RxnDsp_Molecule_t *, int, int);

char           RxnDsp_AtomCharge_Get (RxnDsp_Atom_t *, int);
void           RxnDsp_AtomCharge_Put (RxnDsp_Atom_t *, int, char );

int            RxnDsp_AtomSelect_Get (RxnDsp_Atom_t *, int);
void           RxnDsp_AtomSelect_Put (RxnDsp_Atom_t *, int, int);

int            RxnDsp_AtomXCoord_Get (RxnDsp_Atom_t *, int);
void           RxnDsp_AtomXCoord_Put (RxnDsp_Atom_t *, int, int);

int            RxnDsp_AtomYCoord_Get (RxnDsp_Atom_t *, int);
void           RxnDsp_AtomYCoord_Put (RxnDsp_Atom_t *, int, int);

int            RxnDsp_BondSelect_Get (RxnDsp_Bond_t *, int);
void           RxnDsp_BondSelect_Put (RxnDsp_Bond_t *, int, int);

U8_t           RxnDsp_BondOrder_Get  (RxnDsp_Bond_t *, int);
void           RxnDsp_BondOrder_Put  (RxnDsp_Bond_t *, int, U8_t);

U8_t           RxnDsp_BondStereo_Get (RxnDsp_Bond_t *, int);
void           RxnDsp_BondStereo_Put (RxnDsp_Bond_t *, int, U8_t);
*****************************************************************/
#define RxnDsp_AtomThere_Get(rxn_p, pat, x, y) \
  (RxnDsp_Atom_t *) rxnget_Atom_There ((rxn_p), (pat), (x), (y), 0)

#define RxnDsp_AtomIntrfr_Get(rxn_p, pat, x, y, r) \
  (RxnDsp_Atom_t *) rxnget_Atom_There ((rxn_p), (pat), (x), (y), (r))

#define RxnDsp_BondThere_Get(rxn_p, pat1, x1, y1, pat2, x2, y2) \
  (RxnDsp_Bond_t *) rxnget_Bond_There ((rxn_p), (pat1), (x1), (y1), \
  (pat2), (x2), (y2))

#define RxnDsp_AtomPtr_Get(rxn_p, inx) \
  ((RxnDsp_Atom_t *) ((rxn_p)->both_dm.atoms + (inx)))
#define RxnDsp_AtomPtr_Put(rxn_p, inx, atom) \
  (rxn_p)->both_dm.atoms = (atom)

#define RxnDsp_BondPtr_Get(rxn_p, inx) \
  ((RxnDsp_Bond_t *) ((rxn_p)->both_dm.bonds + (inx)))
#define RxnDsp_BondPtr_Put(rxn_p, inx, bond) \
  (rxn_p)->both_dm.bonds = (bond)

#define RxnDsp_RxncPtr_Get(rxn_p) \
  ((RxnDsp_Atom_t *) ((rxn_p)->both_dm.rxncnr_p))
#define RxnDsp_RxncPtr_Put(rxn_p, rxnc) \
  (rxn_p)->both_dm.rxncnr_p = (rxnc)

#define RxnDsp_RootPtr_Get(rxn_p, inx) \
  ((Root_Syn_t *) ((rxn_p)->root_syn + (inx)))

#define RxnDsp_RootCnt_Get(rxn_p) \
  (rxn_p)->num_roots
#define RxnDsp_RootCnt_Put(rxn_p, count) \
  (rxn_p)->num_roots = (count)

#define RxnDsp_AtomIsRoot_Get(rxn_p, ainx, rinx) \
  (RxnDsp_AtomXCoord_Get (RxnDsp_AtomPtr_Get ((rxn_p), (ainx)), GOAL) == \
  RxnDsp_RootPtr_Get ((rxn_p), (rinx))->x && \
  RxnDsp_AtomYCoord_Get (RxnDsp_AtomPtr_Get ((rxn_p), (ainx)), GOAL) == \
  RxnDsp_RootPtr_Get ((rxn_p), (rinx))->y)

#define RxnDsp_AtomCharge_Get(atom_p, pat) \
  (atom_p)->chg[(pat)]
#define RxnDsp_AtomCharge_Put(atom_p, pat, achg) \
  (atom_p)->chg[(pat)] = (achg)

#define RxnDsp_AtomSelect_Get(atom_p, pat) \
  (int) (((atom_p)->isSelected & (LOWORD_MASK << (16 * (pat)))) >> (16 * (pat)))
#define RxnDsp_AtomSelect_Put(atom_p, pat, asel) \
  (atom_p)->isSelected = \
  ((atom_p)->isSelected & (HIWORD_MASK >> (16 * (pat)))) | ((asel) << (16 * (pat)))

#define RxnDsp_AtomXCoord_Get(atom_p, pat) \
  (int) (((atom_p)->x & (LOWORD_MASK << (16 * (pat)))) >> (16 * (pat)))
#define RxnDsp_AtomXCoord_Put(atom_p, pat, xc) \
  (atom_p)->x = \
  ((atom_p)->x & (HIWORD_MASK >> (16 * (pat)))) | ((xc) << (16 * (pat)))

#define RxnDsp_AtomYCoord_Get(atom_p, pat) \
  (int) (((atom_p)->y & (LOWORD_MASK << (16 * (pat)))) >> (16 * (pat)))
#define RxnDsp_AtomYCoord_Put(atom_p, pat, yc) \
  (atom_p)->y = \
  ((atom_p)->y & (HIWORD_MASK >> (16 * (pat)))) | ((yc) << (16 * (pat)))

#define RxnDsp_BondSelect_Get(bond_p, pat) \
  (int) (((bond_p)->isSelected & (LOWORD_MASK << (16 * (pat)))) >> (16 * (pat)))
#define RxnDsp_BondSelect_Put(bond_p, pat, bsel) \
  (bond_p)->isSelected = \
  ((bond_p)->isSelected & (HIWORD_MASK >> (16 * (pat)))) | ((bsel) << (16 * (pat)))

#define RxnDsp_BondOrder_Get(bond_p, pat) \
  (U8_t) (((bond_p)->nlines & (LONYBL_MASK << (4 * (pat)))) >> (4 * (pat)))
#define RxnDsp_BondOrder_Put(bond_p, pat, bord) \
  (bond_p)->nlines = \
  ((bond_p)->nlines & (HINYBL_MASK >> (4 * (pat)))) | ((bord) << (4 * (pat)))

#define RxnDsp_BondStereo_Get(bond_p, pat) \
  (U8_t) (((bond_p)->stereo & (LONYBL_MASK << (4 * (pat)))) >> (4 * (pat)))
#define RxnDsp_BondStereo_Put(bond_p, pat, bst) \
  (bond_p)->stereo = \
  ((bond_p)->stereo & (HINYBL_MASK >> (4 * (pat)))) | ((bst) << (4 * (pat)))

/* Function prototypes */
Boolean_t      rxn_delete_Atom (RxnDsp_Molecule_t *, RxnDsp_Atom_t *, int);
Boolean_t      rxn_delete_Bond (RxnDsp_Molecule_t *, RxnDsp_Bond_t *, int);
RxnDsp_Atom_t *rxn_store_Atom  (RxnDsp_Molecule_t *, char *, char *, char *,
                                int, int);
RxnDsp_Bond_t *rxn_store_Bond  (RxnDsp_Molecule_t *, int, int, int, int,
                                int, int);
void           copy_Reaction   (RxnDsp_Molecule_t *, RxnDsp_Molecule_t *);
void           free_Reaction   (RxnDsp_Molecule_t *);
Boolean_t      rxndsp_Shelley  (RxnDsp_Molecule_t *);
RxnDsp_Molecule_t *Xtr2RxnDsp  (Xtr_t *, Xtr_t *, U32_t *, U32_t *);
Dsp_Molecule_t *Xtr2Dsp_NoMessup (Xtr_t *);
void           dump_RxnDsp     (RxnDsp_Molecule_t *, char *);
void           dump_Dsp        (Dsp_Molecule_t *, char *);
RxnDsp_Atom_t *rxnget_Atom_There (RxnDsp_Molecule_t *, int, int, int, int);
RxnDsp_Bond_t *rxnget_Bond_There (RxnDsp_Molecule_t *, int, int, int,
                                  int, int, int);

#endif
