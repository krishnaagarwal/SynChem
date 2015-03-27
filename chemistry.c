/******************************************************************************
*
*  Copyright (C) 1993-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     CHEMISTRY.C
*
*    This module is the repository of as much of the chemistry knowledge that
*    is encoded algorithmically into SYNCHEM.  The Reaction Database contains
*    the rules, Atomsym.C contains relevant stuff, Resonantbonds.C is chemistry
*    encodings, but information such as an Alcohol is related to a Foo should
*    go in here.
*
*  Routines:
* 
*    xxx
*
*  Creation Date:
*
*    01-Jan-1993
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
* 19-Apr-95  Cheung	Ether and alcohol groups are checked in 
*			Xtr_FuncGrp_Instance_IsValid
*
******************************************************************************/

#include "synchem.h"
#include "synio.h"
#include "debug.h"

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/* was instok: */
Boolean_t Xtr_FuncGrp_Instance_IsValid
  (
  Xtr_t        *xtr_p,                      /* Molecule to check */
  U16_t         fg_num,                     /* FG # to check */
  U16_t         instance                    /* Which instance is this */
  )
{
  FuncGroups_t *funcgrp_p;                  /* Temporary - FG type */
  U16_t         num_instances;              /* # instances for this FG */
  U16_t         i, j;                       /* Counter */
  U16_t         inst_root;                  /* Root atom for instance index */
  U16_t         atom, nid;                  /* Atom index of neighbor */
  U16_t         neigh;                      /* Atomid of neighbor of neighbr */
  Boolean_t     found;

  /* The original instok checked (below), but this should be covered by the
     functional group finding algorithm.
     - Alcohols for carboxylic acids
     - Ethers for carboxylic esters
  */

  funcgrp_p = Xtr_FuncGroups_Get (xtr_p);
  num_instances = FuncGrp_SubstructureCount_Get (funcgrp_p, fg_num);
  inst_root = FuncGrp_SubstructureInstance_Get (funcgrp_p, fg_num, instance);

  switch (fg_num)
    {
  case CARBONYL:

    /* Check carbonyl carbon of aldehyde or ketone for valid configuration.
       If passes test then must be a hetero-atom in the slot.
    */

    atom = Xtr_Attr_NeighborId_Get (xtr_p, inst_root, BOND_DIR_UP);
    for (i = 0; i < 3; i++)
      {
      nid = Xtr_Attr_NeighborId_Get (xtr_p, atom, i);

      neigh = Xtr_Attr_Atomid_Get (xtr_p, nid);

      if (neigh == XTR_INVALID)    /* Ketene */
        return TRUE;

      if ((neigh == OXYGEN) && (Xtr_Attr_NeighborBond_Get (xtr_p, atom, i) !=
          BOND_DOUBLE))
        {
        for (j = 0, found = FALSE; j < 2 && !found; j++)
          found = Xtr_Attr_Atomid_Get (xtr_p, Xtr_Attr_NeighborId_Get (xtr_p, nid, j)) == SILICON;
/* Special case until organosilicon chapter can be added */
        if (!found) return FALSE;
        }

      if ((neigh != HYDROGEN) && (neigh != OXYGEN) && (neigh != CARBON))
        return FALSE;
      }

    break;

  case CN_SINGLE :

    for (i = 1; i <= num_instances; i++)
      if (inst_root == FuncGrp_SubstructureInstance_Get (funcgrp_p, 
          CARBOXYLIC_AMIDE, i))
        return FALSE;

    break;

  case ALCOHOL :

    for (i = 1; i <= num_instances; ++i)
      if (inst_root == FuncGrp_SubstructureInstance_Get (funcgrp_p, 
          CARBOXYLIC_ACID, i))
        return FALSE;

    break;

  case ETHER :

    for (i = 1; i <= num_instances; ++i)
      if (inst_root == FuncGrp_SubstructureInstance_Get (funcgrp_p, 
          CARBOXYLIC_ESTER, i))
        return FALSE;

    break;

  default :

    break;
    }

  return TRUE;

}
