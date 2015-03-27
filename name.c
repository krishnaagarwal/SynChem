/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     NAME.C
*
*    This module is the abstraction for the Name structure which is a
*    sub-structure of the XTR.  The Name_t type contains information 
*    relating to the unique name for the molecule (canonical Sling),
*    and equivilance classes for stereochemistry and constitutionality.
*
*  Routines:
*
*    Name_ConstitutionalEquivilance_Get
*    Name_Copy
*    Name_Create
*    Name_Destroy
*    Name_Dump
*    Name_Sling_Get
*    Name_SlingMap_Get
*    Name_StereochemicalEquivilance_Get
*    Xtr_Name_Set
*    SPartition
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
* 02-Mar-95  Cheung	In Xtr_Name_Set, Name_Create cannot be used for 
*			memory allocation.  Mem_Alloc & Array_2d_Create are
*			used instead.
*
*			In previous Name_Sling_Get, the length of 
*			returned sling did not match the size of its actual 
*			storage.  
* 11-May-95  Cheung     In Name_ConstitutionalEquivilance_Get,
*			Name_StereochemicalEquivilance_Get, 
*			Name_Sling_Get,
*			added - if name_p == NULL, call Xtr_Name_Set
*			
******************************************************************************/

#include <string.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_NAME_
#include "name.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

/* Static Routine Prototypes */

static void SPartition (Name_t *, String_t, Boolean_t);

/****************************************************************************
*
*  Function Name:                 Name_ConstitutionalEquivilance_Get
*
*    This function checks whether two atoms are constitutionally equivilant.
*
*  Used to be:
*
*    ce:
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
*    TRUE  - atoms 1 and 2 are constitutionally equivilant
*    FALSE - atoms 1 and 2 are NOT constitutionally equivilant
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Name_ConstitutionalEquivilance_Get
  (
  Xtr_t        *xtr_p,                     /* Molecule to check */
  U16_t         atom1,                     /* Atom index of one atom */
  U16_t         atom2                      /* Atom index of the other atom */
  )
{
  Name_t       *name_p;                    /* Temporary */

  name_p = Xtr_Name_Get (xtr_p);

  if (name_p == NULL)
    {
    Xtr_Name_Set (xtr_p, NULL);
    name_p = Xtr_Name_Get (xtr_p);
    }

  /*  Make sure equivalence class is valid.  */
  if (Name_Flags_AnyCE_Get (name_p) == TRUE
      && Name_CEMember_Get (name_p, atom1) == Name_CEMember_Get (name_p, atom2)
      && Name_CEMember_Get (name_p, atom1) < Name_NumAtoms_Get (name_p))
    return TRUE;

  return FALSE;
}

/****************************************************************************
*
*  Function Name:                 Name_Copy
*
*    This function makes a copy of Name_t parameter.
*
*  Used to be:
*
*    nname:
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
*    Address of copy of Name structure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Name_t *Name_Copy
  (
  Name_t       *name_p                     /* Name structure to copy */
  )
{
  Name_t       *name_tmp;                  /* Copy */

  if (name_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_NAMECOPY, TL_PARAMS, (outbuf,
    "Entering Name_Copy, Name = %p", name_p));

#ifdef _MIND_MEM_
  mind_malloc ("name_tmp", "name{1}", &name_tmp, NAMESIZE);
#else
  Mem_Alloc (Name_t *, name_tmp, NAMESIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_NAMECREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Name structure in Name_Copy at %p", name_p));

  if (name_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for a Name structure in Name_Copy");

#ifdef _MIND_MEM_
  mind_Array_Copy ("Name_EquivClasses_Get(name_tmp)", "name{1}", Name_EquivClasses_Get (name_p),
    Name_EquivClasses_Get (name_tmp));
#else
  Array_Copy (Name_EquivClasses_Get (name_p),
    Name_EquivClasses_Get (name_tmp));
#endif

  Name_Canonical_Put (name_tmp, Sling_Copy (Name_Canonical_Get (name_p)));
  Name_ConstitutionalLength_Put (name_tmp,
    Name_ConstitutionalLength_Get (name_p));
  Name_Flags_AnyCE_Put (name_tmp, Name_Flags_AnyCE_Get (name_p));
  Name_Flags_AnySE_Put (name_tmp, Name_Flags_AnySE_Get (name_p));

  DEBUG (R_XTR, DB_NAMECOPY, TL_PARAMS, (outbuf,
    "Leaving Name_Copy, Name = %p", name_tmp));

  return name_tmp;
}

/****************************************************************************
*
*  Function Name:                 Name_Create
*
*    This function creates a Name structure.  It also parses the canonical
*    Sling parameter to derive the equivilance classes for the molecule.
*
*  Used to be:
*
*    stname:
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
*    Address of new Name structure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Name_t *Name_Create
  (
  U16_t         num_atoms,                 /* # atoms in molecule */
  Sling_t       sling                      /* Canonical Sling */
  )
{
  Name_t       *name_p;                    /* Temporary */
  U16_t         i, j;                      /* Counters / indices */
  String_t      temp_str;                  /* Temporary */

  DEBUG (R_XTR, DB_NAMECREATE, TL_PARAMS, (outbuf,
    "Entering Name_Create, # atoms = %u, sling not shown", num_atoms));

#ifdef _MIND_MEM_
  mind_malloc ("name_p", "name{2}", &name_p, NAMESIZE);
#else
  Mem_Alloc (Name_t *, name_p, NAMESIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_NAMECREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Name structure in Name_Create at %p", name_p));

  if (name_p == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for a Name structure in Name_Create");

  /* Init the Name_t fields */

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("Name_EquivClasses_Get(name_p)", "name{2}", Name_EquivClasses_Get (name_p), 2, num_atoms, WORDSIZE);
#else
  Array_2d_Create (Name_EquivClasses_Get (name_p), 2, num_atoms, WORDSIZE);
#endif

  Name_Canonical_Put (name_p, sling);
  Name_ConstitutionalLength_Put (name_p, Sling_Length_Get (sling));
  Name_Flags_Put (name_p, 0);

  for (i = 0; i < Name_NumAtoms_Get (name_p); i++)
    {
    Name_CEMember_Put (name_p, i, -i - 1);
    Name_SEMember_Put (name_p, i, -i - 1);
    }

  /* Look for a constitutional equivilance set */

  i = PL1_Index ((char *)Sling_Name_Get (sling), Sling_Length_Get (sling),
    CE_SEP, (U16_t)1);
  if (i == (U16_t)INFINITY)
    {
    DEBUG (R_XTR, DB_NAMECREATE, TL_PARAMS, (outbuf,
      "Leaving Name_Create, Name = %p", name_p));

    return name_p;
    }

  /* Skip over the parity portion of the canonical Sling */

  j = PL1_Index ((char *)&Sling_Name_Get (sling)[i + 1],
    (U16_t)Sling_Length_Get (sling) - i - 1, PARITY_SEP, (U16_t)1);
  if (j == (U16_t)INFINITY)
    j = i;
  else
    j += i + 1;

  if (j == i)
    j = Sling_Length_Get (sling);
  else
    Name_ConstitutionalLength_Put (name_p, j);

  temp_str = String_Create_nn ((char *)&Sling_Name_Get (sling)[i + 1],
    (j - i - 1));

  if (String_Compare_c (temp_str, NOCEBS) != 0)
    {
    SPartition (name_p, temp_str, TRUE);
    Name_Flags_AnyCE_Put (name_p, TRUE);
    }

  String_Destroy (temp_str);
  if (j == Sling_Length_Get (sling))
    {
    DEBUG (R_XTR, DB_NAMECREATE, TL_PARAMS, (outbuf,
      "Leaving Name_Create, Name = %p", name_p));

    return name_p;
    }

  /* Look for the stereochemical equivilance set */

  i = PL1_Index ((char *)&Sling_Name_Get (sling)[j + 1],
    (U16_t)(Sling_Length_Get (sling) - (j + 1)), SE_SEP, (U16_t)1);
  if (i == (U16_t)INFINITY)
    i = j;
  else
    i += j + 1;

  if (i == j)
    {
    DEBUG (R_XTR, DB_NAMECREATE, TL_PARAMS, (outbuf,
      "Leaving Name_Create, Name = %p", name_p));

    return name_p;
    }

  temp_str = String_Create_nn ((char *)&Sling_Name_Get (sling)[i + 1],
    (Sling_Length_Get (sling) - (i + 1)));

  if (String_Compare_c (temp_str, NOSEBS) != 0)
    {
    SPartition (name_p, temp_str, FALSE);
    Name_Flags_AnySE_Put (name_p, TRUE);
    }

  String_Destroy (temp_str);

  DEBUG (R_XTR, DB_NAMECREATE, TL_PARAMS, (outbuf,
    "Leaving Name_Create, Name = %p", name_p));

  return name_p;
}

/****************************************************************************
*
*  Function Name:                 Name_Destroy
*
*    This routine destroys a Name_t structure.
*
*  Used to be:
*
*    zname:
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
void Name_Destroy
  (
  Name_t       *name_p                     /* Name to destroy */
  )
{
  if (name_p == NULL)
    return;

  DEBUG (R_XTR, DB_NAMEDESTROY, TL_PARAMS, (outbuf,
    "Entering Name_Destroy, Name = %p", name_p));

  Sling_Destroy (Name_Canonical_Get (name_p));
#ifdef _MIND_MEM_
  mind_Array_Destroy ("Name_EquivClasses_Get(name_p)", "name", Name_EquivClasses_Get (name_p));
#else
  Array_Destroy (Name_EquivClasses_Get (name_p));
#endif

  DEBUG_DO (DB_NAMEDESTROY, TL_MEMORY,
    {
    memset (name_p, 0, NAMESIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("name_p", "name", name_p);
#else
  Mem_Dealloc (name_p, NAMESIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_NAMEDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Name structure in Name_Destroy at %p", name_p));

  DEBUG (R_XTR, DB_NAMEDESTROY, TL_PARAMS, (outbuf,
    "Leaving Name_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Name_Dump
*
*    This routine prints a formatted dump of a Name_t structure.
*
*  Used to be:
*
*    pname:
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
void Name_Dump
  (
  Name_t       *name_p,                    /* Instance to dump */
  FileDsc_t    *filed_p                    /* File descriptor to dump to */
  )
{
  FILE         *f;                         /* Temporary */
  U16_t         equiv, atom;               /* Counters */
  Boolean_t     flag;                      /* Flag for formatting control */

  f = IO_FileHandle_Get (filed_p);
  if (name_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Name\n\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_NAMECREATE, name_p);
  fprintf (f, "Dump of Name structure\n");
  fprintf (f, "Canonical length = %u\n",
    Name_ConstitutionalLength_Get (name_p));
  Sling_Dump (Name_Canonical_Get (name_p), filed_p);

  if (Name_Flags_AnyCE_Get (name_p))
    {
    fprintf (f, "\nConstitutionally equivilant atoms\nClass#\tatoms\n------\t-----\n");
    for (equiv = 0; equiv < Name_NumAtoms_Get (name_p); equiv++)
      {
      for (atom = 0, flag = FALSE; atom < Name_NumAtoms_Get (name_p); atom++)
        if (Name_CEMember_Get (name_p, atom) == equiv)
          {
          if (flag == FALSE)
            fprintf (f, "  %u\t", equiv), flag = TRUE;

          fprintf (f, "%u ", atom);
          }

      if (flag == TRUE)
        fprintf (f, "\n");
      }
    }
  else
    fprintf (f, "This molecule has no constitutionally equivilant atoms\n");

  if (Name_Flags_AnySE_Get (name_p) != 0)
    {
    fprintf (f, "\nStereochemically equivilant atoms\nClass#\tatoms\n------\t-----\n");
    for (equiv = 0; equiv < Name_NumAtoms_Get (name_p); equiv++)
      {
      for (atom = 0, flag = FALSE; atom < Name_NumAtoms_Get (name_p); atom++)
        if (Name_SEMember_Get (name_p, atom) == equiv)
          {
          if (flag == FALSE)
            fprintf (f, "  %u\t", equiv), flag = TRUE;

          fprintf (f, "%u ", atom);
          }

      if (flag == TRUE)
        fprintf (f, "\n");
      }
    }
  else
    fprintf (f, "This molecule has no stereochemically equivilant atoms\n");

  fprintf (f, "\n");

  return;
}

/****************************************************************************
*
*  Function Name:                 Name_Sling_Get
*
*    This function returns a portion of the canonical Sling as requested.
*
*  Used to be:
*
*    getname:
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
*    Copy of Sling from Name structure.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Sling_t Name_Sling_Get
  (
  Xtr_t        *xtr_p,                     /* Molecule to set the Name of */
  Boolean_t     constitutional             /* How much of name to return */
  )
{
  Name_t       *name_p;                    /* Temporary */
  Sling_t       sling;                     /* Temporary */

  DEBUG (R_XTR, DB_NAMESLINGGET, TL_PARAMS, (outbuf,
    "Entering Name_Sling_Get, Xtr = %p, flag = %hu", xtr_p, constitutional));

  name_p = Xtr_Name_Get (xtr_p);
  if (name_p == NULL)
    {
    Xtr_Name_Set (xtr_p, NULL);
    name_p = Xtr_Name_Get (xtr_p);
    }

  if (constitutional == TRUE)
    {
    sling = Sling_Create (Name_ConstitutionalLength_Get (name_p));
    memcpy (Sling_Name_Get (sling), Sling_Name_Get (
      Name_Canonical_Get (name_p)), Sling_Length_Get (sling));
    }
  else
     sling = Sling_Copy (Name_Canonical_Get (name_p));

  DEBUG (R_XTR, DB_NAMESLINGGET, TL_PARAMS, (outbuf,
    "Leaving Name_Sling_Get, Sling not shown"));

  return sling;
}

/****************************************************************************
*
*  Function Name:                 Name_SlingMap_Get
*
*    This function gets the Sling portion of the Name, but after recalculating
*    the name and it also fills in a TSD-like mapping.
*
*  Used to be:
*
*    mgtname:
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
*    Address of Sling, length possibly altered
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Sling_t Name_SlingMap_Get
  (
  Xtr_t        *xtr_p,                     /* Molecule to set the Name of */
  Boolean_t     constitutional,            /* How much of name to return */
  Array_t      *map_p                      /* Some sort of TSD-like map */
  )
{
  Name_t       *name_p;                    /* Temporary */
  Sling_t       sling;                     /* Temporary */

  DEBUG (R_XTR, DB_NAMESLINGMAP, TL_PARAMS, (outbuf,
    "Entering Name_SlingMap_Get, Xtr = %p, flag = %hu, map array = %p", xtr_p,
    constitutional, map_p));

  name_p = Xtr_Name_Get (xtr_p);
  Name_Destroy (name_p);
  Xtr_Name_Put (xtr_p, NULL);
  Xtr_Name_Set (xtr_p, map_p);
  name_p = Xtr_Name_Get (xtr_p);


  sling = Sling_Copy (Name_Canonical_Get (name_p));

  if (constitutional == TRUE)
    {
    Sling_Length_Put (sling, Name_ConstitutionalLength_Get (name_p));
    Sling_Name_Get (sling)[Sling_Length_Get (sling)] = '\0';
    }

  DEBUG (R_XTR, DB_NAMESLINGMAP, TL_PARAMS, (outbuf,
    "Leaving Name_SlingMap_Get, Sling not shown"));

  return sling;
}

/****************************************************************************
*
*  Function Name:                 Name_StereochemicalEquivilance_Get
*
*    This function checks to see whether two atoms are stereochemically
*    equivilant.
*
*  Used to be:
*
*    se:
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
*    TRUE  - atoms 1 and 2 are stereochemically equivilant
*    FALSE - atoms 1 and 2 are NOT stereochemically equivilant
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Name_StereochemicalEquivilance_Get
  (
  Xtr_t        *xtr_p,                     /* Molecule to check */
  U16_t         atom1,                     /* Atom index to match */
  U16_t         atom2                      /* Other atom index to match */
  )
{
  Name_t       *name_p;                    /* Temporary */

  name_p = Xtr_Name_Get (xtr_p);

  if (name_p == NULL)
    {
    Xtr_Name_Set (xtr_p, NULL);
    name_p = Xtr_Name_Get (xtr_p);
    }

  /*  Make sure equivalence class is valid.  */
  if (Name_Flags_AnySE_Get (name_p) == TRUE
      && Name_SEMember_Get (name_p, atom1) == Name_SEMember_Get (name_p, atom2)
      && Name_SEMember_Get (name_p, atom1) < Name_NumAtoms_Get (name_p))
    return TRUE;

  return FALSE;

}

/****************************************************************************
*
*  Function Name:                 Xtr_Name_Set
*
*    This routine guides the setting of the canonical name and associated
*    information in the XTR.  It generates the canonical Sling and then
*    concatenates the CE Brothers string, Parity bit string and SE Brothers
*    string onto it.  The consitutional Name is simply the canonical Sling
*    plus the CE Brothers.
*
*    NOTE: the compaction flag should be TRUE, but until it is verified to
*    work ...
*
*  Used to be:
*
*    build_name:
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
void Xtr_Name_Set
  (
  Xtr_t        *xtr_p,                     /* Molecule to set Name info of */
  Array_t      *map_p                      /* 1-d word (not alloc) mapping
    array for some callers */
  )
{
  Name_t       *name_p;                    /* Temporary */
  Tsd_t        *tsd_p;                     /* TSD format of input XTR */
  Tsd_t        *canonical_tsd_p;           /* Canonical TSD */
  AtomArray_t  *parityvector_p;            /* Linked list of arrays */
  AtomArray_t  *atmarr_p;                  /* For list destruction */
  U16_t         num_atoms;                 /* # atoms in molecule */
  U16_t         num_carbons;               /* # carbon atoms in m'cule */
  U16_t         compact_size;              /* # atoms in compacted molecule */
  U16_t         length;                    /* For string manipulation */
  U16_t         i;                         /* Counter */
  Boolean_t     chiral;                    /* Chiral molecule flag */
  Boolean_t     compaction;                /* Flag to indicate compaction */
  String_t      canonical_string;          /* Canonical Sling as a String */
  String_t      ce_str;                    /* Const. Equiv. string */
  String_t      se_str;                    /* Stereochem. Equiv. string */
  String_t      temp_str;                  /* For string bashing */
  Array_t       paritybits;                /* 1-d bit (num_carbons) of
    parity */
  Array_t       mapb;                      /* 1-d word, TSD map */
  Array_t       cebs;                      /* 1-d word (compact_size) */
  Array_t       sebs;                      /* 1-d word (compact size) */
  Array_t       asymmetric;                /* 1-d bit (num_carbons) */

#ifdef _DEBUG_
printf("entering Xtr_Name_Set\n");
#endif
  DEBUG (R_XTR, DB_NAMESET, TL_PARAMS, (outbuf,
    "Entering Xtr_Name_Set, Xtr = %p, map array = %p", xtr_p, map_p));

  num_atoms = Xtr_NumAtoms_Get (xtr_p);

/*
  Sling_t       sling;
  FILL (sling, 0);
  name_p = Name_Create (num_atoms, sling);
*/
  /* the above code is replaced by the following */

#ifdef _MIND_MEM_
  mind_malloc ("name_p", "name{3}", &name_p, NAMESIZE);
  if (name_p == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for a Name structure in Xtr_Name_Set");

  mind_Array_2d_Create ("Name_EquivClasses_Get(name_p)", "name{3}", Name_EquivClasses_Get (name_p), 2, num_atoms, WORDSIZE);
#else
  Mem_Alloc (Name_t *, name_p, NAMESIZE, GLOBAL);
  if (name_p == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for a Name structure in Xtr_Name_Set");

  Array_2d_Create (Name_EquivClasses_Get (name_p), 2, num_atoms, WORDSIZE);
#endif

  tsd_p = Xtr2Tsd (xtr_p);
  compaction = TRUE;

  /* Note that some of the bit strings get put into the canonical name, so
     it looks like they should be treated as strings of '0's and '1's, ie
     characters of bytes rather than real bits.  But on second thought, do 
     the semi-painful conversion inline.
  */

#ifdef _DEBUG_
printf("about to generate cansling\n");
#endif
#ifdef _MIND_MEM_
in_subgenr(-81110101);
#endif
  Sling_Canonical_Generate (tsd_p, &canonical_tsd_p, &parityvector_p, &cebs,
    &sebs, &canonical_string, &paritybits, &ce_str, &se_str, &asymmetric,
    &mapb, compaction, &chiral, &num_carbons, &compact_size);
#ifdef _MIND_MEM_
in_subgenr(-81110102);
#endif
#ifdef _DEBUG_
printf("cansling done\n");
#endif

  length = String_Length_Get (ce_str);
  if (strncmp ((char *)&String_Value_Get (ce_str)[length - 2], 
	EQUIVCLASS_SEP, 2) == 0)
    String_Truncate (ce_str, 2);

  length = String_Length_Get (se_str);
  if (strncmp ((char *)&String_Value_Get (se_str)[length - 2], 
	EQUIVCLASS_SEP, 2) == 0)
    String_Truncate (se_str, 2);

#ifdef _MIND_MEM_
in_subgenr(-81110103);
#endif
  String_Concat_c (&canonical_string, CE_SEP);
  String_Concat (&canonical_string, ce_str);
  Name_ConstitutionalLength_Put (name_p, String_Length_Get (canonical_string));
#ifdef _MIND_MEM_
in_subgenr(-81110104);
#endif

  temp_str = String_Create (NULL, (U16_t) Array_Columns_Get (&paritybits) + 1);
  for (i = 0; i < Array_Columns_Get (&paritybits); i++)
    String_Value_Get (temp_str)[i] = Array_1d1_Get (&paritybits, i) + '0';

#ifdef _MIND_MEM_
in_subgenr(-81110105);
#endif
  String_Value_Get (temp_str)[i] = '\0';
  String_Length_Put (temp_str, i);

  String_Concat_c (&canonical_string, PARITY_SEP);
  String_Concat (&canonical_string, temp_str);
  String_Concat_c (&canonical_string, SE_SEP);
  String_Concat (&canonical_string, se_str);
  String_Destroy (temp_str);

#ifdef _MIND_MEM_
in_subgenr(-81110106);
#endif
  Name_Canonical_Put (name_p, String2Sling (canonical_string));

  if (String_Compare_c (ce_str, NOCEBS) != 0)
    {
    for (i = 0; i < num_atoms; i++)
      if ((Array_1d16_Get (&mapb, i) < compact_size) && (Array_1d16_Get (&mapb,
          i) != TSD_INVALID))
        {
        Name_CEMember_Put (name_p, i, Array_1d16_Get (&cebs, Array_1d16_Get (
          &mapb, i)));
        }
    Name_Flags_AnyCE_Put (name_p, TRUE);
    }
  else
    Name_Flags_AnyCE_Put (name_p, FALSE);

  if (String_Compare_c (se_str, NOSEBS) != 0)
    {
    for (i = 0; i < num_atoms; i++)
      if ((Array_1d16_Get (&mapb, i) < compact_size) && (Array_1d16_Get (&mapb,
          i) != TSD_INVALID))
        {
        Name_SEMember_Put (name_p, i, Array_1d16_Get (&sebs, Array_1d16_Get (
          &mapb, i)));
        }
    Name_Flags_AnySE_Put (name_p, TRUE);
    }
  else
    Name_Flags_AnySE_Put (name_p, FALSE);

  /* Free all the allocated heap storage */

  for (atmarr_p = parityvector_p; parityvector_p != NULL; atmarr_p =
       parityvector_p)
    {
    parityvector_p = AtmArr_Next_Get (atmarr_p);
    AtmArr_Destroy (atmarr_p);
    }

  Tsd_Destroy (tsd_p);
  Tsd_Destroy (canonical_tsd_p);

  String_Destroy (ce_str);
  String_Destroy (se_str);
  String_Destroy (canonical_string);

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&paritybits", "name", &paritybits);
  mind_Array_Destroy ("&cebs", "name", &cebs);
  mind_Array_Destroy ("&sebs", "name", &sebs);
  mind_Array_Destroy ("&asymmetric", "name", &asymmetric);
#else
  Array_Destroy (&paritybits);
  Array_Destroy (&cebs);
  Array_Destroy (&sebs);
  Array_Destroy (&asymmetric);
#endif

  if (map_p != NULL)
    *map_p = mapb;
  else
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&mapb", "name", &mapb);
#else
    Array_Destroy (&mapb);
#endif

  Xtr_Name_Put (xtr_p, name_p);

#ifdef _DEBUG_
printf("leaving Xtr_Name_Set\n");
#endif
  DEBUG (R_XTR, DB_NAMESET, TL_PARAMS, (outbuf,
    "Leaving Xtr_Name_Set, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SPartition
*
*    This routine does some sort of partitioning of a Sling to determine
*    some characteristics about constitutional and stereochemical equivilance.
*
*  Used to be:
*
*    partition:
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
static void SPartition
  (
  Name_t       *name_p,                    /* Name structure to update */
  String_t      class_str,                 /* Class string to convert */
  Boolean_t     const_equiv                /* Flag for which partition */
  )
{
  U8_t         *oldalloc_p;                /* Temporary */
  U16_t         i;                         /* Counters / indices */
  U16_t         equiv;                     /* Class index for equivilance */
  U16_t         number;                    /* For string to numeric conver. */
  U16_t         comma;                     /* String index */
  String_t      equiv_class;               /* Temporary */

  equiv = 0;
  equiv_class = String_Copy (class_str);
  oldalloc_p = String_Value_Get (equiv_class);
  comma = String_Index_c (equiv_class, CLASSMEMBER_SEP);

  while (comma != (U16_t)INFINITY)
    {
    for (i = 0, number = 0; i < comma; i++)
      number = number * 10 + String_Value_Get (equiv_class)[i] - '0';

    if (const_equiv == TRUE)
      {
      Name_CEMember_Put (name_p, number, equiv);
      }
    else
      {
      Name_SEMember_Put (name_p, number, equiv);
      }

    String_Discard (equiv_class, comma + 1);

    /* If we stripped to the first comma of the Class separator, then it looks
       just like a Member separator is left.  Yes, this violates any useful
       stuff from literals ...
    */

    if (memcmp (String_Value_Get (equiv_class), CLASSMEMBER_SEP, 
	sizeof (CLASSMEMBER_SEP) - 1) == 0)
      {
      ++equiv;
      String_Discard (equiv_class, sizeof (CLASSMEMBER_SEP) - 1);
      }

    comma = String_Index_c (equiv_class, CLASSMEMBER_SEP);
    }

  if (String_Length_Get (equiv_class) != 0)
    {
    for (i = 0, number = 0; i < String_Length_Get (equiv_class); i++)
      number = number * 10 + String_Value_Get (equiv_class)[i] - '0';

    if (const_equiv == TRUE)
      {
      Name_CEMember_Put (name_p, number, equiv);
      }
    else
      {
      Name_SEMember_Put (name_p, number, equiv);
      }
    }

  String_Value_Put (equiv_class, oldalloc_p);
  String_Destroy (equiv_class);

  return;
}
/* End of SPartition */
/* End of NAME.C */
