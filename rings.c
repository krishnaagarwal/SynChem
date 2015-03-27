/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RINGS.C
*
*    This module is the abstraction for the Rings sub data-structure for the
*    XTR.  The rings are comprised of a list of ring systems and the components
*    of the rings.
*
*  Routines:
*
*    Ring_AtomIn
*    Ring_AtomInSpecific
*    Ring_Bonds_Set
*    Rings_Copy
*    Rings_Create
*    Rings_Destroy
*    Rings_Dump
*    Ring_Systems_Find
*    SBiconnect
*    SComponent_Add
*    SEdge_Create
*    SEdge_Destroy
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
* 27-Aug-01  Miller     Corrected misuse of Array_Copy() in Rings_Copy().
*
******************************************************************************/

#include <string.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_RINGSYSTEMS_
#include "ringsystems.h"
#endif

#ifndef _H_RINGS_
#include "rings.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/* Static routines */

static void    SComponent_Add (Xtr_t *, Ringsys_t *, Stack_t *);
static void    SBiconnect     (Xtr_t *, Rings_t *, Ringsys_t **, U16_t *, U16_t,
  U16_t, U16_t *, Array_t *, Array_t *, Stack_t *);
static Edge_t *SEdge_Create   (U16_t, U16_t);

/*  static void    SEdge_Destroy  (Edge_t *);  */

/****************************************************************************
*
*  Function Name:                 Ring_AtomIn
*
*    This function simply checks to see if an atom is in a ring, any ring.
*
*  Used to be:
*
*    inring:
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
*    Boolean_t as to whether the specified atom is a member of a ring system
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Ring_AtomIn
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  U16_t         atom                      /* Which atom to check */
  )
{
  Rings_t      *ring_p;                   /* Temporary */

  ring_p = Xtr_Rings_Get (xtr_p);
  if (ring_p == NULL)
    return FALSE;

  if (atom == XTR_INVALID)
    return FALSE;
  else
    return Ring_RingBit_Get (ring_p, atom);
}

/****************************************************************************
*
*  Function Name:                 Ring_AtomInSpecific
*
*    This function checks to see if the specified atom is in the specfied
*    ring.  Returns true if it is, false otherwise.  The function does NOT
*    check to see if the specified ring is in the list, that is the 
*    caller's responsibility.
*
*  Used to be:
*
*    inrnsys:
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
*    Boolean_t as to whether the atom is in a specific ring system
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Ring_AtomInSpecific
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  U16_t         num,                      /* Ring system index */
  U16_t         atom                      /* Atom to check */
  )
{
  Rings_t      *rings_p;                  /* Temporary */
  Ringsys_t    *ringsys_p;                /* Temporary */
  U16_t         i;                        /* Counter */

  rings_p = Xtr_Rings_Get (xtr_p);
  if (Ring_NumRingSystems_Get (rings_p) == 0)
    return FALSE;

  ringsys_p = Ring_RingsysList_Get (rings_p);
  for (i = 0; i < num; i++)
    {
    ASSERT (ringsys_p != NULL,
      {
      IO_Exit_Error (R_XTR, X_SYNERR,
        "Ran off end of Ring System list in Ring_AtomInSpecific");
      });

    ringsys_p = Ringsys_Next_Get (ringsys_p);
    }

  return Ringsys_Isin_Get (ringsys_p, atom);
}

/****************************************************************************
*
*  Function Name:                 Ring_Bonds_Set
*
*    This routine fills in the bond array with a mask of which bonds
*    participate in the specified ring.
*
*  Used to be:
*
*    ringsys:
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
void Ring_Bonds_Set
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  U16_t         num,                      /* Which ring to get bonds of */
  Array_t      *bonds_p                   /* 2-d bit, for bond mask */
  )
{
  Ringsys_t    *ringsys_p;                /* Temporary */
  Rings_t      *ring_p;                   /* Temporary */
  U16_t         i, j;                     /* Counters */

  DEBUG (R_XTR, DB_RINGBONDSET, TL_PARAMS, (outbuf,
    "Entering Ring_Bonds_Set, Xtr = %p, ring index = %u, bonds = %p",
    xtr_p, num, bonds_p));

  ring_p = Xtr_Rings_Get (xtr_p);
  if (ring_p == NULL)
    Array_Set (bonds_p, FALSE);
  else
    {
    for (i = 0, ringsys_p = Ring_RingsysList_Get (ring_p); i < num; i++)
      ringsys_p = Ringsys_Next_Get (ringsys_p);

    for (i = 0; i < Ringsys_NumAtoms_Get (ringsys_p); i++)
      for (j = 0; j < MX_NEIGHBORS; j++)
        Array_2d1_Put (bonds_p, i, j, Ringsys_Component_Get (ringsys_p, i, j));
    }

  DEBUG (R_XTR, DB_RINGBONDSET, TL_PARAMS, (outbuf,
    "Leaving Ring_Bonds_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Rings_Copy
*
*    This function copies a Rings_t data-structure.
*
*  Used to be:
*
*    nrings:
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
*    Address of copy of Rings data-structure
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Rings_t *Rings_Copy
  (
  Rings_t      *ring_p                    /* Ring descriptor to copy */
  )
{
  Ringsys_t    *ringsys_from;             /* Temporary */
  Ringsys_t    *ringsys_tmp;              /* Temporary */
  Ringsys_t    *ringsys_to;               /* Temporary */
  Rings_t      *ring_tmp;                 /* Temporary */
  U16_t         num_atoms;                /* # atoms in m'cule */
  U16_t         i;                        /* Counter */

  if (ring_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_RINGCOPY, TL_PARAMS, (outbuf,
    "Entering Rings_Copy, Ring = %p", ring_p));

  num_atoms = Ring_NumAtoms_Get (ring_p);
  ring_tmp = Rings_Create (num_atoms, FALSE);

/*
#ifdef _MIND_MEM_
  mind_Array_Copy ("Ring_RingBitHandle_Get(ring_tmp)", "rings{1}", Ring_RingBitHandle_Get (ring_p), Ring_RingBitHandle_Get (
    ring_tmp));
#else
  Array_Copy (Ring_RingBitHandle_Get (ring_p), Ring_RingBitHandle_Get (
    ring_tmp));
#endif
*/

/* The above is a misuse of Array_Copy().  The array already exists, because Rings_Create() creates it.  The correct call appears
   below: */

  Array_CopyContents (Ring_RingBitHandle_Get (ring_p), Ring_RingBitHandle_Get (ring_tmp));

  Ring_NumRingSystems_Put (ring_tmp, Ring_NumRingSystems_Get (ring_p));

  /* Copy list of ring systems */

  ringsys_from = Ring_RingsysList_Get (ring_p);
  for (i = 0, ringsys_tmp = NULL; i < Ring_NumRingSystems_Get (ring_p); i++)
    {
    ringsys_to = Ringsys_Copy (ringsys_from);
    Ringsys_Next_Put (ringsys_to, ringsys_tmp);
    ringsys_tmp = ringsys_to;
    ringsys_from = Ringsys_Next_Get (ringsys_from);
    }

  /* At this point the list has been copied but the copy is in reverse order;
     reverse the order of this list.
  */

  ringsys_from = ringsys_tmp;
  for (i = 0; i < Ring_NumRingSystems_Get (ring_tmp); i++)
    {
    ringsys_tmp = Ringsys_Next_Get (ringsys_from);
    Ringsys_Next_Put (ringsys_from, Ring_RingsysList_Get (ring_tmp));
    Ring_RingsysList_Put (ring_tmp, ringsys_from);
    ringsys_from = ringsys_tmp;
    }

  DEBUG (R_XTR, DB_RINGCOPY, TL_PARAMS, (outbuf,
    "Leaving Rings_Copy, Ring = %p", ring_tmp));

  return ring_tmp;
}

/****************************************************************************
*
*  Function Name:                 Rings_Create
*
*    This function creates a new Rings data-structure.
*
*  Used to be:
*
*    N/A:
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
*    Address of new Rings data-structure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Rings_t *Rings_Create
  (
  U16_t         num_atoms,                /* Number of atoms in m'cule */
  Boolean_t     init                      /* Initial value for ring bits */
  )
{
  Rings_t      *ring_tmp;                 /* Temporary */

  DEBUG (R_XTR, DB_RINGCREATE, TL_PARAMS, (outbuf,
    "Entering Rings_Create, # atoms = %u, init value = %hu", num_atoms, init));

#ifdef _MIND_MEM_
  mind_malloc ("ring_tmp", "rings{2}", &ring_tmp, RINGSSIZE);
#else
  Mem_Alloc (Rings_t *, ring_tmp, RINGSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Rings in Rings_Create at %p", ring_tmp));

  if (ring_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL, "No memory for a Rings in Rings_Create");

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("Ring_RingBitHandle_Get(ring_tmp)", "rings{2}", Ring_RingBitHandle_Get (ring_tmp), num_atoms, BITSIZE);
#else
  Array_1d_Create (Ring_RingBitHandle_Get (ring_tmp), num_atoms, BITSIZE);
#endif
  Array_Set (Ring_RingBitHandle_Get (ring_tmp), init);

  Ring_NumRingSystems_Put (ring_tmp, 0);
  Ring_RingsysList_Put (ring_tmp, NULL);

  DEBUG (R_XTR, DB_RINGCREATE, TL_PARAMS, (outbuf,
    "Leaving Rings_Create, Ring = %p", ring_tmp));

  return ring_tmp;
}

/****************************************************************************
*
*  Function Name:                 Rings_Destroy
*
*    This function deallocates an Rings data-structure.
*
*  Used to be:
*
*    N/A:
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
void Rings_Destroy
  (
  Rings_t      *ring_p                    /* Ring to destroy */
  )
{
  Ringsys_t    *ringsys_tmp;              /* For Ring System List traversal */

  if (ring_p == NULL)
    return;

  DEBUG (R_XTR, DB_RINGDESTROY, TL_PARAMS, (outbuf,
    "Entering Rings_Destroy, Ring = %p", ring_p));

  for (ringsys_tmp = Ring_RingsysList_Get (ring_p); ringsys_tmp != NULL;
       ringsys_tmp = Ring_RingsysList_Get (ring_p))
    {
    Ring_RingsysList_Put (ring_p, Ringsys_Next_Get (ringsys_tmp));
    Ringsys_Destroy (ringsys_tmp);
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("Ring_RingBitHandle_Get(ring_p)", "rings", Ring_RingBitHandle_Get (ring_p));
#else
  Array_Destroy (Ring_RingBitHandle_Get (ring_p));
#endif

  DEBUG_DO (DB_RINGDESTROY, TL_MEMORY,
    {
    memset (ring_p, 0, RINGSSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("ring_p", "rings", ring_p);
#else
  Mem_Dealloc (ring_p, RINGSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for a Rings in Rings_Destroy at %p", ring_p));

  DEBUG (R_XTR, DB_RINGDESTROY, TL_PARAMS, (outbuf,
    "Leaving Rings_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Rings_Dump
*
*    This routine prints a formatted dump of a Rings descriptor.
*
*  Used to be:
*
*    prings:
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
void Rings_Dump
  (
  Rings_t       *ring_p,                     /* Instance to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */
  Ringsys_t     *t;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (ring_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Rings\n\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_RINGCREATE, ring_p);
  fprintf (f, 
    "Dump of Rings structure - # Atoms : %lu - RingBits of Ring structure\n",
    Ring_NumAtoms_Get (ring_p));

  Array_Dump (Ring_RingBitHandle_Get (ring_p), filed_p);

  fprintf (f, "Traverse list of Ring Systems\n");
  for (t = Ring_RingsysList_Get (ring_p); t != NULL; )
    {
    Ringsys_Dump (t, filed_p);
    t = Ringsys_Next_Get (t);
    }

  fprintf (f, "\n");

  return;
}

/****************************************************************************
*
*  Function Name:                 Ring_Systems_Find
*
*    This routine controls the building and recording of the ring systems.
*    But it doesn't do the actual computation itself.
*
*  Used to be:
*
*    frngsys:
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
void Ring_Systems_Find
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  Rings_t      *ring_p,                   /* Ring structure to manipulate */
  Ringsys_t   **ringsys_pp,               /* Ring system list accumulator */
  U16_t        *num_ringsys_p             /* Counter for list */
  )
{
  Stack_t      *stack_p;                  /* Stack of edges to search */
  U16_t         atom;                     /* Counter */
  U16_t         level;                    /* So can start someplace */
  Array_t       number;                   /* Scoring array */
  Array_t       lowpt;                    /* Same */

  DEBUG (R_XTR, DB_RINGSYSFIND, TL_PARAMS, (outbuf,
    "Entering Ring_Systems_Find, Xtr = %p, Ring = %p", xtr_p, ring_p));

  TRACE_DO (DB_RINGSYSFIND, TL_MAJOR,
    {
    IO_Put_Trace (R_XTR, "In Ring_Systems_Find");
    Xtr_Dump (xtr_p, &GTraceFile);
    });

  *num_ringsys_p = 0;
  level = 0;
  *ringsys_pp = NULL;

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&number", "rings{3}", &number, Xtr_NumAtoms_Get (xtr_p), WORDSIZE);
  mind_Array_1d_Create ("&lowpt", "rings{3}", &lowpt, Xtr_NumAtoms_Get (xtr_p), WORDSIZE);
#else
  Array_1d_Create (&number, Xtr_NumAtoms_Get (xtr_p), WORDSIZE);
  Array_1d_Create (&lowpt, Xtr_NumAtoms_Get (xtr_p), WORDSIZE);
#endif
  Array_Set (&number, 0);

  stack_p = Stack_Create (STACK_NORMAL);
  for (atom = 0; atom < Xtr_NumAtoms_Get (xtr_p); atom++)
    if (Array_1d16_Get (&number, atom) == 0)
      SBiconnect (xtr_p, ring_p, ringsys_pp, num_ringsys_p, atom,
        0, &level, &number, &lowpt, stack_p);

  Stack_Destroy (stack_p);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&number", "rings", &number);
  mind_Array_Destroy ("&lowpt", "rings", &lowpt);
#else
  Array_Destroy (&number);
  Array_Destroy (&lowpt);
#endif

  DEBUG (R_XTR, DB_RINGSYSFIND, TL_PARAMS, (outbuf,
    "Leaving Ring_Systems_Find, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SBiconnect
*
*    This routine is derived from Robert Tarjan's algorithm for finding
*    biconnected components, described in the June 1972 issue of the
*    SIAM Journal of Computing.  The input is an XTR, the output is a one
*    way list of ring system structures.  Note that if two atoms are 
*    biconnected then they must be in a ring.
*
*  Used to be:
*
*    biconnect:
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
static void SBiconnect
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  Rings_t      *ring_p,                   /* Ring structure to manipulate */
  Ringsys_t   **ringsys_pp,               /* Ring system list (modified) */
  U16_t        *num_ringsys_p,            /* # ring systems found */
  U16_t         atom,                     /* An atom index */
  U16_t         prev,                     /* Also an atom index */
  U16_t        *level_p,                  /* Level in search */
  Array_t      *number_p,                 /* Accumulator array */
  Array_t      *lowpt_p,                  /* Same */
  Stack_t      *stack_p                   /* Stack of edges */
  )
{
  Ringsys_t   *ringsys_tmp;               /* New ring system */
  Edge_t       *edge_p;                   /* Temporary */
  U16_t        n, k, l;                   /* Counters */
  U16_t        num_neighbors;             /* # neighbors of an atom */
  U16_t        neigh_id;                  /* Neighbor atom id */
  U8_t         neigh_index;               /* Neighbor index counter */

  DEBUG (R_XTR, DB_RINGSTATIC, TL_PARAMS, (outbuf, 
    "Entering SBiconnect, xtr = %p, ring = %p, ring system = %p, num ringsys"
    " = %u, atom = %u, prev = %u, level = %u, number array  = %p,"
    " lowpt array = %p", xtr_p, ring_p, *ringsys_pp, *num_ringsys_p, atom, 
    prev, *level_p, number_p, lowpt_p));

  /* Note which level we are on, mark the current atom as being found at this
     level
  */

  (*level_p)++;
  Array_1d16_Put (number_p, atom, *level_p);
  Array_1d16_Put (lowpt_p, atom, *level_p);
  num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, atom);

  /* Loop through the current atom's neighbors.  If one is found that hasn't
     been looked at yet, push the edge crossed to reach it on the stack and
     recurse.
  */

  for (neigh_index = 0; neigh_index < num_neighbors; neigh_index++)
    {
    neigh_id = Xtr_Attr_NeighborId_Get (xtr_p, atom, neigh_index);
    if (Array_1d16_Get (number_p, neigh_id) == 0)
      {
      edge_p = SEdge_Create (atom, neigh_id);
      Stack_PushAdd (stack_p, edge_p);
      SBiconnect (xtr_p, ring_p, ringsys_pp, num_ringsys_p, neigh_id,
        atom, level_p, number_p, lowpt_p, stack_p);
      Array_1d16_Put (lowpt_p, atom, MIN (Array_1d16_Get (lowpt_p, atom),
        Array_1d16_Get (lowpt_p, neigh_id)));

      if (Array_1d16_Get (lowpt_p, neigh_id) >= Array_1d16_Get (number_p,
          atom))
        {
        if (Array_1d16_Get (number_p, Edge_First_Get ((Edge_t *)
            Stack_TopAdd (stack_p))) >= Array_1d16_Get (number_p, neigh_id))
          {
          /* Make sure to create the ring system only once for each cycle
             found
          */

          ringsys_tmp = Ringsys_Create (Xtr_NumAtoms_Get (xtr_p));
          Ringsys_Next_Put (ringsys_tmp, *ringsys_pp);
          *ringsys_pp = ringsys_tmp;
          }
        else
          {
          /* If no Ring System is allocated, then the rest should be skipped */

          ringsys_tmp = NULL;
          Stack_Pop (stack_p);
          continue;
          }

        /* Neigh has got a low 'number' due to being the one to close the
           cycle, so anything in between it and the root of the cycle
           must be in the cycle.  Add the last edge if it is on the stack.
        */

        n = 0;
        while (Array_1d16_Get (number_p, Edge_First_Get ((Edge_t *)
            Stack_TopAdd (stack_p))) >= Array_1d16_Get (number_p, neigh_id))
          {
          SComponent_Add (xtr_p, ringsys_tmp, stack_p);
          n++;
          }

        if (Stack_Size_Get (stack_p) > 0)
          if ((Edge_First_Get ((Edge_t *)Stack_TopAdd (stack_p)) == atom) &&
              (Edge_Second_Get ((Edge_t *)Stack_TopAdd (stack_p)) == neigh_id))
            {
            SComponent_Add (xtr_p, ringsys_tmp, stack_p);
            n++;
            }

        /* Eliminate bogus cycles of size 1, they shouldn't happen ???
           Otherwise fill in the Rings information from the cycle.
        */

        if (n == 1 && ringsys_tmp != NULL)
          {
          *ringsys_pp = Ringsys_Next_Get (ringsys_tmp);
          Ringsys_Destroy (ringsys_tmp);
          }
        else
          {
          (*num_ringsys_p)++;
          for (k = 0; k < Xtr_NumAtoms_Get (xtr_p); k++)
            {
            for (l = 0; l < MX_NEIGHBORS; l++)
              {
              if (Ringsys_Component_Get (ringsys_tmp, k, l) == TRUE)
                {
                Ringsys_Isin_Put (ringsys_tmp, k, TRUE);
                Ring_RingBit_Put (ring_p, k, TRUE);
                }
              }
            }

          DEBUG_DO (DB_RINGSTATIC, TL_MAJOR,
            {
            char outbuf[MX_OUTPUT_SIZE];
            char *cur;

            if (ringsys_tmp != NULL)
              {
              IO_Put_Debug (R_XTR,
                "SBiconnect, bond array of ring system follows");

              for (k = 0, cur = outbuf; k < Xtr_NumAtoms_Get (xtr_p);
                   k++, cur = outbuf)
                {
                for (l = 0; l < MX_NEIGHBORS; l++)
                  {
                  sprintf (cur, " %2d", Ringsys_Component_Get (ringsys_tmp, k,
                    l));
                  cur += 3;
                  }

                IO_Put_Debug (R_XTR, outbuf);
                }
              }
            });
          }          /* End of else n != 1 */
        }            /* End of if-lowpt_p <= number_p */
      }              /* End of if-number_p != 0 */
    else
      if ((Array_1d16_Get (number_p, neigh_id)
          < Array_1d16_Get (number_p, atom)) && (neigh_id != prev))
        {
        /* We have found a cycle!  Since the atoms are numbered in increasing
           order going down the DFS if we encounter one (not our parent) that
           has a lower number it must have been higher up the DFS-tree.
           - So create an edge for this and push it on the stack
           - Keep track of low point which will enable us to tell which
             edges to pop off the stack (when we match the low-point we are
             back at the top again)
        */

        edge_p = SEdge_Create (atom, neigh_id);
        Stack_PushAdd (stack_p, edge_p);
        Array_1d16_Put (lowpt_p , atom, MIN (Array_1d16_Get (lowpt_p, atom),
          Array_1d16_Get (number_p, neigh_id)));
        }
    }                /* End for-neigh_index  */

  DEBUG (R_XTR, DB_RINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SBiconnect, Ring System list = %p, # Ring Systems = %u",
    *ringsys_pp, *num_ringsys_p));

  return;
}

/****************************************************************************
*
*  Function Name:                 SComponent_Add
*
*    This routine adds a component to the ring system list from the stack
*    of components that have been found to be biconnected.
*
*  Used to be:
*
*    add_component:
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
static void SComponent_Add
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  Ringsys_t    *ringsys_p,                /* Ring system to modify */
  Stack_t      *stack_p                   /* Stack of edges to use */
  )
{
  Edge_t       *edge_p;                   /* Temporary */
  U16_t         i, j;                     /* Indexes */

  DEBUG (R_XTR, DB_RINGSTATIC, TL_PARAMS, (outbuf,
    "Entering SComponent_Add, Xtr = %p, Ring System = %p", xtr_p, ringsys_p));

  edge_p = (Edge_t *) Stack_TopAdd (stack_p);
  i = Edge_First_Get (edge_p);
  j = Edge_Second_Get (edge_p);

  Ringsys_Component_Put (ringsys_p, i, Xtr_Attr_NeighborIndex_Find (xtr_p,
    i, j), TRUE);

  Ringsys_Component_Put (ringsys_p, j, Xtr_Attr_NeighborIndex_Find (xtr_p,
    j, i), TRUE);

  Stack_Pop (stack_p);

  DEBUG (R_XTR, DB_RINGSTATIC, TL_PARAMS, (outbuf,
    "Leaving SComponent_Add, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SEdge_Create, SEdge_Destroy
*
*    These functions create and destroy a simple data-structure used by the
*    Biconnected algorithm to keep track of edges.
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
static Edge_t *SEdge_Create
  (
  U16_t         from,                     /* One vertex of the edge */
  U16_t         to                        /* The other vertex */
  )
{
  Edge_t       *edge_tmp;

#ifdef _MIND_MEM_
  mind_malloc ("edge_tmp", "rings{4}", &edge_tmp, EDGESIZE);
#else
  Mem_Alloc (Edge_t *, edge_tmp, EDGESIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_XTRETCMISC, TL_MEMORY, (outbuf,
    "Allocated memory for an Edge descriptor in SEdge_Create at %p",
    edge_tmp));

  if (edge_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for Edge descriptor in SEdge_Create");

  Edge_First_Put (edge_tmp, from);
  Edge_Second_Put (edge_tmp, to);

  return edge_tmp;
}

/*
static void SEdge_Destroy
  (
  Edge_t       *edge_tmp                    
  )
{
  if (edge_tmp == NULL)
    return;

#ifdef _MIND_MEM_
  mind_free ("edge_tmp", "rings", edge_tmp);
#else
  Mem_Dealloc (edge_tmp, EDGESIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_XTRETCMISC, TL_MEMORY, (outbuf,
    "Deallocated memory for an Edge in SEdge_Destroy at %p", edge_tmp));

  return;
}
   End of SEdge_Destroy */

/* End of RINGS.C */
