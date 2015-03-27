/******************************************************************************
*
*  Copyright (C) 1999, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RXNDSP.C
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
 
#include <X11/X.h>
#include <Xm/Xm.h>
 
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
 
#include "synchem.h"
#include "debug.h"
#include "synio.h"
 
#ifndef _H_RXNDSP_
#include "rxndsp.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

Boolean_t rxn_delete_Atom
  (
  RxnDsp_Molecule_t *rxndsp_p,
  RxnDsp_Atom_t     *rd_atom_p,
  int                which
)
{
  int            other, i;
  Boolean_t      has_nbrs;
  char           savechg[3];
  RxnDsp_Atom_t *rd_nbr_p;
  RxnDsp_Bond_t *rd_bond_p;

  if (which != BOTH)
    {
    other = which == GOAL ? SUBG : GOAL;
    for (i = 0, has_nbrs = FALSE;
      !has_nbrs && i < ((Dsp_Atom_t *) rd_atom_p)->adj_info.num_neighbs; i++)
      {
      rd_nbr_p = rxndsp_p->both_dm.atoms +
        ((Dsp_Atom_t *) rd_atom_p)->adj_info.adj_atms[i];
      rd_bond_p = RxnDsp_BondThere_Get (rxndsp_p, which,
        RxnDsp_AtomXCoord_Get (rd_atom_p, which),
        RxnDsp_AtomYCoord_Get (rd_atom_p, which), which,
        RxnDsp_AtomXCoord_Get (rd_nbr_p, which),
        RxnDsp_AtomYCoord_Get (rd_nbr_p, which));
      if (RxnDsp_BondOrder_Get (rd_bond_p, other) != 0)
        has_nbrs = TRUE;
      }
    if (has_nbrs) return (FALSE);
    }
  for (i = 0; i < 3; i++)
    savechg[i] = rxndsp_p->both_dm.atoms[rxndsp_p->both_dm.natms - 1].chg[i];
  delete_Atom (&rxndsp_p->both_dm, rd_atom_p);
  for (i = 0; i < 3; i++)
    rd_atom_p->chg[i] = savechg[i];
  return (TRUE);
}

Boolean_t rxn_delete_Bond
  (
  RxnDsp_Molecule_t *rxndsp_p,
  RxnDsp_Bond_t     *rd_bond_p,
  int                which
)
{
  int other;

  if (which != BOTH)
    {
    other = which == GOAL ? SUBG : GOAL;
    if (RxnDsp_BondOrder_Get (rd_bond_p, other) != 0)
      {
      RxnDsp_BondOrder_Put (rd_bond_p, which, 0);
      return (FALSE);
      }
    }
  delete_Bond (&rxndsp_p->both_dm, rd_bond_p);
  return (TRUE);
}

RxnDsp_Atom_t *rxn_store_Atom
  (
  RxnDsp_Molecule_t *rxndsp_p,
  char              *sym,
  char              *chg,
  char              *map,
  int                x,
  int                y
)
{
  char           savesgchg = chg[1];
  RxnDsp_Atom_t *rd_atom_p;

  rd_atom_p = (RxnDsp_Atom_t *) store_Atom (&rxndsp_p->both_dm, sym, chg, map,
    x, y);
  if (rd_atom_p != NULL)
    {
    RxnDsp_AtomCharge_Put (rd_atom_p, SUBG, savesgchg);
    RxnDsp_AtomSelect_Put (rd_atom_p, SUBG,
      RxnDsp_AtomSelect_Get (rd_atom_p, GOAL));
    RxnDsp_AtomXCoord_Put (rd_atom_p, SUBG,
      RxnDsp_AtomXCoord_Get (rd_atom_p, GOAL));
    RxnDsp_AtomYCoord_Put (rd_atom_p, SUBG,
      RxnDsp_AtomYCoord_Get (rd_atom_p, GOAL));
    }
  return (rd_atom_p);
}

RxnDsp_Bond_t *rxn_store_Bond
  (
  RxnDsp_Molecule_t *rxndsp_p,
  int                x1,
  int                y1,
  int                x2,
  int                y2,
  int                nglines,
  int                nsglines
)
{
  RxnDsp_Bond_t *rd_bond_p;

  rd_bond_p = (RxnDsp_Bond_t *) store_Bond (&rxndsp_p->both_dm, x1, y1, x2, y2,
    1);
  if (rd_bond_p != NULL)
    {
    RxnDsp_BondOrder_Put (rd_bond_p, GOAL, nglines);
    RxnDsp_BondOrder_Put (rd_bond_p, SUBG, nsglines);
    RxnDsp_BondSelect_Put (rd_bond_p, SUBG,
      RxnDsp_BondSelect_Get (rd_bond_p, GOAL));
    RxnDsp_BondStereo_Put (rd_bond_p, SUBG,
      RxnDsp_BondStereo_Get (rd_bond_p, GOAL));
    }
  return (rd_bond_p);
}

void copy_Reaction
  (
  RxnDsp_Molecule_t *dest_rxndsp_p,
  RxnDsp_Molecule_t *src_rxndsp_p
)
{
  char savesgchg[DSP_MAXNUM_ATMS];
  int  i;

  for (i = 0; i < src_rxndsp_p->both_dm.natms; i++)
    savesgchg[i] = RxnDsp_AtomCharge_Get (src_rxndsp_p->both_dm.atoms + i,
      SUBG);
  RxnDsp_RootCnt_Put (dest_rxndsp_p, RxnDsp_RootCnt_Get (src_rxndsp_p));
  for (i = 0; i < RxnDsp_RootCnt_Get (src_rxndsp_p); i++)
    {
    RxnDsp_RootPtr_Get (dest_rxndsp_p, i)->x =
      RxnDsp_RootPtr_Get (src_rxndsp_p, i)->x;
    RxnDsp_RootPtr_Get (dest_rxndsp_p, i)->y =
      RxnDsp_RootPtr_Get (src_rxndsp_p, i)->y;
    RxnDsp_RootPtr_Get (dest_rxndsp_p, i)->syntheme =
      RxnDsp_RootPtr_Get (src_rxndsp_p, i)->syntheme;
    }
  copy_Molecule (&dest_rxndsp_p->both_dm, &src_rxndsp_p->both_dm);
  for (i = 0; i < dest_rxndsp_p->both_dm.natms; i++)
    RxnDsp_AtomCharge_Put (src_rxndsp_p->both_dm.atoms + i, SUBG, savesgchg[i]);
  if (src_rxndsp_p->goal_dm_p != NULL && dest_rxndsp_p->goal_dm_p != NULL)
    copy_Molecule (dest_rxndsp_p->goal_dm_p, src_rxndsp_p->goal_dm_p);
  if (src_rxndsp_p->subgoal_dm_p != NULL && dest_rxndsp_p->subgoal_dm_p != NULL)
    copy_Molecule (dest_rxndsp_p->subgoal_dm_p, src_rxndsp_p->subgoal_dm_p);
}

void free_Reaction
  (
  RxnDsp_Molecule_t *rxndsp_p
)
{
  if (rxndsp_p->goal_dm_p != NULL)
    free_Molecule (rxndsp_p->goal_dm_p);
  if (rxndsp_p->subgoal_dm_p != NULL)
    free_Molecule (rxndsp_p->subgoal_dm_p);
  free (rxndsp_p->both_dm.atoms);
  free (rxndsp_p->both_dm.bonds);
  free (rxndsp_p);
}

RxnDsp_Atom_t *rxnget_Atom_There
  (
  RxnDsp_Molecule_t *mol_p,
  int pattern,
  int x,
  int y,
  int radius
)
{
  int            atm_i, dx, dy;
  RxnDsp_Atom_t *atm_p;

  atm_p = RxnDsp_AtomPtr_Get (mol_p, 0);
  for (atm_i = 0; atm_i < mol_p->both_dm.natms; atm_i++) 
    {
    if (radius == 0)
      {
      if ((x > (RxnDsp_AtomXCoord_Get (atm_p, pattern) - DRW_HILIT_AREA))
          &&
          (x < (RxnDsp_AtomXCoord_Get (atm_p, pattern) + DRW_HILIT_AREA)) 
          &&
	  (y > (RxnDsp_AtomYCoord_Get (atm_p, pattern) - DRW_HILIT_AREA))
          &&
          (y < (RxnDsp_AtomYCoord_Get (atm_p, pattern) + DRW_HILIT_AREA)))
        return (atm_p);
      }

    else
      {
      dx = x - RxnDsp_AtomXCoord_Get (atm_p, pattern);
      dy = y - RxnDsp_AtomYCoord_Get (atm_p, pattern);

      if (radius * radius > dx * dx + dy * dy)
        return (atm_p);
      }

    atm_p++;
    }

  return ((RxnDsp_Atom_t *) NULL);
}
  
RxnDsp_Bond_t *rxnget_Bond_There
  (
  RxnDsp_Molecule_t *mol_p,
  int pattern1,
  int x1,
  int y1,
  int pattern2,
  int x2,
  int y2
)
{
  int            bnd_i;
  RxnDsp_Bond_t *bnd_p;

  if (x1 > 0xff00 || y1 > 0xff00 || x2 > 0xff00 || y2 > 0xff00) return ((RxnDsp_Bond_t *) NULL);

  bnd_p = RxnDsp_BondPtr_Get (mol_p, 0);
  for (bnd_i = 0; bnd_i < mol_p->both_dm.nbnds; bnd_i++) 
    {
    if (((x1 > (RxnDsp_AtomXCoord_Get (bnd_p->latm_p, pattern1)
           - DRW_HILIT_AREA))
         && 
	 (x1 < (RxnDsp_AtomXCoord_Get (bnd_p->latm_p, pattern1)
           + DRW_HILIT_AREA))
         &&
	 (y1 > (RxnDsp_AtomYCoord_Get (bnd_p->latm_p, pattern1)
           - DRW_HILIT_AREA))
         && 
	 (y1 < (RxnDsp_AtomYCoord_Get (bnd_p->latm_p, pattern1)
           + DRW_HILIT_AREA))
         &&
	 (x2 > (RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, pattern2)
           - DRW_HILIT_AREA))
         && 
	 (x2 < (RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, pattern2)
           + DRW_HILIT_AREA))
         &&
	 (y2 > (RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, pattern2)
           - DRW_HILIT_AREA))
         && 
	 (y2 < (RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, pattern2)
           + DRW_HILIT_AREA)))
      ||
        ((x1 > (RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, pattern1)
           - DRW_HILIT_AREA))
         && 
	 (x1 < (RxnDsp_AtomXCoord_Get (bnd_p->ratm_p, pattern1)
           + DRW_HILIT_AREA))
         &&
	 (y1 > (RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, pattern1)
           - DRW_HILIT_AREA))
         && 
	 (y1 < (RxnDsp_AtomYCoord_Get (bnd_p->ratm_p, pattern1)
           + DRW_HILIT_AREA))
         &&
	 (x2 > (RxnDsp_AtomXCoord_Get (bnd_p->latm_p, pattern2)
           - DRW_HILIT_AREA))
         && 
	 (x2 < (RxnDsp_AtomXCoord_Get (bnd_p->latm_p, pattern2)
           + DRW_HILIT_AREA))
         &&
	 (y2 > (RxnDsp_AtomYCoord_Get (bnd_p->latm_p, pattern2)
           - DRW_HILIT_AREA))
         && 
	 (y2 < (RxnDsp_AtomYCoord_Get (bnd_p->latm_p, pattern2)
           + DRW_HILIT_AREA))))
      return (bnd_p);

    bnd_p++;
    }

  return ((RxnDsp_Bond_t *) NULL);
}
  
Boolean_t rxndsp_Shelley
  (
  RxnDsp_Molecule_t *rxndsp_p
)
{
  static char tmpchg[3] = {0, 0, 0};
  Boolean_t   goal_ok, subgoal_ok, root_found, isolate;
  int         i, j, k, root_inx, atom_inx, root_atom[MAXROOTS], num_isol[2],
              first_isol[2], last_isol[2], first_null_bond[2];

  if (rxndsp_p->goal_dm_p == NULL)
    {
    rxndsp_p->goal_dm_p = (Dsp_Molecule_t *) malloc (DSP_MOLECULE_SIZE);
    rxndsp_p->goal_dm_p->atoms =
      (Dsp_Atom_t *) malloc (DSP_ATOM_SIZE * rxndsp_p->both_dm.nallocatms);
    rxndsp_p->goal_dm_p->bonds =
      (Dsp_Bond_t *) malloc (DSP_BOND_SIZE * rxndsp_p->both_dm.nallocbnds);
    }
  else
    {
    if (rxndsp_p->goal_dm_p->nallocatms < rxndsp_p->both_dm.nallocatms)
      rxndsp_p->goal_dm_p->atoms =
        (Dsp_Atom_t *) realloc (rxndsp_p->goal_dm_p->atoms,
        DSP_ATOM_SIZE * rxndsp_p->both_dm.nallocatms);
    if (rxndsp_p->goal_dm_p->nallocbnds < rxndsp_p->both_dm.nallocbnds)
      rxndsp_p->goal_dm_p->bonds =
        (Dsp_Bond_t *) realloc (rxndsp_p->goal_dm_p->bonds,
        DSP_BOND_SIZE * rxndsp_p->both_dm.nallocbnds);
    }
  if (rxndsp_p->subgoal_dm_p == NULL)
    {
    rxndsp_p->subgoal_dm_p = (Dsp_Molecule_t *) malloc (DSP_MOLECULE_SIZE);
    rxndsp_p->subgoal_dm_p->atoms =
      (Dsp_Atom_t *) malloc (DSP_ATOM_SIZE * rxndsp_p->both_dm.nallocatms);
    rxndsp_p->subgoal_dm_p->bonds =
      (Dsp_Bond_t *) malloc (DSP_BOND_SIZE * rxndsp_p->both_dm.nallocbnds);
    }
  else
    {
    if (rxndsp_p->subgoal_dm_p->nallocatms < rxndsp_p->both_dm.nallocatms)
      rxndsp_p->subgoal_dm_p->atoms =
        (Dsp_Atom_t *) realloc (rxndsp_p->subgoal_dm_p->atoms,
        DSP_ATOM_SIZE * rxndsp_p->both_dm.nallocatms);
    if (rxndsp_p->subgoal_dm_p->nallocbnds < rxndsp_p->both_dm.nallocbnds)
      rxndsp_p->subgoal_dm_p->bonds =
        (Dsp_Bond_t *) realloc (rxndsp_p->subgoal_dm_p->bonds,
        DSP_BOND_SIZE * rxndsp_p->both_dm.nallocbnds);
    }

  copy_Molecule (rxndsp_p->goal_dm_p, &rxndsp_p->both_dm);
  copy_Molecule (rxndsp_p->subgoal_dm_p, &rxndsp_p->both_dm);

  for (i = 0; i < rxndsp_p->both_dm.natms; i++)
    {
    tmpchg[0] = RxnDsp_AtomCharge_Get (RxnDsp_AtomPtr_Get (rxndsp_p, i), GOAL);
    strcpy (rxndsp_p->goal_dm_p->atoms[i].chg, tmpchg);
    rxndsp_p->goal_dm_p->atoms[i].isSelected =
      RxnDsp_AtomSelect_Get (RxnDsp_AtomPtr_Get (rxndsp_p, i), GOAL);

    tmpchg[0] = RxnDsp_AtomCharge_Get (RxnDsp_AtomPtr_Get (rxndsp_p, i), SUBG);
    strcpy (rxndsp_p->subgoal_dm_p->atoms[i].chg, tmpchg);
    rxndsp_p->subgoal_dm_p->atoms[i].isSelected =
      RxnDsp_AtomSelect_Get (RxnDsp_AtomPtr_Get (rxndsp_p, i), SUBG);
    }

  for (i = 0; i < rxndsp_p->both_dm.nbnds; i++)
    {
    rxndsp_p->goal_dm_p->bonds[i].isSelected =
      RxnDsp_BondSelect_Get (RxnDsp_BondPtr_Get (rxndsp_p, i), GOAL);
    rxndsp_p->goal_dm_p->bonds[i].nlines = 
      RxnDsp_BondOrder_Get (RxnDsp_BondPtr_Get (rxndsp_p, i), GOAL);
    rxndsp_p->goal_dm_p->bonds[i].stereo =
      RxnDsp_BondStereo_Get (RxnDsp_BondPtr_Get (rxndsp_p, i), GOAL);

    rxndsp_p->subgoal_dm_p->bonds[i].isSelected =
      RxnDsp_BondSelect_Get (RxnDsp_BondPtr_Get (rxndsp_p, i), SUBG);
    rxndsp_p->subgoal_dm_p->bonds[i].nlines =
      RxnDsp_BondOrder_Get (RxnDsp_BondPtr_Get (rxndsp_p, i), SUBG);
    rxndsp_p->subgoal_dm_p->bonds[i].stereo =
      RxnDsp_BondStereo_Get (RxnDsp_BondPtr_Get (rxndsp_p, i), SUBG);
    }

  isolate = FALSE;

  for (i = 0; i < rxndsp_p->goal_dm_p->nbnds; )
    {
    if (rxndsp_p->goal_dm_p->bonds[i].nlines == 0)
      {
      /* Isolate goal and subgoal by eliminating null bonds and adjacencies */
      isolate = TRUE;
      for (j = 0;
           j < rxndsp_p->goal_dm_p->bonds[i].latm_p->adj_info.num_neighbs; )
        if (rxndsp_p->goal_dm_p->bonds[i].latm_p->adj_info.adj_atms[j] ==
            rxndsp_p->goal_dm_p->bonds[i].ratm_p - rxndsp_p->goal_dm_p->atoms)
        {
        rxndsp_p->goal_dm_p->bonds[i].latm_p->adj_info.num_neighbs--;
        for (k = j;
             k < rxndsp_p->goal_dm_p->bonds[i].latm_p->adj_info.num_neighbs;
             k++)
          rxndsp_p->goal_dm_p->bonds[i].latm_p->adj_info.adj_atms[k] =
            rxndsp_p->goal_dm_p->bonds[i].latm_p->adj_info.adj_atms[k + 1];
        }
      else
        j++;
      for (j = 0;
           j < rxndsp_p->goal_dm_p->bonds[i].ratm_p->adj_info.num_neighbs; )
        if (rxndsp_p->goal_dm_p->bonds[i].ratm_p->adj_info.adj_atms[j] ==
            rxndsp_p->goal_dm_p->bonds[i].latm_p - rxndsp_p->goal_dm_p->atoms)
        {
        rxndsp_p->goal_dm_p->bonds[i].ratm_p->adj_info.num_neighbs--;
        for (k = j;
             k < rxndsp_p->goal_dm_p->bonds[i].ratm_p->adj_info.num_neighbs;
             k++)
          rxndsp_p->goal_dm_p->bonds[i].ratm_p->adj_info.adj_atms[k] =
            rxndsp_p->goal_dm_p->bonds[i].ratm_p->adj_info.adj_atms[k + 1];
        }
      else
        j++;
      rxndsp_p->goal_dm_p->nbnds--;
      for (j = i; j < rxndsp_p->goal_dm_p->nbnds; j++)
        rxndsp_p->goal_dm_p->bonds[j] = rxndsp_p->goal_dm_p->bonds[j + 1];
      }
    else
      i++;
    }

  for (i = 0; i < rxndsp_p->subgoal_dm_p->nbnds; )
    {
    if (rxndsp_p->subgoal_dm_p->bonds[i].nlines == 0)
      {
      /* Isolate goal and subgoal by eliminating null bonds and adjacencies */
      isolate = TRUE;
      for (j = 0;
           j < rxndsp_p->subgoal_dm_p->bonds[i].latm_p->adj_info.num_neighbs; )
        if (rxndsp_p->subgoal_dm_p->bonds[i].latm_p->adj_info.adj_atms[j] ==
            rxndsp_p->subgoal_dm_p->bonds[i].ratm_p -
            rxndsp_p->subgoal_dm_p->atoms)
        {
        rxndsp_p->subgoal_dm_p->bonds[i].latm_p->adj_info.num_neighbs--;
        for (k = j;
             k < rxndsp_p->subgoal_dm_p->bonds[i].latm_p->adj_info.num_neighbs;
             k++)
          rxndsp_p->subgoal_dm_p->bonds[i].latm_p->adj_info.adj_atms[k] =
            rxndsp_p->subgoal_dm_p->bonds[i].latm_p->adj_info.adj_atms[k + 1];
        }
      else
        j++;
      for (j = 0;
           j < rxndsp_p->subgoal_dm_p->bonds[i].ratm_p->adj_info.num_neighbs; )
        if (rxndsp_p->subgoal_dm_p->bonds[i].ratm_p->adj_info.adj_atms[j] ==
            rxndsp_p->subgoal_dm_p->bonds[i].latm_p -
            rxndsp_p->subgoal_dm_p->atoms)
        {
        rxndsp_p->subgoal_dm_p->bonds[i].ratm_p->adj_info.num_neighbs--;
        for (k = j;
             k < rxndsp_p->subgoal_dm_p->bonds[i].ratm_p->adj_info.num_neighbs;
             k++)
          rxndsp_p->subgoal_dm_p->bonds[i].ratm_p->adj_info.adj_atms[k] =
            rxndsp_p->subgoal_dm_p->bonds[i].ratm_p->adj_info.adj_atms[k + 1];
        }
      else
        j++;
      rxndsp_p->subgoal_dm_p->nbnds--;
      for (j = i; j < rxndsp_p->subgoal_dm_p->nbnds; j++)
        rxndsp_p->subgoal_dm_p->bonds[j] = rxndsp_p->subgoal_dm_p->bonds[j + 1];
      }
    else
      i++;
    }

  if (isolate)
    {
    for (atom_inx = num_isol[GOAL] = num_isol[SUBG] = 0,
         first_null_bond[GOAL] = rxndsp_p->goal_dm_p->nbnds,
         first_null_bond[SUBG] = rxndsp_p->subgoal_dm_p->nbnds;
         atom_inx < rxndsp_p->both_dm.natms; atom_inx++)
      {
      if (rxndsp_p->goal_dm_p->atoms[atom_inx].adj_info.num_neighbs == 0)
        {
        if (num_isol[GOAL]++ == 0) first_isol[GOAL] = atom_inx;
        else
          {
          rxndsp_p->goal_dm_p->atoms[last_isol[GOAL]].adj_info.adj_atms
            [rxndsp_p->goal_dm_p->atoms[last_isol[GOAL]].adj_info.num_neighbs++]
            = atom_inx;
          rxndsp_p->goal_dm_p->atoms[atom_inx].adj_info.adj_atms
            [rxndsp_p->goal_dm_p->atoms[atom_inx].adj_info.num_neighbs++] =
            last_isol[GOAL];
          rxndsp_p->goal_dm_p->bonds[rxndsp_p->goal_dm_p->nbnds].latm_p =
            rxndsp_p->goal_dm_p->atoms + last_isol[GOAL];
          rxndsp_p->goal_dm_p->bonds[rxndsp_p->goal_dm_p->nbnds++].ratm_p =
            rxndsp_p->goal_dm_p->atoms + atom_inx;
          }
        last_isol[GOAL] = atom_inx;
        }

      if (rxndsp_p->subgoal_dm_p->atoms[atom_inx].adj_info.num_neighbs == 0)
        {
        if (num_isol[SUBG]++ == 0) first_isol[SUBG] = atom_inx;
        else
          {
          rxndsp_p->subgoal_dm_p->atoms[last_isol[SUBG]].adj_info.adj_atms
            [rxndsp_p->subgoal_dm_p->atoms
            [last_isol[SUBG]].adj_info.num_neighbs++] = atom_inx;
          rxndsp_p->subgoal_dm_p->atoms[atom_inx].adj_info.adj_atms
            [rxndsp_p->subgoal_dm_p->atoms[atom_inx].adj_info.num_neighbs++] =
            last_isol[SUBG];
          rxndsp_p->subgoal_dm_p->bonds[rxndsp_p->subgoal_dm_p->nbnds].latm_p =
            rxndsp_p->subgoal_dm_p->atoms + last_isol[SUBG];
          rxndsp_p->subgoal_dm_p->bonds[rxndsp_p->subgoal_dm_p->nbnds++].ratm_p
            = rxndsp_p->subgoal_dm_p->atoms + atom_inx;
          }
        last_isol[SUBG] = atom_inx;
        }
      }

    if (num_isol[GOAL] > 2)
    /* Make cycle of null bonds */
      {
      rxndsp_p->goal_dm_p->atoms[last_isol[GOAL]].adj_info.adj_atms
        [rxndsp_p->goal_dm_p->atoms[last_isol[GOAL]].adj_info.num_neighbs++] =
        first_isol[GOAL];
      rxndsp_p->goal_dm_p->atoms[first_isol[GOAL]].adj_info.adj_atms
        [rxndsp_p->goal_dm_p->atoms[first_isol[GOAL]].adj_info.num_neighbs++] =
        last_isol[GOAL];
      rxndsp_p->goal_dm_p->bonds[rxndsp_p->goal_dm_p->nbnds].latm_p =
        rxndsp_p->goal_dm_p->atoms + last_isol[GOAL];
      rxndsp_p->goal_dm_p->bonds[rxndsp_p->goal_dm_p->nbnds++].ratm_p =
        rxndsp_p->goal_dm_p->atoms + first_isol[GOAL];
      }

    if (num_isol[SUBG] > 2)
    /* Make cycle of null bonds */
      {
      rxndsp_p->subgoal_dm_p->atoms[last_isol[SUBG]].adj_info.adj_atms
        [rxndsp_p->subgoal_dm_p->atoms[last_isol[SUBG]].adj_info.num_neighbs++]
        = first_isol[SUBG];
      rxndsp_p->subgoal_dm_p->atoms[first_isol[SUBG]].adj_info.adj_atms
        [rxndsp_p->subgoal_dm_p->atoms[first_isol[SUBG]].adj_info.num_neighbs++]
        = last_isol[SUBG];
      rxndsp_p->subgoal_dm_p->bonds[rxndsp_p->subgoal_dm_p->nbnds].latm_p =
        rxndsp_p->subgoal_dm_p->atoms + last_isol[SUBG];
      rxndsp_p->subgoal_dm_p->bonds[rxndsp_p->subgoal_dm_p->nbnds++].ratm_p =
        rxndsp_p->subgoal_dm_p->atoms + first_isol[SUBG];
      }

      for (i = first_null_bond[GOAL]; i < rxndsp_p->goal_dm_p->nbnds; i++)
        rxndsp_p->goal_dm_p->bonds[i].nlines =
        rxndsp_p->goal_dm_p->bonds[i].stereo =
        rxndsp_p->goal_dm_p->bonds[i].isSelected = 0;

      for (i = first_null_bond[SUBG]; i < rxndsp_p->subgoal_dm_p->nbnds; i++)
        rxndsp_p->subgoal_dm_p->bonds[i].nlines =
        rxndsp_p->subgoal_dm_p->bonds[i].stereo =
        rxndsp_p->subgoal_dm_p->bonds[i].isSelected = 0;

    }

  for (root_inx = 0; root_inx < RxnDsp_RootCnt_Get (rxndsp_p); root_inx++)
    for (atom_inx = 0, root_found = FALSE;
        !root_found && atom_inx < rxndsp_p->both_dm.natms;
        atom_inx++) if (RxnDsp_AtomIsRoot_Get (rxndsp_p, atom_inx, root_inx))
    {
    root_found = TRUE;
    root_atom[root_inx] = atom_inx;
    }

  goal_ok = dsp_Shelley (rxndsp_p->goal_dm_p);
  subgoal_ok = dsp_Shelley (rxndsp_p->subgoal_dm_p);

  if (!goal_ok || !subgoal_ok)
    {
    /* Update root (x, y) just in case ... */
    for (root_inx = 0; root_inx < RxnDsp_RootCnt_Get (rxndsp_p); root_inx ++)
      {
      RxnDsp_RootPtr_Get (rxndsp_p, root_inx)->x =
        RxnDsp_AtomPtr_Get (rxndsp_p, root_atom[root_inx])->x;
      RxnDsp_RootPtr_Get (rxndsp_p, root_inx)->y =
        RxnDsp_AtomPtr_Get (rxndsp_p, root_atom[root_inx])->y;
      }
    return (FALSE);
    }

  if (isolate)
    for (i = 0; i < rxndsp_p->both_dm.natms; i++)
    {
    RxnDsp_AtomXCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), GOAL,
      rxndsp_p->goal_dm_p->atoms[i].x);
    RxnDsp_AtomXCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), SUBG,
      rxndsp_p->subgoal_dm_p->atoms[i].x);

    RxnDsp_AtomYCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), GOAL,
      rxndsp_p->goal_dm_p->atoms[i].y);
    RxnDsp_AtomYCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), SUBG,
      rxndsp_p->subgoal_dm_p->atoms[i].y);
    }

  else
    for (i = 0; i < rxndsp_p->both_dm.natms; i++)
    {
    RxnDsp_AtomXCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), GOAL,
      (rxndsp_p->goal_dm_p->atoms[i].x =
      rxndsp_p->subgoal_dm_p->atoms[i].x));
    RxnDsp_AtomXCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), SUBG,
      rxndsp_p->subgoal_dm_p->atoms[i].x);

    RxnDsp_AtomYCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), GOAL,
      (rxndsp_p->goal_dm_p->atoms[i].y =
      rxndsp_p->subgoal_dm_p->atoms[i].y));
    RxnDsp_AtomYCoord_Put (RxnDsp_AtomPtr_Get (rxndsp_p, i), SUBG,
      rxndsp_p->subgoal_dm_p->atoms[i].y);
    }

  if (isolate)
    {
    rxndsp_p->both_dm.molw = rxndsp_p->goal_dm_p->molw >
      rxndsp_p->subgoal_dm_p->molw ? rxndsp_p->goal_dm_p->molw :
      rxndsp_p->subgoal_dm_p->molw;
    rxndsp_p->both_dm.molh = rxndsp_p->goal_dm_p->molh >
      rxndsp_p->subgoal_dm_p->molh ? rxndsp_p->goal_dm_p->molh :
      rxndsp_p->subgoal_dm_p->molh;
    }

  else
    {
    rxndsp_p->both_dm.molw = rxndsp_p->goal_dm_p->molw =
      rxndsp_p->subgoal_dm_p->molw;
    rxndsp_p->both_dm.molh = rxndsp_p->goal_dm_p->molh =
      rxndsp_p->subgoal_dm_p->molh;
    }

  /* Update root (x, y) */
  for (root_inx = 0; root_inx < RxnDsp_RootCnt_Get (rxndsp_p); root_inx ++)
    {
    RxnDsp_RootPtr_Get (rxndsp_p, root_inx)->x =
      RxnDsp_AtomXCoord_Get (RxnDsp_AtomPtr_Get (
        rxndsp_p, root_atom[root_inx]), GOAL);
    RxnDsp_RootPtr_Get (rxndsp_p, root_inx)->y =
      RxnDsp_AtomYCoord_Get (RxnDsp_AtomPtr_Get (
        rxndsp_p, root_atom[root_inx]), GOAL);
    }

  return (TRUE);
}

RxnDsp_Molecule_t *Xtr2RxnDsp (Xtr_t *goalxtr, Xtr_t *subgxtr, U32_t *roots,
  U32_t *synthemes)
{
  RxnDsp_Molecule_t       *rxnpatt;
  int                      nroots, i, j, k, gla, gra, sgla, sgra;
  RxnDsp_Bond_t           *gbonds, *sgbonds;
  RxnDsp_Atom_t           *gatoms, *sgatoms;
  Boolean_t                found;

  rxnpatt = (RxnDsp_Molecule_t *) malloc (RXNDSP_MOLECULE_SIZE);
  rxnpatt->goal_dm_p = Xtr2Dsp_NoMessup (goalxtr);
  rxnpatt->subgoal_dm_p = Xtr2Dsp_NoMessup (subgxtr);
  rxnpatt->both_dm = *(rxnpatt->goal_dm_p);
  RxnDsp_RootCnt_Put (rxnpatt, 0); /* to be updated at end of function */

  if (rxnpatt->both_dm.nallocatms < DRW_MEM_UNIT)
    {
    rxnpatt->both_dm.nallocatms = DRW_MEM_UNIT;
    rxnpatt->both_dm.atoms = (RxnDsp_Atom_t *) realloc (rxnpatt->both_dm.atoms,
      rxnpatt->both_dm.nallocatms * RXNDSP_ATOM_SIZE);
    }

  /* If realloc changed pointer value, remap bonds, which use atom pointers
     instead of indices! */
  if (rxnpatt->both_dm.atoms != rxnpatt->goal_dm_p->atoms)
    {
    for (i = 0; i < rxnpatt->both_dm.nbnds; i++)
      {
      gla = rxnpatt->both_dm.bonds[i].latm_p - rxnpatt->goal_dm_p->atoms;
      gra = rxnpatt->both_dm.bonds[i].ratm_p - rxnpatt->goal_dm_p->atoms;
      rxnpatt->both_dm.bonds[i].latm_p = rxnpatt->both_dm.atoms + gla;
      rxnpatt->both_dm.bonds[i].ratm_p = rxnpatt->both_dm.atoms + gra;
      }
    j = rxnpatt->both_dm.rxncnr_p - rxnpatt->goal_dm_p->atoms;
    rxnpatt->both_dm.rxncnr_p = rxnpatt->both_dm.atoms + j;
    }

  if (rxnpatt->both_dm.nallocbnds < DRW_MEM_UNIT)
    {
    rxnpatt->both_dm.nallocbnds = DRW_MEM_UNIT;
    rxnpatt->both_dm.bonds = (RxnDsp_Bond_t *) realloc (rxnpatt->both_dm.bonds,
      rxnpatt->both_dm.nallocbnds * RXNDSP_BOND_SIZE);
    }

  /* combine goal & subgoal; run rxndsp_Shelley to get coordinates before
     assigning root coordinates */
  gbonds = rxnpatt->both_dm.bonds;
  gatoms = rxnpatt->both_dm.atoms;
  sgbonds = rxnpatt->subgoal_dm_p->bonds;
  sgatoms = rxnpatt->subgoal_dm_p->atoms;
  for (i = 0; i < rxnpatt->both_dm.natms; i++)
    {
    RxnDsp_AtomCharge_Put (RxnDsp_AtomPtr_Get (rxnpatt, i), SUBG,
      sgatoms[i].chg[0]);
    RxnDsp_AtomSelect_Put (RxnDsp_AtomPtr_Get (rxnpatt, i), SUBG,
      sgatoms[i].isSelected);
    }

  for (i = 0; i < rxnpatt->subgoal_dm_p->nbnds; i++)
    {
    sgla = sgbonds[i].latm_p - sgatoms;
    sgra = sgbonds[i].ratm_p - sgatoms;
    for (j = 0, found = FALSE; !found && j < rxnpatt->both_dm.nbnds; j++)
      {
      gla = gbonds[j].latm_p - gatoms;
      gra = gbonds[j].ratm_p - gatoms;
      if ((sgla == gla && sgra == gra) || (sgla == gra && sgra == gla))
        {
        found = TRUE;
        k = j;
        }
      }
    if (!found)
      {
      k = rxnpatt->both_dm.nbnds++;
      RxnDsp_BondPtr_Get (rxnpatt, k)->latm_p =
        RxnDsp_AtomPtr_Get (rxnpatt, sgla);
      RxnDsp_BondPtr_Get (rxnpatt, k)->ratm_p =
        RxnDsp_AtomPtr_Get (rxnpatt, sgra);
      RxnDsp_AtomPtr_Get (rxnpatt, sgla)->adj_info.adj_atms
        [RxnDsp_AtomPtr_Get (rxnpatt, sgla)->adj_info.num_neighbs++] = sgra;
      RxnDsp_AtomPtr_Get (rxnpatt, sgra)->adj_info.adj_atms
        [RxnDsp_AtomPtr_Get (rxnpatt, sgra)->adj_info.num_neighbs++] = sgla;
      RxnDsp_BondSelect_Put (RxnDsp_BondPtr_Get (rxnpatt, k), GOAL, 0);
      RxnDsp_BondOrder_Put (RxnDsp_BondPtr_Get (rxnpatt, k), GOAL, 0);
      RxnDsp_BondStereo_Put (RxnDsp_BondPtr_Get (rxnpatt, k), GOAL, 0);
      }
    RxnDsp_BondSelect_Put (RxnDsp_BondPtr_Get (rxnpatt, k), SUBG,
      sgbonds[i].isSelected);
    RxnDsp_BondOrder_Put (RxnDsp_BondPtr_Get (rxnpatt, k), SUBG,
      sgbonds[i].nlines);
    RxnDsp_BondStereo_Put (RxnDsp_BondPtr_Get (rxnpatt, k), SUBG,
      sgbonds[i].stereo);
    }

    free (rxnpatt->goal_dm_p);
    rxnpatt->goal_dm_p = NULL;
    free_Molecule (rxnpatt->subgoal_dm_p);
    rxnpatt->subgoal_dm_p = NULL;

  if (!rxndsp_Shelley (rxnpatt))
    {
    printf ("Error importing drawing\n");
    free (rxnpatt);
    return ((RxnDsp_Molecule_t *) NULL);
    }

  for (i = nroots = 0; i < MAXROOTS; i++)
    if (roots[i] != REACT_NODE_INVALID)
    {
    nroots++;
    RxnDsp_RootPtr_Get (rxnpatt, i)->x =
      RxnDsp_AtomXCoord_Get (RxnDsp_AtomPtr_Get (rxnpatt, roots[i]), GOAL);
    RxnDsp_RootPtr_Get (rxnpatt, i)->y =
      RxnDsp_AtomYCoord_Get (RxnDsp_AtomPtr_Get (rxnpatt, roots[i]), GOAL);
    RxnDsp_RootPtr_Get (rxnpatt, i)->syntheme = synthemes[i];
    }

  RxnDsp_RootCnt_Put (rxnpatt, nroots);

  return (rxnpatt);
}

Dsp_Molecule_t *Xtr2Dsp_NoMessup (Xtr_t *xtr_p)
{
  Dsp_Molecule_t   *mol_p;
  Dsp_Atom_t       *atom_p;
  Dsp_Bond_t       *bond_p;
  U16_t             atm_i;
  U8_t              bond_info, ii;
  char              sym[5];
  int               var_i;

  mol_p = (Dsp_Molecule_t *) malloc (DSP_MOLECULE_SIZE);

  /* Transform atom relative info first */
  mol_p->natms = Xtr_NumAtoms_Get (xtr_p);
  mol_p->atoms = (Dsp_Atom_t *) malloc (DSP_ATOM_SIZE * mol_p->natms);
  mol_p->nallocatms = mol_p->natms;

  mol_p->nbnds = 0;
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) {

    /* Convert atom symbol to dsp format */
    strcpy (sym, Atomid2Symbol (Xtr_Attr_Atomid_Get (xtr_p, atm_i)));
    if (strcmp (sym, "I") == 0)
      strcpy (atom_p->sym, " I ");
    else if (strcasecmp (sym, "#J") == 0)
      strcpy (atom_p->sym, "X");
    else if (sym[0] == '$')
      {
      sscanf (sym + 1, "%d", &var_i);
      if (var_i % 2 == 0) strcpy (atom_p->sym, "R");
      else strcpy (atom_p->sym, "R'");
      }
    else if (strcmp (sym, ":`") == 0)
      strcpy (atom_p->sym, " : ");
    else if (strcmp (sym, ".`") == 0)
      strcpy (atom_p->sym, " . ");
    else 
      strcpy (atom_p->sym, sym);

    strcpy (atom_p->chg, "\0");
    strcpy (atom_p->map, "\0");

    if (strcmp (sym, "C") == 0)
      atom_p->isC = TRUE;
    else
      atom_p->isC = FALSE;

    /* Mark all atoms as selected for latter bond transformation */
    atom_p->isSelected = DRW_SELECT_TOTAL;

    atom_p->adj_info.num_neighbs = Xtr_Attr_NumNeighbors_Get (xtr_p, atm_i);
    mol_p->nbnds += atom_p->adj_info.num_neighbs;

    for (ii = 0; ii < atom_p->adj_info.num_neighbs; ii++)
      atom_p->adj_info.adj_atms[ii] = 
	(long) Xtr_Attr_NeighborId_Get (xtr_p, atm_i, ii);

    atom_p++;
  } /* End for all atoms */

  /* Transform bond relative info */
  mol_p->nbnds = mol_p->nbnds >> 1;
  mol_p->bonds = (Dsp_Bond_t *) malloc (DSP_BOND_SIZE * mol_p->nbnds);
  mol_p->nallocbnds = mol_p->nbnds;

  atom_p = mol_p->atoms;
  bond_p = mol_p->bonds;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) {

    for (ii = 0; ii < atom_p->adj_info.num_neighbs; ii++) {
      /* Make only for selected atoms, 
	 so all bonds will be traced only once
       */
      if ((mol_p->atoms + atom_p->adj_info.adj_atms[ii])->isSelected 
	  == DRW_SELECT_TOTAL) {

	bond_p->latm_p = atom_p;
	bond_p->ratm_p = mol_p->atoms + atom_p->adj_info.adj_atms[ii];

	/* Convert multiplicity of bond */
	bond_info = Xtr_Attr_NeighborBond_Get(xtr_p, atm_i, ii);

	switch (bond_info) {
	case  BOND_SINGLE:
	  bond_p->nlines = DSP_BOND_SINGLE;
	  break;
	case  BOND_DOUBLE:
	  bond_p->nlines = DSP_BOND_DOUBLE;
	  break;
	case  BOND_TRIPLE:
	  bond_p->nlines = DSP_BOND_TRIPLE;
	  break;
	case  BOND_RESONANT:
	  /* If molecule has resonant bond representation, DO NOT fix it into 
	     "single-double" representation - these are embedding patterns */
          bond_p->nlines = DSP_BOND_RESONANT;
	  break;
        case  BOND_VARIABLE:
          bond_p->nlines = DSP_BOND_VARIABLE;
	  break;
	}

	/* Convert stereo information */
/*	bond_info = Xtr_Attr_NeighborStereo_Get (xtr_p, atm_i, ii); */
	bond_info = 100;

	switch (bond_info) {
	case  BOND_DIR_UP:
	  bond_p->stereo = DSP_BOND_STEREO_UP;
	  break;
	case  BOND_DIR_DOWN:
	  bond_p->stereo = DSP_BOND_STEREO_DOWN;
	  break;
	default:
	  bond_p->stereo = DSP_BOND_STEREO_NONE;
	}

	bond_p->isSelected = DRW_SELECT_NONE;

	bond_p++;
      } /* End of if atom neighb info wasn't traced */
    }

    atom_p->isSelected = DRW_SELECT_NONE;
    atom_p++;
  } /* End for all atoms */

  return (mol_p);
}

void dump_RxnDsp (RxnDsp_Molecule_t *rxndsp_p, char *where)
{
  int i;

  printf ("Dump of RxnDsp_Molecule_t (%p) at %s\n", rxndsp_p,where);
  printf ("---------------------------\n");
  printf ("%d roots:\n", rxndsp_p->num_roots);
  for (i=0; i<rxndsp_p->num_roots; i++)
    printf("\tSyntheme %d rooted at (%d, %d)\n", rxndsp_p->root_syn[i].syntheme,
      rxndsp_p->root_syn[i].x, rxndsp_p->root_syn[i].y);
  printf ("Goal:\n");
  if (rxndsp_p->goal_dm_p == NULL) printf("\tNULL\n");
  else dump_Dsp (rxndsp_p->goal_dm_p, "\t");
  printf ("Subgoal:\n");
  if (rxndsp_p->subgoal_dm_p == NULL) printf("\tNULL\n");
  else dump_Dsp (rxndsp_p->subgoal_dm_p, "\t");
  printf ("Both:\n");
  dump_Dsp (&rxndsp_p->both_dm, "\t");
}

void dump_Dsp (Dsp_Molecule_t *dsp_p, char *indent)
{
  int i, j;

  printf ("%s%d (%d) atoms:\n", indent, dsp_p->natms, dsp_p->nallocatms);
  for (i=0; i<dsp_p->natms; i++)
    {
    printf ("%s\t%d: %s [map=%s chg=%d %d] (%d, %d)%s - %d neighbors:\n%s\t\t",
      indent, i, dsp_p->atoms[i].sym, dsp_p->atoms[i].map,
      dsp_p->atoms[i].chg[0], dsp_p->atoms[i].chg[1],
      dsp_p->atoms[i].x, dsp_p->atoms[i].y,
      dsp_p->atoms[i].isC ? " carbon" :"",
      dsp_p->atoms[i].adj_info.num_neighbs, indent);
    for (j=0; j<dsp_p->atoms[i].adj_info.num_neighbs; j++)
      printf (" %d", dsp_p->atoms[i].adj_info.adj_atms[j]);
    printf ("\n%s\tkvadr: %d; xp/xn/yp/yn: %c/%c/%c/%c%s\n",
      indent, dsp_p->atoms[i].kvadr, dsp_p->atoms[i].xp ? 'Y' : 'N',
      dsp_p->atoms[i].xn ? 'Y' : 'N', dsp_p->atoms[i].yp ? 'Y' : 'N',
      dsp_p->atoms[i].yn ? 'Y' : 'N',
      dsp_p->atoms[i].isSelected ? "; selected" : "");
    }
  printf ("%s%d (%d) bonds:\n", indent, dsp_p->nbnds, dsp_p->nallocbnds);
  for (i=0; i<dsp_p->nbnds; i++)
    printf ("%s\t%d, %d: %d; stereo=%d%s\n", indent,
      dsp_p->bonds[i].latm_p - dsp_p->atoms,
      dsp_p->bonds[i].ratm_p - dsp_p->atoms,
      dsp_p->bonds[i].nlines, dsp_p->bonds[i].stereo,
      dsp_p->bonds[i].isSelected ? "; selected" : "");
  printf ("%smolh=%d molw=%d\n", indent, dsp_p->molh, dsp_p->molw);
  printf ("%srxncnr_p: ", indent);
  if (dsp_p->rxncnr_p == NULL)
    printf ("NULL\n");
  else
    printf ("%d\n", dsp_p->rxncnr_p - dsp_p->atoms);
  printf ("%sscale=%0.2f%s\n", indent, dsp_p->scale,
    dsp_p->map_em ? "; map_em" : "");
}
