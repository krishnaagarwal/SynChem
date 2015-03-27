/******************************************************************************
*
*  Copyright (C) 1994, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     DSP.C
*
*    This module is the code for the abstract data-type, DSP.  This is 
*    a molecule representation designated for 2D drawing and displaying.
*    The purpose of the functions defined is to manipulate DSP and has
*    nothing to do with actual drawings. The module is independent of
*    X/Motif.
*
*  Creation Date:
*
*    01-Jan-1994
*
*  Authors:
*
*    Ruslan Bilorusets
*    Daren Krebsbach
*
*  Routines:
*
*    get_Atom_There
*    get_Bond_There
*    delete_Atom
*    delete_Bond
*    store_Atom
*    store_Bond
*    copy_Molecule
*    free_Molecule
*    Xtr2Dsp
*    Dsp2Xtr
*    dsp_Shelley
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Russ       xxx
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
/*
#include <X11/X.h>
#include <Xm/Xm.h>
*/

#include "dummy.h"

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"
 
#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

/****************************************************************************
*
*  Function Name:                 get_Atom_There
*
*    This routine checks if there is an atom in the DSP structure within
*    a certain area on XY-coord plain. The area is specified by the
*    arguments passed to the routine and by DRW_HILIT_AREA defined
*    in dsp.h.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p - pointer to a DSP data structure
*    x     - x-coordinate of the area's center
*    y     - y-coordinate of the area's center
*
*  Return Values:
*
*    get_Atom_There returns a pointer to an atom in the DSP,
*    if such an atom exist within the specified area. Otherwise,
*    NULL pointer is returned.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Dsp_Atom_t  *get_Atom_There (Dsp_Molecule_t*  mol_p,
			     int x, int y)
{
  int           atm_i;
  Dsp_Atom_t   *atm_p;

  atm_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    if ((x > (atm_p->x - DRW_HILIT_AREA)) && (x < (atm_p->x + DRW_HILIT_AREA)) 
	&&
	(y > (atm_p->y - DRW_HILIT_AREA)) && (y < (atm_p->y + DRW_HILIT_AREA)))
      return (atm_p);

    atm_p++;
    }

  return ((Dsp_Atom_t *) NULL);
}


/****************************************************************************
*
*  Function Name:                 get_Atom_There
*
*    This routine checks if there is a bond in the DSP structure within
*    a certain area on XY-coord plain. The area is specified by the
*    arguments passed to the routine and by DRW_HILIT_AREA defined
*    in dsp.h. Order in which the DSP's bond data structure keeps its ponters
*    to the adjacent atoms does not make difference.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p - pointer to a DSP data structure
*    x1    - x-coordinate of "left" adjacent atom
*    y1    - y-coordinate of "left" adjacent atom
*    x2    - x-coordinate of "right" adjacent atom
*    y2    - y-coordinate of "right" adjacent atom
*
*  Return Values:
*
*    get_Bond_There returns a pointer to a bond in the DSP,
*    if such an atom exist within the specified area. Otherwise,
*    NULL pointer is returned.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Dsp_Bond_t  *get_Bond_There (Dsp_Molecule_t*  mol_p,
			     int x1, int y1,
			     int x2, int y2)
{
  int           bnd_i;
  Dsp_Bond_t   *bnd_p;

  bnd_p = mol_p->bonds;
  for (bnd_i = 0; bnd_i < mol_p->nbnds; bnd_i++) 
    {
    if (((x1 > (bnd_p->latm_p->x - DRW_HILIT_AREA)) && 
	 (x1 < (bnd_p->latm_p->x + DRW_HILIT_AREA)) &&
	 (y1 > (bnd_p->latm_p->y - DRW_HILIT_AREA)) && 
	 (y1 < (bnd_p->latm_p->y + DRW_HILIT_AREA)) &&
	 (x2 > (bnd_p->ratm_p->x - DRW_HILIT_AREA)) && 
	 (x2 < (bnd_p->ratm_p->x + DRW_HILIT_AREA)) &&
	 (y2 > (bnd_p->ratm_p->y - DRW_HILIT_AREA)) && 
	 (y2 < (bnd_p->ratm_p->y + DRW_HILIT_AREA))) ||

	((x1 > (bnd_p->ratm_p->x - DRW_HILIT_AREA)) && 
	 (x1 < (bnd_p->ratm_p->x + DRW_HILIT_AREA)) &&
	 (y1 > (bnd_p->ratm_p->y - DRW_HILIT_AREA)) && 
	 (y1 < (bnd_p->ratm_p->y + DRW_HILIT_AREA)) &&
	 (x2 > (bnd_p->latm_p->x - DRW_HILIT_AREA)) && 
	 (x2 < (bnd_p->latm_p->x + DRW_HILIT_AREA)) &&
	 (y2 > (bnd_p->latm_p->y - DRW_HILIT_AREA)) && 
	 (y2 < (bnd_p->latm_p->y + DRW_HILIT_AREA))))
      return (bnd_p);

    bnd_p++;
    }

  return ((Dsp_Bond_t *) NULL);
}


/****************************************************************************
*
*  Function Name:                 delete_Atom
*
*    This routine deletes an atom from the DSP structure.
*    Bonds adjacent to the atom are deleted as well.
*    Number of atoms in the DSP (natms) is decremented.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p  - pointer to a DSP data structure
*    atom_p - pointer to the atom to be deleted
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    The last atom in the atom list (atoms) of the DSP data structure
*    is copied to the place of the deleted atom. That implies that
*    the adjacent information about the last atom in the atoms adjacent
*    to it is being changed. The atom pointers (latm_p and ratm_p)
*    in the bonds adjacent to the last atom are changed as well and
*    point to the new position of the (used to be) last atom.
*
******************************************************************************/
void  delete_Atom (Dsp_Molecule_t  *mol_p,
		   Dsp_Atom_t      *atom_p)
{
  int              adj_i, ii, jj;
  Dsp_Bond_t      *bond_p;
  Dsp_Atom_t      *adjatm_p;

  /* For all neighbors of this atom in DrawAdjList */
  while (atom_p->adj_info.num_neighbs != 0) 
    {
    /* Delete bonds adjacent to the atom to be deleted */
    bond_p = get_Bond_There(mol_p, atom_p->x, atom_p->y, 
      (mol_p->atoms + atom_p->adj_info.adj_atms[0])->x,
      (mol_p->atoms + atom_p->adj_info.adj_atms[0])->y);
    delete_Bond (mol_p, bond_p);

    }/* End for all neighbors */

  mol_p->natms--;
  if (mol_p->natms == 0)
    return;

  /* Copy the last atom data to the atom to be deleted */
  atom_p->x = (mol_p->atoms + mol_p->natms)->x;
  atom_p->y = (mol_p->atoms + mol_p->natms)->y;
  atom_p->isC = (mol_p->atoms + mol_p->natms)->isC;
  atom_p->isSelected = (mol_p->atoms + mol_p->natms)->isSelected;
  atom_p->adj_info.num_neighbs = 
    (mol_p->atoms + mol_p->natms)->adj_info.num_neighbs;

  for (jj = 0; jj < atom_p->adj_info.num_neighbs; jj++)
    atom_p->adj_info.adj_atms[jj] = 
      (mol_p->atoms + mol_p->natms)->adj_info.adj_atms[jj];

  strcpy (atom_p->sym, (mol_p->atoms + mol_p->natms)->sym);
  strcpy (atom_p->map, (mol_p->atoms + mol_p->natms)->map);
  strcpy (atom_p->chg, (mol_p->atoms + mol_p->natms)->chg);

  (mol_p->atoms + mol_p->natms)->sym[0] = '\0';
  (mol_p->atoms + mol_p->natms)->map[0] = '\0';
  (mol_p->atoms + mol_p->natms)->chg[0] = '\0';

  /* Find number of this atom in "mol_p" strusture */
  for (adj_i = 0; adj_i < mol_p->natms; adj_i++)
    if (((mol_p->atoms + adj_i)->x == atom_p->x) &&
	((mol_p->atoms + adj_i)->y == atom_p->y))
      break;

  /* Change neighbor-info for all atoms adjacent to the
     atom that was last in DrawAdjList, but now is 
     at the place of the atom to be deleted.
   */
  for (ii = 0; ii < atom_p->adj_info.num_neighbs; ii++) 
    {
    adjatm_p = mol_p->atoms + (int) atom_p->adj_info.adj_atms[ii];
    for (jj = 0; jj < adjatm_p->adj_info.num_neighbs; jj++)
      if (adjatm_p->adj_info.adj_atms[jj] == mol_p->natms)
	adjatm_p->adj_info.adj_atms[jj] = adj_i;
    }

  /* Change pointers to the last atom in bond structures */
  bond_p = mol_p->bonds;
  for (ii = 0; ii < mol_p->nbnds; ii++) 
    {
    if (bond_p->latm_p == (mol_p->atoms + mol_p->natms))
      bond_p->latm_p = atom_p;
    if (bond_p->ratm_p == (mol_p->atoms + mol_p->natms))
      bond_p->ratm_p = atom_p;

    bond_p++;
    }

  return ;
}


/****************************************************************************
*
*  Function Name:                 delete_Bond
*
*    This routine deletes a bond from the DSP structure.
*    Adjacent information that this bond was carring out
*    is deleted from the atom data structures.
*    Number of bonds in the DSP (nbnds) is decremented.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p  - pointer to a DSP data structure
*    bond_p - pointer to the bond to be deleted
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    The last bond in the bond list (bonds) of the DSP data structure
*    is copied to the place of the deleted bond.
*
******************************************************************************/
void  delete_Bond (Dsp_Molecule_t  *mol_p,
		   Dsp_Bond_t      *dlbnd_p)
{
  int              adj_i, adj_j, jj;
  Boolean_t        del_fg;

  /* Find number of "left" atom adjacent to the bond to be deleted 
     in "mol_p" strusture */
  for (adj_i = 0; adj_i < mol_p->natms; adj_i++)
    if ((mol_p->atoms + adj_i) == dlbnd_p->latm_p)
      break;

  /* Find number of "right" atom adjacent to the bond to be deleted 
     in "mol_p" strusture */
  for (adj_j = 0; adj_j < mol_p->natms; adj_j++)
    if ((mol_p->atoms + adj_j) == dlbnd_p->ratm_p)
      break;

  /* Delete the adjacency info about this bond */
  del_fg = FALSE;
  for (jj = 0; jj < dlbnd_p->latm_p->adj_info.num_neighbs; jj++) 
    {
    if (del_fg)
      dlbnd_p->latm_p->adj_info.adj_atms[jj - 1] = 
	dlbnd_p->latm_p->adj_info.adj_atms[jj];
    if (dlbnd_p->latm_p->adj_info.adj_atms[jj] == adj_j)
      del_fg = TRUE;
    }

  (dlbnd_p->latm_p->adj_info.num_neighbs)--;

  del_fg = FALSE;
  for (jj = 0; jj < dlbnd_p->ratm_p->adj_info.num_neighbs; jj++) 
    {
    if (del_fg)
      dlbnd_p->ratm_p->adj_info.adj_atms[jj - 1] = 
	dlbnd_p->ratm_p->adj_info.adj_atms[jj];
    if (dlbnd_p->ratm_p->adj_info.adj_atms[jj] == adj_i)
      del_fg = TRUE;
    }

  (dlbnd_p->ratm_p->adj_info.num_neighbs)--;
  mol_p->nbnds--;

  /* Copy the last bond data to the bond to be deleted */
  dlbnd_p->latm_p = (mol_p->bonds + mol_p->nbnds)->latm_p;
  dlbnd_p->ratm_p = (mol_p->bonds + mol_p->nbnds)->ratm_p;
  dlbnd_p->nlines = (mol_p->bonds + mol_p->nbnds)->nlines;
  dlbnd_p->stereo = (mol_p->bonds + mol_p->nbnds)->stereo;
  dlbnd_p->isSelected = (mol_p->bonds + mol_p->nbnds)->isSelected;

  return ;
}


/****************************************************************************
*
*  Function Name:                 store_Atom
*
*    This routine stores an atom in the DSP structure. 
*    
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p - pointer to DSP data structure
*    sym   - atom symbol
*    chg   - atom charge
*    map   - not used
*    x     - atom's x-coordinate
*    y     - atom's y-coordinate
*
*  Return Values:
*
*    store_Atom returns a pointer to a new atom that has been stored.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Dsp_Atom_t  *store_Atom (Dsp_Molecule_t  *mol_p,
			 char *sym, char *chg, char *map,
			 int x, int y)
{
  Dsp_Atom_t      *dratm_p;

  /* If atom already exists, just return pointer to it 
   */
  dratm_p = get_Atom_There (mol_p, x, y);
  if (dratm_p != NULL)
    return (dratm_p);

  /* If num of atoms exceeds num of atom data structures allocated
     in memory, alloc another piece
   */
  if (mol_p->natms == mol_p->nallocatms) 
    {
    mol_p->nallocatms += DRW_MEM_UNIT;
    if (!(mol_p->atoms = (Dsp_Atom_t *) realloc (mol_p->atoms,
	mol_p->nallocatms * DSP_ATOM_SIZE))) 
      {
      fprintf (stderr, "store_Atom : Can't allocate memory for new atom\n");
      exit(-1);
      }
    }
  
  strcpy ((mol_p->atoms + mol_p->natms)->sym, sym);
  strcpy ((mol_p->atoms + mol_p->natms)->chg, chg);
  strcpy ((mol_p->atoms + mol_p->natms)->map, map);

  (mol_p->atoms + mol_p->natms)->x = x;
  (mol_p->atoms + mol_p->natms)->y = y;

  if (strcmp (sym, "C") == 0)
    (mol_p->atoms + mol_p->natms)->isC = TRUE;
  else
    (mol_p->atoms + mol_p->natms)->isC = FALSE;

  (mol_p->atoms + mol_p->natms)->isSelected = DRW_SELECT_NONE;
  (mol_p->atoms + mol_p->natms)->adj_info.num_neighbs = 0;

  mol_p->natms++;
  return (mol_p->atoms + mol_p->natms - 1);
}


/****************************************************************************
*
*  Function Name:                 store_Bond
*
*    This routine stores a bond in the DSP structure. 
*    
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p  - pointer to DSP data structure
*    x1     - "left" atom's x-coordinate
*    y1     - "left" atom's y-coordinate
*    x2     - "right" atom's x-coordinate
*    y2     - "right" atom's y-coordinate
*    nlines - multiplicity of bond
*
*  Return Values:
*
*    store_Bond returns a pointer to a new bond that has been stored.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
 Dsp_Bond_t *store_Bond (Dsp_Molecule_t  *mol_p,
			       int x1, int y1,
			       int x2, int y2,
			       int nlines)
{
  int              adj_i, adj_j;
  Dsp_Atom_t      *statm1_p, *statm2_p;

  statm1_p = get_Atom_There (mol_p, x1, y1);

  /* If the bond to be stored exceed max number of bonds
     that could be attached to one atom, return NULL.
   */
  if (statm1_p->adj_info.num_neighbs == 6)
    return ((Dsp_Bond_t *) NULL);

  statm2_p = get_Atom_There (mol_p, x2, y2);

  /* If the bond to be stored exceed max number of bonds
     that could be attached to one atom, return NULL.
   */
  if (statm2_p->adj_info.num_neighbs == 6)
    return ((Dsp_Bond_t *) NULL);

  /* Find number of "left" atom that is adjacent to the bond to be stored 
     in "mol_p" strusture */
  for (adj_i = 0; adj_i < mol_p->natms; adj_i++)
    if (((mol_p->atoms + adj_i)->x == statm1_p->x) &&
	((mol_p->atoms + adj_i)->y == statm1_p->y))
      break;

  /* Find number of "right" atom that is adjacent to the bond to be stored
     in "mol_p" strusture */
  for (adj_j = 0; adj_j < mol_p->natms; adj_j++)
    if (((mol_p->atoms + adj_j)->x == statm2_p->x) &&
	((mol_p->atoms + adj_j)->y == statm2_p->y))
      break;

  statm1_p->adj_info.adj_atms[statm1_p->adj_info.num_neighbs] = (long) adj_j;
  statm1_p->adj_info.num_neighbs++;
  statm2_p->adj_info.adj_atms[statm2_p->adj_info.num_neighbs] = (long) adj_i;
  statm2_p->adj_info.num_neighbs++;

  /* If num of bonds exceeds num of bond data structures allocated
     in memory, alloc another piece
   */
  if (mol_p->nbnds == mol_p->nallocbnds) 
    {
    mol_p->nallocbnds += DRW_MEM_UNIT;
    if (!(mol_p->bonds = (Dsp_Bond_t *) realloc (mol_p->bonds,
	mol_p->nallocbnds * DSP_BOND_SIZE))) 
      {
      fprintf (stderr, "store_Bond : Can't allocate memory for new bond\n");
      exit(-1);
      }
    }

  /* Store pointers to the adjacent atoms of the bond. */
  (mol_p->bonds + mol_p->nbnds)->latm_p = statm1_p;
  (mol_p->bonds + mol_p->nbnds)->ratm_p = statm2_p;
  (mol_p->bonds + mol_p->nbnds)->nlines = nlines;
  (mol_p->bonds + mol_p->nbnds)->isSelected = DRW_SELECT_NONE;
  (mol_p->bonds + mol_p->nbnds)->stereo = DSP_BOND_STEREO_NONE;
  mol_p->nbnds++;

  return (mol_p->bonds + mol_p->nbnds - 1);
}


/****************************************************************************
*
*  Function Name:                 copy_Molecule
*
*    This routine makes an exact copy of a DSP molecule.
*    Space for the dupplicate molecule must be preallocated.
*    
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol1_p - pointer to DSP molecule (original)
*    mol2_p - pointer to DSP molecule (dupplicate)
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
void copy_Molecule (Dsp_Molecule_t  *mol1_p,
		    Dsp_Molecule_t  *mol2_p)
{
  Dsp_Atom_t    *atom1_p, *atom2_p;
  Dsp_Bond_t    *bond1_p, *bond2_p;
  int            atm_i, bnd_i, ii;

  mol1_p->natms = mol2_p->natms;
  mol1_p->nbnds = mol2_p->nbnds;
  mol1_p->map_em = mol2_p->map_em;
  mol1_p->nallocatms = mol2_p->nallocatms;
  mol1_p->nallocbnds = mol2_p->nallocbnds;
  mol1_p->scale = mol2_p->scale;

  atom1_p = mol1_p->atoms;
  atom2_p = mol2_p->atoms;
  for (atm_i = 0; atm_i < mol2_p->natms; atm_i++)
    {
    strcpy (atom1_p->sym, atom2_p->sym);
    strcpy (atom1_p->map, atom2_p->map);
    strcpy (atom1_p->chg, atom2_p->chg);

    atom1_p->x = atom2_p->x;
    atom1_p->y = atom2_p->y;
    atom1_p->isC = atom2_p->isC;

    atom1_p->kvadr = atom2_p->kvadr;
    atom1_p->xp = atom2_p->xp;
    atom1_p->xn = atom2_p->xn;
    atom1_p->yp = atom2_p->yp;
    atom1_p->yn = atom2_p->yn;
    atom1_p->isSelected = atom2_p->isSelected;

    atom1_p->adj_info.num_neighbs = atom2_p->adj_info.num_neighbs;

    for (ii = 0; ii < atom2_p->adj_info.num_neighbs; ii++)
      atom1_p->adj_info.adj_atms[ii] = atom2_p->adj_info.adj_atms[ii];

    atom1_p++;
    atom2_p++;
    } /* End for all atoms of mol2 */

  bond1_p = mol1_p->bonds;
  bond2_p = mol2_p->bonds;
  for (bnd_i = 0; bnd_i < mol2_p->nbnds; bnd_i++) 
    {
    bond1_p->nlines = bond2_p->nlines;
    bond1_p->stereo = bond2_p->stereo;
    bond1_p->isSelected = bond2_p->isSelected;
    atom2_p = mol2_p->atoms;
    for (ii = 0; ii < mol2_p->natms; ii++) 
      {
      if (bond2_p->latm_p == atom2_p)
	break;

      atom2_p++;
      }

    bond1_p->latm_p = (mol1_p->atoms + ii);
    atom2_p = mol2_p->atoms;
    for (ii = 0; ii < mol2_p->natms; ii++) 
      {
      if (bond2_p->ratm_p == atom2_p)
	break;

      atom2_p++;
      } 

    bond1_p->ratm_p = (mol1_p->atoms + ii);

    bond1_p++;
    bond2_p++;

    } /* For all bonds of mol2 */

  return ;
}


/****************************************************************************
*
*  Function Name:                 free_Molecule
*
*    This routine frees a DSP molecule.
*    
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Arguments:
*
*    mol_p - pointer to DSP molecule
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
void free_Molecule (Dsp_Molecule_t  *mol_p)
{
  free (mol_p->atoms);
  free (mol_p->bonds);
  free (mol_p);
  return ;
}

/**************************************************
**      Xtr2Dsp
**************************************************/
Dsp_Molecule_t   *Xtr2Dsp (Xtr_t  *xtr_p)
{
  Dsp_Molecule_t   *mol_p;
  Dsp_Atom_t       *atom_p;
  Dsp_Bond_t       *bond_p;
  U16_t             atm_i;
  U8_t              bond_info, ii;
  char              sym[3];

  mol_p = (Dsp_Molecule_t *) malloc (DSP_MOLECULE_SIZE);

  /* Transform atom relative info first */
  mol_p->natms = Xtr_NumAtoms_Get (xtr_p);
  mol_p->atoms = (Dsp_Atom_t *) malloc (DSP_ATOM_SIZE * mol_p->natms);
  mol_p->nallocatms = mol_p->natms;

  mol_p->nbnds = 0;
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) {

    /* Convert atom symbol to dsp format */
    strcpy (sym, Atomid2Symbol (Xtr_Attr_Atomid_Get(xtr_p, atm_i)));
    if (strcmp (sym, "I") == 0)
      strcpy (atom_p->sym, " I ");
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
	  /* If molecule has resonant bond representation, fix it into 
	     "single-double" representation */
	  ResonantBonds_Fix (xtr_p, atm_i, ii, 1);
	  bond_p->nlines = Xtr_Attr_NeighborBond_Get(xtr_p, atm_i, ii);
	  break;
	}

	/* Convert stereo information */
/*      bond_info = Xtr_Attr_NeighborStereo_Get (xtr_p, atm_i, ii); */
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


/**************************************************
**      Dsp2Xtr
**************************************************/
Xtr_t *Dsp2Xtr (Dsp_Molecule_t *mol_p)
{
  Xtr_t            *xtr_p;
  Dsp_Atom_t       *atom_p, *adjatm_p;
  Dsp_Bond_t       *bond_p;
  int               atm_i, ii;
  char              sym[3];

  xtr_p = (Xtr_t *) malloc (XTRSIZE);

  /* Transform atom relative info first */
  Xtr_NumAtoms_Put (xtr_p, mol_p->natms);
  xtr_p->attributes = (XtrRow_t *) malloc (XTRROWSIZE * mol_p->natms);

  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    /* Convert atom symbol in dsp format to atom id */
    if (strcmp (atom_p->sym, " I ") == 0)
      strcpy (sym, "I");      
    else if (strcmp (atom_p->sym, " : ") == 0)
      strcpy (sym, ":`");
    else if (strcmp (atom_p->sym, " . ") == 0)
      strcpy (sym, ".`");
    Xtr_Attr_Atomid_Put (xtr_p, atm_i, Atomsymbol2Id((U8_t *)sym));        

    Xtr_Attr_NumNeighbors_Put (xtr_p, atm_i, atom_p->adj_info.num_neighbs);

    /* Transform bond relative info */
    for (ii = 0; ii < atom_p->adj_info.num_neighbs; ii++) 
      {
      Xtr_Attr_NeighborId_Put (xtr_p, atm_i, ii, 
	(U16_t) atom_p->adj_info.adj_atms[ii]);
      adjatm_p = mol_p->atoms + (int) atom_p->adj_info.adj_atms[ii];
      bond_p = get_Bond_There (mol_p, atom_p->x, atom_p->y,
	adjatm_p->x, adjatm_p->y);

      /* Convert multiplicity of bond */
      switch (bond_p->nlines) 
	{
	case  DSP_BOND_SINGLE:
	  Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_SINGLE);
	  break;
	case  DSP_BOND_DOUBLE:
	  Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_DOUBLE);
	  break;
	case  DSP_BOND_TRIPLE:
	  Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_TRIPLE);
	  break;
	case  DSP_BOND_RESONANT:
	  Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_RESONANT);
	  break;
	default:
	  Xtr_Attr_NeighborBond_Put (xtr_p, atm_i, ii, BOND_SINGLE);
	}

      /* Convert stereo information */
      switch (bond_p->stereo) 
	{
	case  DSP_BOND_STEREO_UP:       
	case  DSP_BOND_STEREO_OPP_UP:
	  Xtr_Attr_NeighborStereo_Put (xtr_p, atm_i, ii, BOND_DIR_UP);
	  break;
	case  DSP_BOND_STEREO_DOWN:
	case  DSP_BOND_STEREO_OPP_DOWN:
	  Xtr_Attr_NeighborStereo_Put (xtr_p, atm_i, ii, BOND_DIR_DOWN);
	  break;
	default:
	  Xtr_Attr_NeighborStereo_Put (xtr_p, atm_i, ii, BOND_DIR_INVALID);
	}
      } /* End for all neighbors */

    atom_p++;
    } /* End for all atoms */

  return (xtr_p);
}

/********************************************
**       dsp_Shelley
*********************************************/
Boolean_t dsp_Shelley (Dsp_Molecule_t *mol_p)
{
  Dsp_Atom_t  *atom_p;
  long int     adj_list[DSP_MAXNUM_ATMS][6]; /* Adj list for the molecule */
  float        xarray[DSP_MAXNUM_ATMS];      /* The output coord arrays */
  float        yarray[DSP_MAXNUM_ATMS];      
  float        minx, maxy;
  float        miny, maxx;
  int          atm_i, ngb_i;
  long int     atm_num;

  atm_num = (long int) mol_p->natms;
  minx =  1000.0;
  maxy = -1000.0;
  miny =  1000.0;
  maxx = -1000.0;


  /* Initialize adjacency list */
  for (atm_i = 0; atm_i < atm_num; atm_i++)
    for (ngb_i = 0; ngb_i < 6; ngb_i++)
      adj_list[atm_i][ngb_i] = 0;

  /* Store info about adjacency into adj_list.
     Since Fortran stores arrays differently from C,
     store atom numbers incremented.
   */
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < atm_num; atm_i++) 
    {
    for (ngb_i = 0; ngb_i < atom_p->adj_info.num_neighbs; ngb_i++)
      adj_list[atm_i][ngb_i] = atom_p->adj_info.adj_atms[ngb_i] + 1;

    atom_p++;
    }

  /* Calculate new coords, using Shelley code */
  draw2_ (&adj_list[0][0], &atm_num, xarray, yarray);

  /* If first 3 coords are zero, 
     Shelley code couldn't handle transformation. Return FALSE
   */
  if ((xarray[0] == 0) && (xarray[1] == 0) && (xarray[2] == 0) 
      && (yarray[0] == 0) && (yarray[1] == 0) && (yarray[2] == 0))
    return (FALSE);

  /* Calculate min/max coordinates of the molecule */
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    if (minx > xarray[atm_i]) minx = xarray[atm_i];
    if (maxx < xarray[atm_i]) maxx = xarray[atm_i];
    if (miny > yarray[atm_i]) miny = yarray[atm_i];
    if (maxy < yarray[atm_i]) maxy = yarray[atm_i];
    }

  mol_p->molw = (Dimension) ((maxx - minx) * SHELLY2DSP_SCALE);
  mol_p->molh = (Dimension) ((maxy - miny) * SHELLY2DSP_SCALE);

  /* Convert cortesian xy-coordinates into Motif coordinates.
     Store old coordinates of atoms in xarray and yarray.
   */
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    atom_p->x = (int) ((xarray[atm_i] - minx) * SHELLY2DSP_SCALE);
    atom_p->y = (int) ((maxy - yarray[atm_i]) * SHELLY2DSP_SCALE);
    atom_p++;
    }


  /*  Look for coordinate overlap---shift y-coordinate a bit.  */
  atom_p = mol_p->atoms;
  for (atm_i = 0; atm_i < mol_p->natms; atm_i++) 
    {
    for (ngb_i = atm_i + 1; ngb_i < mol_p->natms; ngb_i++) 
      {
      if ((atom_p->x == (mol_p->atoms + ngb_i)->x) &&
	  (atom_p->y == (mol_p->atoms + ngb_i)->y))
	{
	(mol_p->atoms + ngb_i)->y += DSP_SHELLEY_SHIFT;
	if ((mol_p->atoms + ngb_i)->y > mol_p->molh)
	  mol_p->molh = (Dimension) (mol_p->atoms + ngb_i)->y;
	}
      }

    atom_p++;
    }

  return (TRUE);
}
