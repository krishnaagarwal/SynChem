/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     DOTS.C
*
*    Provides for dotting of nodes, which is a means of eliminating
*    potentially time-consuming duplicate mappings.
*
*  Creation Date:
*
*    16-Feb-2000
*
*  Authors:
*
*    Gerald A. Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <stdio.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_DOTS_
#include "dots.h"
#endif

#ifndef _H_CEDOT_
#include "cedot.h"
#endif

void dots (Xtr_t *gxtr, Xtr_t *sxtr, U32_t *roots, char *buffer)
{
  int i, j, n, deg, nbr[MX_NEIGHBORS], num_nbrs;

  n = Xtr_NumAtoms_Get (gxtr);
  for (i = 0; i < n; i++)
    Xtr_AttrFlags_Dot_Put (gxtr, i, FALSE);
  for (i = 0; i < n; i++)
    {
    deg = Xtr_Attr_NumNeighbors_Get (gxtr, i);
    if (deg > 1)
      {
      for (j = 0; j < MX_NEIGHBORS; j++) nbr[j] = 0;
      num_nbrs = 0;
      find_all_monovalent_neighbors (gxtr, i, deg, nbr, &num_nbrs);
      dot_constants (gxtr, nbr, num_nbrs, buffer);
      dot_pairs (gxtr, sxtr, i, nbr, num_nbrs, buffer);
      dot_remainder (gxtr, sxtr, i, deg, nbr, num_nbrs, buffer);
      }
    }
/* Has to wait until all the ignored source code gets converted!
*/
  cedot (gxtr, sxtr, roots, buffer);
/**/

  for (i = 0; i < MX_ROOTS && roots[i] != REACT_NODE_INVALID; i++)
    Xtr_AttrFlags_Dot_Put (gxtr, roots[i], FALSE);
}

void find_all_monovalent_neighbors (Xtr_t *gx, int a, int deg, int *nb, int *num)
{
  int i, j;

/*
    FOR NODE "A" COUNT AND RECORD IN THE ARRAY "NB" ALL SINGLY
    CONNECTED NEIGHBORS.
                                                              */
  for (i = 0; i < deg; i++)
    {
    j = Xtr_Attr_NeighborId_Get (gx, a, i);
    if (Xtr_Attr_NumNeighbors_Get (gx, j) == 1) nb[(*num)++] = j;
    }
}

void dot_constants (Xtr_t *gx, int *nb, int num, char *buf)
{
  int i, id;

/*
    DOT ALL SINGLY CONNECTED CONSTANT ATOMS IN THE PATTERN.
                                                            */
  for (i = 0; i < num; i++) if ((id = Xtr_Attr_Atomid_Get (gx, nb[i])) < VARIABLE_START || id > VARIABLE_END)
    {
    Xtr_AttrFlags_Dot_Put (gx, nb[i], TRUE);
    buf += strlen(buf);
    sprintf (buf, "NODE %d IS DOTTED BECAUSE IT IS A CONSTANT ATOM.\n", nb[i]);
    }
}

void dot_pairs (Xtr_t *gx, Xtr_t *sx, int a, int *nb, int num, char *buf)
{
  int i, j, deg1, deg2;

/*
    DOT ALL PAIRS OF SINGLY CONNECTED NODES AROUND A MULTIVALENT
    NODE IF THE PAIR FULFILLS ANY OF THE FOLLOWING CONDITIONS:
         1.  IF THEY ARE BOTH DISCONNECTED IN THE SUBGOAL
         2.  IF THEY ARE BOTH CONNECTED TO THE SAME NODE IN THE SUBGOAL
         3.  IF THEY ARE EACH CONNECTED TO THE SAME KIND OF ATOM IN
             THE SUBGOAL (THIS HAS BEEN EXTENDED TO INCLUDE ONE OR
             TWO OTHER ATOMS).
                                                                  */
  for (i = 0; i < num - 1; i++)
    {
    deg1 = Xtr_Attr_NumNeighbors_Get (sx, nb[i]);
    for (j = i + 1; j < num; j++)
      {
      deg2 = Xtr_Attr_NumNeighbors_Get (sx, nb[j]);
      if (deg1 == 0  && deg2 == 0)
        {
        Xtr_AttrFlags_Dot_Put (gx, nb[i], TRUE);
        Xtr_AttrFlags_Dot_Put (gx, nb[j], TRUE);
        buf += strlen(buf);
        sprintf (buf, "NODES %d AND %d ARE DOTTED BECAUSE THEY ARE BOTH UNCONNECTED IN THE SUBGOAL.\n", nb[i], nb[j]);
        }
      else if (deg1 == 1 && deg2 == 1)
        {
        if (Xtr_Attr_NeighborId_Get (sx, nb[i], 0) == Xtr_Attr_NeighborId_Get (sx, nb[j], 0) &&
          Xtr_Attr_NeighborBond_Get (sx, nb[i], 0) == Xtr_Attr_NeighborBond_Get (sx, nb[j], 0))
          {
          Xtr_AttrFlags_Dot_Put (gx, nb[i], TRUE);
          Xtr_AttrFlags_Dot_Put (gx, nb[j], TRUE);
          buf += strlen(buf);
          sprintf (buf, "NODES %d AND %d ARE DOTTED BECAUSE THEY ARE CONNECTED TO THE SAME ATOM IN THE SUBGOAL.\n", nb[i], nb[j]);
          }
        else if (same_kind_of_atom (sx, nb, num, i, j))
          {
          Xtr_AttrFlags_Dot_Put (gx, nb[i], TRUE);
          Xtr_AttrFlags_Dot_Put (gx, nb[j], TRUE);
          buf += strlen(buf);
          sprintf (buf, "NODES %d AND %d ARE DOTTED BECAUSE THEY ARE CONNECTED TO THE SAME KIND OF ATOM OR ATOMS IN THE SUBGOAL.\n",
            nb[i], nb[j]);
          }
        }
      }
    }
}

Boolean_t same_kind_of_atom (Xtr_t *sx, int *nb, int num, int x, int y)
{
  int n[2], nn[2], deg1, deg2, i, j, k;

  if (Xtr_Attr_NeighborBond_Get (sx, nb[x], 0) != Xtr_Attr_NeighborBond_Get (sx, nb[y], 0)) return (FALSE);

  n[0] = Xtr_Attr_NeighborId_Get (sx, nb[x], 0);
  n[1] = Xtr_Attr_NeighborId_Get (sx, nb[y], 0);

  if (Xtr_Attr_Atomid_Get (sx, n[0]) != Xtr_Attr_Atomid_Get (sx, n[1])) return (FALSE);

  deg1 = Xtr_Attr_NumNeighbors_Get (sx, n[0]);
  deg2 = Xtr_Attr_NumNeighbors_Get (sx, n[1]);

  if (deg1 != deg2) return (FALSE);
  if (deg1 == 1 && deg2 == 1) return (TRUE);
  if (deg1 > 2 || deg2 > 2) return (FALSE);

  nn[0] = nb[x];
  nn[1] = nb[y];

  for (i = 0; i < 2; i++) for (j = 0; j < 2; j++)
    {
    k = Xtr_Attr_NeighborId_Get (sx, n[i], j);
    if (k != nb[x] && k != nb[y]) nn[i] = k;
    }

  return (Xtr_Attr_Atomid_Get (sx, nn[0]) == Xtr_Attr_Atomid_Get (sx, nn[1]));
}

void dot_remainder (Xtr_t *gx, Xtr_t *sx, int a, int deg, int *nb, int num, char *buf)
{
  int i, j, k, id;

/*
    ANY PASSIVE VARIABLE NODES THAT HAVE NOT PREVIOUSLY BEEN DOTTED
    MAY BE DOTTED IF THE ONLY ACTIVE BONDS TO THE ATOM TO WHICH THEY
    ARE CONNECTED ARE BONDS TO DOTTED CONSTANT NODES.
                                                                  */
  for (i = 0; i < deg; i++)
    {
    j = Xtr_Attr_NeighborId_Get (gx, a, i);
/*
    for (k = 0; Xtr_Attr_NeighborId_Get (sx, a, k) != j; k++);
*/
    if (!dotted_constant (gx, nb, num, j) &&
/*
      Xtr_Attr_NeighborBond_Get (gx, a, i) != Xtr_Attr_NeighborBond_Get (sx, a, k)) return;
*/
      Xtr_Attr_NeighborBond_Find (gx, a, j) != Xtr_Attr_NeighborBond_Find (sx, a, j)) return;
    }

  for (i = 0; i < num; i++)
    if (!Xtr_AttrFlags_Dot_Get (gx, nb[i]) && (id = Xtr_Attr_Atomid_Get (gx, nb[i])) >= VARIABLE_START && id <= VARIABLE_END &&
    Xtr_Attr_NumNeighbors_Get (sx, nb[i]) == 1) /* UNDOTTED VARIABLE NODE */
    if (Xtr_Attr_NeighborBond_Get (gx, nb[i], 0) == Xtr_Attr_NeighborBond_Get (sx, nb[i], 0) &&
    Xtr_Attr_NeighborId_Get (sx, nb[i], 0) == a) /* PASSIVE BOND */
    {
    Xtr_AttrFlags_Dot_Put (gx, nb[i], TRUE);
    buf += strlen(buf);
    sprintf (buf, "NODE %d IS DOTTED BECAUSE IT IS NOT CONNECTED TO THE PATTERN BY AN ACTIVE BOND.\n", nb[i]);
    }
}

Boolean_t dotted_constant (Xtr_t *gx, int *nb, int num, int x)
{
  int i, id;

  for (i = 0; i < num; i++) if (nb[i] == x && Xtr_AttrFlags_Dot_Get (gx, x) &&
    ((id = Xtr_Attr_Atomid_Get (gx, x)) < VARIABLE_START || id > VARIABLE_END)) return (TRUE);

  return (FALSE);
}
