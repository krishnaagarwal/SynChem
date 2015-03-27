/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RINGSYSTEMS.C
*
*    This module is the abstraction for ring systems.   Ring Systems are
*    sets of connected rings.  A molecule may have more than one ring system
*    and each system may have more than one primary cycle (Ring Definition 
*    module).  In many cases it is important to know if two atoms are part
*    of the same cycle or if they are part of the same system of cycles.
*
*  Routines:
*
*    Ringsys_Copy
*    Ringsys_Create
*    Ringsys_Destroy
*    Ringsys_Dump
*    Ringsys_Ringdef_Find
*    Ringsys_Ringdef_Insert
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
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_RINGSYSTEMS_
#include "ringsystems.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif


/****************************************************************************
*
*  Function Name:                 Ringsys_Copy
*
*    This function copies a Ring System.
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
*    Address of copy of Ring System
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Ringsys_t *Ringsys_Copy
  (
  Ringsys_t    *ringsys_p                 /* Address of instance to copy */
  )
{
  Ringsys_t    *ringsys_tmp;              /* Temporary */

  if (ringsys_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_RINGSYSCOPY, TL_PARAMS, (outbuf,
    "Entering Ringsys_Copy, Ring System = %p", ringsys_p));

#ifdef _MIND_MEM_
  mind_malloc ("ringsys_tmp", "ringsystems{1}", &ringsys_tmp, RINGSYSSIZE);
#else
  Mem_Alloc (Ringsys_t *, ringsys_tmp, RINGSYSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGSYSCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Ring System in Ringsys_Copy at %p", ringsys_tmp));

  if (ringsys_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for a Ring System in Ringsys_Copy");

#ifdef _MIND_MEM
  mind_Array_Copy ("Ringsys_ComponentHandle_Get(ringsys_tmp)", "ringsystems{1}", Ringsys_ComponentHandle_Get (ringsys_p),
    Ringsys_ComponentHandle_Get (ringsys_tmp));
#else
  Array_Copy (Ringsys_ComponentHandle_Get (ringsys_p),
    Ringsys_ComponentHandle_Get (ringsys_tmp));
#endif
  Ringsys_Next_Put (ringsys_tmp, NULL);
  Ringsys_Ringdef_Put (ringsys_tmp, Ringdef_Copy (Ringsys_Ringdef_Get (
    ringsys_p)));

  DEBUG (R_XTR, DB_RINGSYSCOPY, TL_PARAMS, (outbuf,
    "Leaving Ringsys_Copy, Ring System = %p", ringsys_tmp));

  return ringsys_tmp;
}

/****************************************************************************
*
*  Function Name:                 Ringsys_Create
*
*    This function creates a Ring System data-structure.
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
*    Address of new Ring System
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Ringsys_t *Ringsys_Create
  (
  U16_t         num_atoms                 /* # atoms in m'cule */
  )
{
  Ringsys_t    *ringsys_tmp;              /* Temporary */

  DEBUG (R_XTR, DB_RINGSYSCREATE, TL_PARAMS, (outbuf,
    "Entering Ringsys_Create, # atoms = %u", num_atoms));

#ifdef _MIND_MEM_
  mind_malloc ("ringsys_tmp", "ringsystems{2}", &ringsys_tmp, RINGSYSSIZE);
#else
  Mem_Alloc (Ringsys_t *, ringsys_tmp, RINGSYSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGSYSCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Ring System in Ringsys_Create at %p", ringsys_tmp));

  if (ringsys_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for a Ring System in Ringsys_Create");

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("Ringsys_ComponentHandle_Get(ringsys_tmp)", "ringsystems{2}",
    Ringsys_ComponentHandle_Get (ringsys_tmp), num_atoms, MX_NEIGHBORS + 1, BITSIZE);
#else
  Array_2d_Create (Ringsys_ComponentHandle_Get (ringsys_tmp), num_atoms,
    MX_NEIGHBORS + 1, BITSIZE);
#endif
  Array_Set (Ringsys_ComponentHandle_Get (ringsys_tmp), FALSE);

  Ringsys_Next_Put (ringsys_tmp, NULL);
  Ringsys_Ringdef_Put (ringsys_tmp, NULL);

  DEBUG (R_XTR, DB_RINGSYSCREATE, TL_PARAMS, (outbuf,
    "Leaving Ringsys_Create, Ring System = %p", ringsys_tmp));

  return ringsys_tmp;
}

/****************************************************************************
*
*  Function Name:                 Ringsys_Destroy
*
*    This function deallocates an Ring System data-structure.
*
*  Used to be:
*
*    zrings:
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
void Ringsys_Destroy
  (
  Ringsys_t    *ringsys_p                 /* Ring system to destroy */
  )
{
  if (ringsys_p == NULL)
    return;

  DEBUG (R_XTR, DB_RINGSYSDESTROY, TL_PARAMS, (outbuf,
    "Entering Ringsys_Destroy, Ring System = %p", ringsys_p));

#ifdef _MIND_MEM_
  mind_Array_Destroy ("Ringsys_ComponentHandle_Get(ringsys_p)", "ringsystems", Ringsys_ComponentHandle_Get (ringsys_p));
#else
  Array_Destroy (Ringsys_ComponentHandle_Get (ringsys_p));
#endif
  Ringdef_Destroy (Ringsys_Ringdef_Get (ringsys_p));

  DEBUG_DO (DB_RINGSYSDESTROY, TL_MEMORY,
    {
    (void) memset (ringsys_p, 0, RINGSYSSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("ringsys_p", "ringsystems", ringsys_p);
#else
  Mem_Dealloc (ringsys_p, RINGSYSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_RINGSYSDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for a Ring System in Ringsys_Destroy at %p",
    ringsys_p));

  DEBUG (R_XTR, DB_RINGSYSDESTROY, TL_PARAMS, (outbuf,
    "Leaving Ringsys_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringsys_Dump
*
*    This routine prints a formatted dump of a Ring System descriptor.
*
*  Used to be:
*
*    ???:
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
void Ringsys_Dump
  (
  Ringsys_t     *ringsys_p,                  /* Instance to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (ringsys_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Ring System\n\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_RINGSYSCREATE, ringsys_p);
  fprintf (f, "Dump of Ring System : components, and in ring system flags\n");
  Array_Dump (Ringsys_ComponentHandle_Get (ringsys_p), filed_p);

  Ringdef_Dump (Ringsys_Ringdef_Get (ringsys_p), filed_p);

  return;
}

/****************************************************************************
*
*  Function Name:                 Ringsys_Ringdef_Find
*
*    This function finds the requested Ring Definition in the list of
*    Ring Systems.  The function does NOT check to see if it will fall off
*    the end of the list, it is the responsibility of the caller to ensure
*    that.
*
*  Used to be:
*
*    ardefn:
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
*    Address of Xth ring system's ring definition.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Ringdef_t *Ringsys_Ringdef_Find
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  U16_t         num                       /* Which ring to find */
  )
{
  Rings_t      *ring_p;                   /* Temporary */
  Ringsys_t    *ringsys_p;                /* Temporary */
  U16_t         i;                        /* Counter */

  ring_p = Xtr_Rings_Get (xtr_p);
  if (ring_p == NULL)
    return NULL;

  for (i = 0, ringsys_p = Ring_RingsysList_Get (ring_p); i < num; i++)
    {
    ASSERT (ringsys_p != NULL,
      {
      IO_Exit_Error (R_XTR, X_SYNERR,
        "Ran off end of Ring System list in Ringsys_Ringdef_Find");
      });

    ringsys_p = Ringsys_Next_Get (ringsys_p);
    }

  return Ringsys_Ringdef_Get (ringsys_p);
}

/****************************************************************************
*
*  Function Name:                 Ringsys_Ringdef_Insert
*
*    This routine inserts a Ring Definition in the specified Ring System.
*    The routine does NOT check to see if it will fall off the end of the
*    list, that is the caller's responsibility.
*
*  Used to be:
*
*    $rdefn:
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
void Ringsys_Ringdef_Insert
  (
  Xtr_t        *xtr_p,                    /* Molecule to check */
  U16_t         num,                      /* Which ring index to install at */
  Ringdef_t    *ringdef_p                 /* Ring definition to insert */
  )
{
  Rings_t      *ring_p;                   /* Temporary */
  Ringsys_t    *ringsys_p;                /* Temporary */
  U16_t         i;                        /* Counter */

  ring_p = Xtr_Rings_Get (xtr_p);
  if (ring_p == NULL)
    return;

  for (i = 0, ringsys_p = Ring_RingsysList_Get (ring_p); i < num; i++)
    {
    ASSERT (ringsys_p != NULL,
      {
      IO_Exit_Error (R_XTR, X_SYNERR,
        "Ran off end of Ring System list in Ringsys_Ringdef_Insert");
      });

    ringsys_p = Ringsys_Next_Get (ringsys_p);
    }

  Ringsys_Ringdef_Put (ringsys_p, ringdef_p);

  return;
}
/* End of Ringsys_Ringdef_Insert */
/* End of RINGSYSTEMS.C */
