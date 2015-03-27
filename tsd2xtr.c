/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     TSD2XTR.C
*
*    This module contains the routines for converting TSDs to XTRs and
*    vice-versa.  These routines are pulled out in order to simplify the
*    linking procedures.
*
*  Routines:
*
*    Tsd2Xtr
*    Xtr2Tsd
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
* 27-Feb-95  Krebsbach  Bug fix:  Tsd2Xtr, line 147:  changed 
*                       Xtr_Attr_NeighborBond_Get to Xtr_Attr_NeighborId_Get
*                       in indexing dotted neighbor.
* 27-Jun-95  Cheung     Added Xtr_Attr_ResonantBonds_Set in routine Xtr2Tsd.
*
******************************************************************************/

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif


/****************************************************************************
*
*  Function Name:                 Tsd2Xtr
*
*    This routine converts a TSD representation of a molecule to an XTR
*    representation.
*
*  Used to be:
*
*    tsdxtr:, $tsdxtr:
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
*    Address of allocated XTR_t
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Xtr_t *Tsd2Xtr
  (
  Tsd_t        *tsd_p                      /* Molecule to change rep. of */
  )
{
  Xtr_t        *xtr_tmp;                   /* XTR ptr */
  U16_t         num_atoms;                 /* Number of atoms in molecule */
  U16_t         i, j, n, m;                /* Counters */
  U16_t         tempray[3][MX_NEIGHBORS];  /* For reordering neighbor vector */
  U16_t         tempindex[MX_NEIGHBORS];   /* For manipulating neighbors */
  Boolean_t     must_reorder;              /* Flag - bond reordering */
  
  if (tsd_p == NULL)
    return NULL;

  DEBUG (R_TSD, DB_TSD2XTR, TL_PARAMS, (outbuf,
    "Entering Tsd2Xtr, tsd = %p", tsd_p));

  must_reorder = FALSE;
  num_atoms = Tsd_NumAtoms_Get (tsd_p);
  xtr_tmp = Xtr_Create (num_atoms);

  for (i = 0; i < num_atoms; i++)
    {
    for (j = 0, n = 0; j < MX_NEIGHBORS; j++)
      {
      if (Tsd_Atom_NeighborId_Get (tsd_p, i, j) != TSD_INVALID)
        {
        Xtr_Attr_NeighborId_Put (xtr_tmp, i, n,
          Tsd_Atom_NeighborId_Get (tsd_p, i, j));
        Xtr_Attr_NeighborBond_Put (xtr_tmp, i, n,
          Tsd_Atom_NeighborBond_Get (tsd_p, i, j));
        Xtr_Attr_NeighborStereo_Put (xtr_tmp, i, n, j);
        n++;
        }
      }
      Xtr_Attr_NumNeighbors_Put (xtr_tmp, i, n);
      Xtr_Attr_Atomid_Put (xtr_tmp, i, Tsd_Atomid_Get (tsd_p, i));
      Xtr_AttrFlags_DontCare_Put (xtr_tmp, i, Tsd_AtomFlags_DontCare_Get (
        tsd_p, i));
      Xtr_AttrFlags_Asymmetry_Put (xtr_tmp, i, Tsd_AtomFlags_Asymmetry_Get (
        tsd_p, i));
      Xtr_AttrFlags_Dot_Put (xtr_tmp, i, Tsd_AtomFlags_Dot_Get (tsd_p, i));

      /* if any dots are present we must reorder bonds */

      if (Tsd_AtomFlags_Dot_Get (tsd_p, i) == TRUE)
        must_reorder = TRUE;
    }                                      /* End of all atoms for-loop */
  /* Now if any dots are present we must reorder neighbor fields to be
     sure all dotted nodes are last in adjacency list.  this is
     necessary if the dots are to aid in the reduction of
     the number of matches obtained from an xtr of a pattern graph
  */

  if (must_reorder == TRUE)
    for (i = 0; i < num_atoms; i++)
      {
      /* we first locate a suitable ordering by scanning the adjacency list */

      n = 0;                               /* used as an index */
      m = Xtr_Attr_NumNeighbors_Get (xtr_tmp, i) - 1;    /* used as an index */
      for (j = 0; j < Xtr_Attr_NumNeighbors_Get (xtr_tmp, i); j++)
        {
        /* dotted neighbors put at end of list */

        if (Xtr_AttrFlags_Dot_Get (xtr_tmp,
            Xtr_Attr_NeighborId_Get (xtr_tmp, i, j)) == TRUE)
          {
          tempindex[m] = j;
          m--;
          }
        else
          {
          /* undotted neighbors put at beginning of list */
          tempindex[n] = j;
          n++;
          }
        }

      /* Now we know a proper order for this adjacency list.
         we simply put the neighbor, bond multiplicity, and
         stereo vectors in the order indicated by tempindex.
      */

      for (j = 0; j < MX_NEIGHBORS; j++)
        {
        tempray[0][j] = Xtr_Attr_NeighborId_Get (xtr_tmp, i, j);
        tempray[1][j] = Xtr_Attr_NeighborBond_Get (xtr_tmp, i, j);
        tempray[2][j] = Xtr_Attr_NeighborStereo_Get (xtr_tmp, i, j);
        }

      for (j = 0; j < Xtr_Attr_NumNeighbors_Get (xtr_tmp, i); j++)
        {
        Xtr_Attr_NeighborId_Put (xtr_tmp, i, j, tempray[0][tempindex[j]]);
        Xtr_Attr_NeighborBond_Put (xtr_tmp, i, j, tempray[1][tempindex[j]]);
        Xtr_Attr_NeighborStereo_Put (xtr_tmp, i, j, tempray[2][tempindex[j]]);
        }
      }                                    /* End-for atoms loop */

  TRACE_DO (DB_TSD2XTR, TL_MAJOR,
    {
    IO_Put_Trace (R_TSD, "In Tsd2Xtr, input TSD is");
    Tsd_Dump (tsd_p, &GTraceFile);
    IO_Put_Trace (R_TSD, "In Tsd2Xtr, output XTR is");
    Xtr_Dump (xtr_tmp, &GTraceFile);
    });

  DEBUG (R_TSD, DB_TSD2XTR, TL_PARAMS, (outbuf,
    "Leaving Tsd2Xtr, Xtr = %p", xtr_tmp));

  return xtr_tmp;
}

/****************************************************************************
*
*  Function Name:                 Xtr2Tsd
*
*    This function converts a molecule in XTR format to TSD format.
*
*  Used to be:
*
*    xtrtsd:
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
*    Address of TSD
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Tsd_t *Xtr2Tsd
  (
  Xtr_t        *xtr_p                      /* Molecule to convert */
  )
{
  Tsd_t        *tsd_p;                     /* Temporary */
  U16_t         atom;                      /* Counter */
  U8_t          neigh;                     /* Counter */
  U8_t          num_neighbors;             /* ??? Extremely broken compiler! */

  DEBUG (R_XTR, DB_XTR2TSD, TL_PARAMS, (outbuf,
    "Entering Xtr2Tsd, Xtr = %p", xtr_p));

  tsd_p = Tsd_Create (Xtr_NumAtoms_Get (xtr_p));

  for (atom = 0; atom < Xtr_NumAtoms_Get (xtr_p); atom++)
    {
    if (Xtr_AttrFlags_Asymmetry_Get (xtr_p, atom) == TRUE)
      Tsd_AtomFlags_Asymmetry_Put (tsd_p, atom, TRUE);

    if (Xtr_AttrFlags_DontCare_Get (xtr_p, atom) == TRUE)
      Tsd_AtomFlags_DontCare_Put (tsd_p, atom, TRUE);

    if (Xtr_AttrFlags_Dot_Get (xtr_p, atom) == TRUE)
      Tsd_AtomFlags_Dot_Put (tsd_p, atom, TRUE);

    Tsd_Atomid_Put (tsd_p, atom, Xtr_Attr_Atomid_Get (xtr_p, atom));

    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);
    for (neigh = 0; neigh < num_neighbors; neigh++)
      {
      switch (Xtr_Attr_NeighborStereo_Get (xtr_p, atom, neigh))
        {
        case BOND_DIR_UP :

          Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_UP,
            Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh));
          Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_UP,
            Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh));
          break;

        case BOND_DIR_DOWN :

          Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_DOWN,
            Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh));
          Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_DOWN,
            Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh));
          break;

        case BOND_DIR_LEFT :

          Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_LEFT,
            Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh));
          Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_LEFT,
            Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh));
          break;

        case BOND_DIR_RIGHT :

          Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_RIGHT,
            Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh));
          Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_RIGHT,
            Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh));
          break;

        case BOND_DIR_IN :

          Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_IN,
            Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh));
          Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_IN,
            Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh));
          break;

        case BOND_DIR_OUT :

          Tsd_Atom_NeighborBond_Put (tsd_p, atom, BOND_DIR_OUT,
            Xtr_Attr_NeighborBond_Get (xtr_p, atom, neigh));
          Tsd_Atom_NeighborId_Put (tsd_p, atom, BOND_DIR_OUT,
            Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh));
          break;

        default :

          IO_Exit_Error (R_XTR, X_SYNERR,
            "Invalid Bond Stereo value found in Xtr in Xtr2Tsd");
          break;
        }
      }
    }

  TRACE_DO (DB_XTR2TSD, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In Xtr2Tsd, compare conversion");
    Xtr_Dump (xtr_p, &GTraceFile);
    Tsd_Dump (tsd_p, &GTraceFile);
    });

  DEBUG (R_XTR, DB_XTR2TSD, TL_PARAMS, (outbuf,
    "Leaving Xtr2Tsd, Tsd = %p", tsd_p));

  return tsd_p;
}
/* End of Xtr2Tsd */
/* End of Tsd2Xtr.c */
