/******************************************************************************
*
*  Copyright (C) 1991-1996 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     UTL.C
*
*    This module is the repository for generally shared and useful data-
*    structures.
*
*    The AtomArray_t is a Array_t structure with a link pointer and a sub-type
*    specific union for additional information.  There are many places where
*    this is useful.  In particular it replaces the vector and cycle structures
*    in the Ring Definition processing.
*
*    The Stack_t and StackElement_t are small structures for implementing a
*    stack (LIFO structure) of arbitrary data-structures.
*
*    The List_t and ListElement_t are small structures (same as for stacks)
*    for implementing lists (FIFO structures, and insertable lists) of
*    arbitrary data-structures.  Both lists and stacks are optimized to
*    handle AtomArray_t's and scalars in that elements won't be created.
*
*    The LnkData_t type is for finding prime cycles in the RINGDEFINITION.C
*    module.
*
*    The String_t type is to have a generic set of allocatable strings.
*
*  Routines:
*
*    AtmArr_Copy
*    AtmArr_Create
*    AtmArr_Destroy
*    AtmArr_Dump
*    List_Dump
*    List_Insert
*    List_Remove
*    LnkData_Copy
*    LnkData_Create
*    LnkData_Destroy
*    LnkData_Dump
*    Number2Char
*    PL1_Index
*    Stack_Copy
*    Stack_Create
*    Stack_Destroy
*    Stack_Destroy_Safe
*    Stack_Pop
*    Stack_Pop_Save
*    Stack_Push
*    Stack_Top
*    String_Compare
*    String_Compare_c
*    String_Concat
*    String_Concat_c
*    String_Copy
*    String_Create
*    String_Create_nn
*    String_Destroy
*    Timers_Dump
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Daren Krebsbach
*    Tito Autrey (rewritten based on others PL/I code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 11-Sep-01  Miller     Fixed String_Destory to remove "symptom correction" (vide infra)
* 06-Aug-97  Krebsbach  Added stack copy and safe destroy functions.
* 10-Oct-96  Krebsbach  Added union for stack and list values.
* 23-Sep-96  Krebsbach  Moved Timer_t to timer.h.
* 12-Sep-95  Cheung	Added routine to dump list.
*
******************************************************************************/

#include <stdio.h>
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

void showstr (char *str)
{
  char *c;

  printf (" \"");
  for (c = str; *c; c++) if (*c >= ' ' && *c <= '~') putchar (*c);
  else printf("[0x%02x]", (unsigned char) *c);
  printf ("\"\n");
}

Boolean_t test_for_encryption (char *str, int len, Boolean_t isenc, Boolean_t before)
{
  int i, nencc;
  char c, *encchars;
  Boolean_t ok;

  ok = TRUE;
  printf ("%s %scryption: String should be %scrypted\n", before ? "Before" : "After",
    isenc ? "de" : "en", isenc == before ? "en": "de");
  for (i = nencc = 0, encchars = NULL; i < len; i++)
  {
    c = isenc == before && i == len - 1 ? str[i] ^ 0xff : str[i];
    if (c > '~' || (c < ' ' && c != '\007' && c != '\t' && c != '\n'))
    {
      encchars = (char *) realloc (encchars, nencc + 1);
      encchars[nencc++] = c;
    }
  }
  if (nencc > 0)
  {
    if (isenc != before)
    {
      printf ("Encryption found in supposedly unencrypted string:");
      for (i=0; i<nencc; i++) printf (" 0x%02x", encchars[i]);
      showstr (str);
      ok = FALSE;
    }
    free (encchars);
  }
  else if (isenc == before)
  {
    printf ("\"Encrypted\" string contained no exceptional characters:");
    showstr (str);
  }

  return (ok);
}


/****************************************************************************
*
*  Function Name:                 AtmArr_Copy
*
*    This function makes a copy of a AtomArray_t.
*
*  Used to be:
*
*    N/A
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
*    Address of copy of AtomArray_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
AtomArray_t *AtmArr_Copy
  (
  AtomArray_t   *inatmarr_p                  /* AtomArray to copy */
  )
{
  AtomArray_t   *atm_tmp;                    /* Temporary for copy */

  if (inatmarr_p == NULL)
    return NULL;

  DEBUG (R_UTL, DB_ATMARRCOPY, TL_PARAMS, (outbuf,
    "Entering AtmArr_Copy, array = %p", inatmarr_p));

#ifdef _MIND_MEM_
  mind_malloc ("atm_tmp", "utl{1}", &atm_tmp, ATOMARRAYSIZE);
#else
  Mem_Alloc (AtomArray_t *, atm_tmp, ATOMARRAYSIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_ATMARRCOPY, TL_MEMORY, (outbuf,
    "Allocated an Atom Array in AtmArr_Copy at %p", atm_tmp));

  DEBUG_DO (DB_ATMARRCREATE, TL_MEMORY,
    {
    (void) memset (atm_tmp, 0, ATOMARRAYSIZE);
    });

  if (atm_tmp == NULL)
    IO_Exit_Error (R_UTL, X_LIBCALL,
      "No memory for Atom Array in AtmArr_Copy");

#ifdef _MIND_MEM_
  mind_Array_Copy ("AtmArr_Array_Get(atm_tmp)", "utl{1}", AtmArr_Array_Get (inatmarr_p), AtmArr_Array_Get (atm_tmp));
#else
  Array_Copy (AtmArr_Array_Get (inatmarr_p), AtmArr_Array_Get (atm_tmp));
#endif
  AtmArr_Next_Put (atm_tmp, NULL);
  atm_tmp->x = inatmarr_p->x;

  DEBUG (R_UTL, DB_ATMARRCOPY, TL_PARAMS, (outbuf,
    "Leaving AtmArr_Copy, atom array = %p", atm_tmp));

  return atm_tmp;
}

/****************************************************************************
*
*  Function Name:                 AtmArr_Create
*
*    This function creates a new Atom Array, these are arrays with links
*    so they can be put together into linked lists.  It may be only a 1
*    dimensional array, in which case integers are used as it will be for
*    atom indexes, otherwise it is two dimensional and is for bond values.
*
*  Used to be:
*
*    N/A
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
*    Address of new AtomArray_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
AtomArray_t *AtmArr_Create
  (
  U16_t          num_atoms,                  /* Number of atoms in molecule */
  U8_t           flag,                       /* What sort of array is this? */
  U16_t          init                        /* Init value of array elements */
  )
{
  AtomArray_t   *atm_tmp;                    /* Temporary structure pointer */

  DEBUG (R_UTL, DB_ATMARRCREATE, TL_PARAMS, (outbuf,
    "Entering AtmArr_Create, # atoms = %d, bond flag = %d, init value = %d",
    num_atoms, flag, init));

#ifdef _MIND_MEM_
  mind_malloc ("atm_tmp", "utl{2}", &atm_tmp, ATOMARRAYSIZE);
#else
  Mem_Alloc (AtomArray_t *, atm_tmp, ATOMARRAYSIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_ATMARRCREATE, TL_MEMORY, (outbuf,
    "Allocated an Atom Array in AtmArr_Create at %p", atm_tmp));

  if (atm_tmp == NULL)
    IO_Exit_Error (R_UTL, X_LIBCALL,
      "No memory for Atom Array in AtmArr_Create");

  DEBUG_DO (DB_ATMARRCREATE, TL_MEMORY,
    {
    (void) memset (atm_tmp, 0, ATOMARRAYSIZE);
    });

#ifdef _MIND_MEM_
  if (flag == ATMARR_BONDS_BYTE)
    mind_Array_2d_Create ("AtmArr_Array_Get(atm_tmp)", "utl{2}", AtmArr_Array_Get (atm_tmp), num_atoms, MX_NEIGHBORS,
      BYTESIZE);
  else
    if (flag == ATMARR_NOBONDS_WORD)
      mind_Array_1d_Create ("AtmArr_Array_Get(atm_tmp)", "utl{2a}", AtmArr_Array_Get (atm_tmp), num_atoms, WORDSIZE);
  else
    if (flag == ATMARR_NOBONDS_BYTE)
      mind_Array_1d_Create ("AtmArr_Array_Get(atm_tmp)", "utl{2b}", AtmArr_Array_Get (atm_tmp), num_atoms, BYTESIZE);
  else
    if (flag == ATMARR_BONDS_BIT)
      mind_Array_2d_Create ("AtmArr_Array_Get(atm_tmp)", "utl{2c}", AtmArr_Array_Get (atm_tmp), num_atoms, MX_NEIGHBORS,
        BITSIZE);
  else
    IO_Exit_Error (R_UTL, X_SYNERR, "Incorrect flag value in AtmArr_Create");
#else
  if (flag == ATMARR_BONDS_BYTE)
    Array_2d_Create (AtmArr_Array_Get (atm_tmp), num_atoms, MX_NEIGHBORS,
      BYTESIZE);
  else
    if (flag == ATMARR_NOBONDS_WORD)
      Array_1d_Create (AtmArr_Array_Get (atm_tmp), num_atoms, WORDSIZE);
  else
    if (flag == ATMARR_NOBONDS_BYTE)
      Array_1d_Create (AtmArr_Array_Get (atm_tmp), num_atoms, BYTESIZE);
  else
    if (flag == ATMARR_BONDS_BIT)
      Array_2d_Create (AtmArr_Array_Get (atm_tmp), num_atoms, MX_NEIGHBORS,
        BITSIZE);
  else
    IO_Exit_Error (R_UTL, X_SYNERR, "Incorrect flag value in AtmArr_Create");
#endif

  Array_Set (AtmArr_Array_Get (atm_tmp), init);
  AtmArr_Next_Put (atm_tmp, NULL);

  DEBUG (R_UTL, DB_ATMARRCREATE, TL_PARAMS, (outbuf,
    "Leaving AtmArr_Create, atom array = %p", atm_tmp));

  return atm_tmp;
}

/****************************************************************************
*
*  Function Name:                 AtmArr_Destroy
*
*    This routine deallocates the memory associated with an Atom Array.
*    The caller must detach it from whatever data-structure it is linked to.
*
*  Used to be:
*
*    N/A
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
void AtmArr_Destroy
  ( 
  AtomArray_t   *atmarr_p                   /* Temporary structure pointer */
  )
{
  DEBUG (R_UTL, DB_ATMARRDESTROY, TL_PARAMS, (outbuf,
    "Entering AtmArr_Destroy, atom array = %p", atmarr_p));

  if (atmarr_p == NULL)
    return;

#ifdef _MIND_MEM_
  mind_Array_Destroy ("AtmArr_Array_Get(atmarr_p)", "utl", AtmArr_Array_Get (atmarr_p));

  mind_free ("atmarr_p", "utl", atmarr_p);
#else
  Array_Destroy (AtmArr_Array_Get (atmarr_p));

  Mem_Dealloc (atmarr_p, ATOMARRAYSIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_ATMARRDESTROY, TL_MEMORY, (outbuf,
    "Deallocated an Atom Array in AtmArr_Destroy at %p", atmarr_p));

  DEBUG (R_UTL, DB_ATMARRDESTROY, TL_PARAMS, (outbuf,
    "Leaving AtmArr_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 AtmArr_Dump
*
*    This routine prints a formatted dump of an Atom Array.
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
*    N/A
*
******************************************************************************/
void AtmArr_Dump
  (
  AtomArray_t  *atmarr_p,                   /* Instance to dump */
  FileDsc_t    *filed_p                     /* Which file to dump to */
  )
{
  FILE         *f;                          /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (atmarr_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Atom Array\n");
    return;
    }

  DEBUG_ADDR (R_UTL, DB_ATMARRCREATE, atmarr_p);
  fprintf (f, "Dump of Atom Array, next in list is %p\n",
    AtmArr_Next_Get (atmarr_p));
  fprintf (f, "X field : carbocyclic = %d\n", atmarr_p->x.carbocyclic);
  Array_Dump (AtmArr_Array_Get (atmarr_p), filed_p);

  return;
}

/****************************************************************************
*
*  Function Name:                 List_Insert
*
*    This routine inserts a new value into the list.  The previous node
*    parameter is used to determine the list discipline.  If it is NULL, 
*    then the node is inserted at the front of the list (LIFO stack), if
*    it is the tail of the list then that is where the node is inserted
*    (FIFO list), otherwise it may be in the middle of the list in which
*    case the new node is inserted after the node pointed to by the
*    parameter.
*
*  Used to be:
*
*    N/A
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
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
void List_Insert
  (
  List_t       *list_p,                     /* List to manipulate */
  void         *prevnode_p,                 /* Node to insert after */
  ListVal_t     listvalue                   /* Value to add */
  )
{
  ListElement_t *elem_tmp;                  /* Temporary */

  DEBUG (R_UTL, DB_LISTINSERT, TL_PARAMS, (outbuf,
    "Entering List_Insert, list = %p, prev node = %p, item = %lu",
    list_p, prevnode_p, LstVal_U32_Get (listvalue)));

  /* When prevnode_p is NULL, then the insertion point is the front of the
     list.
  */

  if (List_IsAtmArr (list_p) == TRUE)
    {
    if (prevnode_p == NULL)
      {
      AtmArr_Next_Put (LstVal_Add_Get (listvalue), List_Front_Get (list_p));
      List_Front_Put (list_p, LstVal_Add_Get (listvalue));
      }
    else
      {
      if (prevnode_p == List_Tail_Get (list_p))
        List_Tail_Put (list_p, LstVal_Add_Get (listvalue));

      AtmArr_Next_Put (LstVal_Add_Get (listvalue), 
        AtmArr_Next_Get (prevnode_p));
      AtmArr_Next_Put (prevnode_p, LstVal_Add_Get (listvalue));
      }
    }
  else
    {
#ifdef _MIND_MEM_
    mind_malloc ("elem_tmp", "utl{3}", &elem_tmp, STACKELEMENTSIZE);
#else
    Mem_Alloc (ListElement_t *, elem_tmp, STACKELEMENTSIZE, GLOBAL);
#endif

    DEBUG (R_UTL, DB_LISTINSERT, TL_MEMORY, (outbuf,
      "Allocated a List Element in List_Insert at %p", elem_tmp));

    if (elem_tmp == NULL)
      IO_Exit_Error (R_UTL, X_LIBCALL,
        "No memory for List Element in List_Insert");

    LstElem_Value_Put (elem_tmp, listvalue);

    if (prevnode_p == NULL)
      {
      LstElem_Next_Put (elem_tmp, List_Front_Get (list_p));
      List_Front_Put (list_p, elem_tmp);
      }
    else
      {
      if (prevnode_p == List_Tail_Get (list_p))
        List_Tail_Put (list_p, elem_tmp);

      LstElem_Next_Put (elem_tmp, LstElem_Next_Get (
        (ListElement_t *)prevnode_p));
      LstElem_Next_Put ((ListElement_t *)prevnode_p, elem_tmp);
      }
    }

  List_Size_Put (list_p, List_Size_Get (list_p) + 1);
  if (List_Size_Get (list_p) == 1)
    List_Tail_Put (list_p, List_Front_Get (list_p));

  DEBUG (R_UTL, DB_LISTINSERT, TL_PARAMS, (outbuf,
    "Leaving List_Insert, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 List_Remove
*
*    This routine chops a value from the middle of the list and deallocates
*    the Atom Array since that is the only type that currenly uses this
*    function.  The node that is removed is determined by the previous node
*    parameter in the same was as for List_Insert.
*
*  Used to be:
*
*    N/A
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
void List_Remove
  (
  List_t       *list_p,                   /* List to manipulate */
  void         *prevnode_p                /* Ptr to node before removed one */
  )
{
  ListElement_t *elem_tmp;                /* Temporary */
  AtomArray_t  *atmarr_p;                 /* Temporary */

  if (List_Size_Get (list_p) == 0)
    return;

  DEBUG (R_UTL, DB_LISTREMOVE, TL_PARAMS, (outbuf,
    "Entering List_Remove, list = %p, prev node = %p", list_p, prevnode_p));

  /* If prevnode_p is NULL, then we want to remove the item at the head of
     the list.
  */

  if (List_IsAtmArr (list_p) == TRUE)
    {
    if (prevnode_p == NULL)
      {
      atmarr_p = (AtomArray_t *)List_Front_Get (list_p);
      List_Front_Put (list_p, AtmArr_Next_Get (atmarr_p));
      }
    else
      {
      atmarr_p = AtmArr_Next_Get (prevnode_p);
      AtmArr_Next_Put (prevnode_p, AtmArr_Next_Get (atmarr_p));
      if ((ListElement_t *)atmarr_p == List_Tail_Get (list_p))
        List_Tail_Put (list_p, prevnode_p);
      }

    AtmArr_Destroy (atmarr_p);
    }
  else
    {
    if (prevnode_p == NULL)
      {
      elem_tmp = (ListElement_t *)List_Front_Get (list_p);
      List_Front_Put (list_p, LstElem_Next_Get (elem_tmp));
      }
    else
      {
      elem_tmp = LstElem_Next_Get ((ListElement_t *)prevnode_p);
      LstElem_Next_Put ((ListElement_t *)prevnode_p,
        LstElem_Next_Get (elem_tmp));
      if (elem_tmp == List_Tail_Get (list_p))
        List_Tail_Put (list_p, prevnode_p);
      }

#ifdef _MIND_MEM_
    if (List_IsNormal (list_p) == TRUE 
        && LstElem_ValueAdd_Get (elem_tmp) != NULL)
      {
      mind_free ("LstElem_ValueAdd_Get(elem_tmp)", "utl", LstElem_ValueAdd_Get (elem_tmp));

      DEBUG (R_UTL, DB_LISTREMOVE, TL_MEMORY, (outbuf,
        "Deallocated value on List = %p in List_Remove at %p", list_p,
        LstElem_ValueAdd_Get (elem_tmp)));
      }

    mind_free ("elem_tmp", "utl", elem_tmp);
#else
    if (List_IsNormal (list_p) == TRUE 
        && LstElem_ValueAdd_Get (elem_tmp) != NULL)
      {
      Mem_Dealloc (LstElem_ValueAdd_Get (elem_tmp), INFINITY, GLOBAL);

      DEBUG (R_UTL, DB_LISTREMOVE, TL_MEMORY, (outbuf,
        "Deallocated value on List = %p in List_Remove at %p", list_p,
        LstElem_ValueAdd_Get (elem_tmp)));
      }

    Mem_Dealloc (elem_tmp, LISTELEMENTSIZE, GLOBAL);
#endif

    DEBUG (R_UTL, DB_LISTREMOVE, TL_MEMORY, (outbuf,
      "Deallocated List Element on List = %p in List_Remove at %p",
      list_p, elem_tmp));
    }

  List_Size_Put (list_p, List_Size_Get (list_p) - 1);
  if (List_Size_Get (list_p) == 0)
    {
    List_Tail_Put (list_p, NULL);
    List_Front_Put (list_p, NULL);
    }

  DEBUG (R_UTL, DB_LISTREMOVE, TL_PARAMS, (outbuf,
    "Leaving List_Remove, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 List_Dump
*
*    This routine prints a formatted dump of a Linked list data structure.
*
*  Used to be:
*
*    N/A
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
void List_Dump
  (
  List_t	*list_p,		    /* list to be dumped */
  FileDsc_t     *filed_p,                   /* Which file to dump to */
  const char	*info,   		    /* discription */
  U8_t           scalar_type
  )
{
  ListElement_t *elem_p;
  FILE		*f;
  U16_t		count;
 
  count = 1;
  f = IO_FileHandle_Get (filed_p);
  elem_p = List_Front_Get (list_p);
  
  fprintf (f, "%s\n", info);

  while (elem_p != NULL)
    {
    if (List_IsScalar (list_p))
      {
      if (scalar_type == LIST_SCALAR_S32)
        fprintf (f, " %5ld", LstElem_ValueS32_Get (elem_p));
      else if (scalar_type == LIST_SCALAR_S16)
        fprintf (f, " %5d", LstElem_ValueS16_Get (elem_p));
      else if (scalar_type == LIST_SCALAR_S8)
        fprintf (f, " %5d", LstElem_ValueS8_Get (elem_p));
      else if (scalar_type == LIST_SCALAR_U32)
        fprintf (f, " %5lu", LstElem_ValueU32_Get (elem_p));
      else if (scalar_type == LIST_SCALAR_U16)
        fprintf (f, " %5u", LstElem_ValueU16_Get (elem_p));
      else if (scalar_type == LIST_SCALAR_U8)
        fprintf (f, " %5hu", LstElem_ValueU8_Get (elem_p));
      }

    else
      fprintf (f, "%p  ", LstElem_ValueAdd_Get (elem_p));

    elem_p = LstElem_Next_Get (elem_p); 
    if (count % 10 == 0) 
      fprintf (f, "\n");
    ++count;
    }
  fprintf (f, "\n");
}
  
/****************************************************************************
*
*  Function Name:                 LnkData_Copy
*
*    This function makes a copy of a Linked Data data structure.
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
*    Address of copy
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
LinkedData_t *LnkData_Copy
  (
  LinkedData_t *lnkdata_p                   /* Temporary */
  )
{
  LinkedData_t *lnkdata_tmp;                /* Temporary */

  DEBUG (R_UTL, DB_LNKDTCOPY, TL_PARAMS, (outbuf,
    "Entering LnkData_Copy, input = %p", lnkdata_p));

  lnkdata_tmp = LnkData_Create ();
  *lnkdata_tmp = *lnkdata_p;

  DEBUG (R_UTL, DB_LNKDTCOPY, TL_PARAMS, (outbuf,
    "Leaving LnkData_Copy, copy = %p", lnkdata_tmp));

  return lnkdata_tmp;
}

/****************************************************************************
*
*  Function Name:                 LnkData_Create
*
*    This function creates a Linked Data data structure.
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
*    Address of new Linked Data data structure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
LinkedData_t *LnkData_Create
  (
  )
{
  LinkedData_t *lnkdata_p;                  /* Temporary */

  DEBUG (R_UTL, DB_LNKDTCREATE, TL_PARAMS, (outbuf,
    "Entering LnkData_Create, input = <void>"));

#ifdef _MIND_MEM_
  mind_malloc ("lnkdata_p", "utl{4}", &lnkdata_p, LINKEDDATASIZE);
#else
  Mem_Alloc (LinkedData_t *, lnkdata_p, LINKEDDATASIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_LNKDTCREATE, TL_MEMORY, (outbuf,
    "Allocated a Linked Data structure in LnkData_Create at %p", lnkdata_p));

  if (lnkdata_p == NULL)
    IO_Exit_Error (R_UTL, X_LIBCALL,
      "No memory for a Linked Data structure in LnkData_Create");

  memset (lnkdata_p, 0, LINKEDDATASIZE);

  DEBUG (R_UTL, DB_LNKDTCREATE, TL_PARAMS, (outbuf,
    "Leaving LnkData_Create, output = %p", lnkdata_p));

  return lnkdata_p;
}

/****************************************************************************
*
*  Function Name:                 LnkData_Destroy
*
*    This routine destroys a Linked Data data structure.
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
void LnkData_Destroy
  (
  LinkedData_t *lnkdata_p                   /* Instance to destroy */
  )
{

  DEBUG (R_UTL, DB_LNKDTDESTROY, TL_PARAMS, (outbuf,
    "Entering LnkData_Destroy, link data = %p", lnkdata_p));

  if (lnkdata_p == NULL)
    return;

#ifdef _MIND_MEM_
  mind_free ("lnkdata_p", "utl{5}", lnkdata_p);
#else
  Mem_Dealloc (lnkdata_p, LINKEDDATASIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_LNKDTDESTROY, TL_MEMORY, (outbuf,
    "Deallocated a Linked Data structure in LnkData_Destroy at %p",
    lnkdata_p));

  DEBUG (R_UTL, DB_LNKDTDESTROY, TL_PARAMS, (outbuf,
    "Leaving LnkData_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 LnkData_Dump
*
*    This routine prints a formatted dump of a Linked Data data structure.
*
*  Used to be:
*
*    print_list:
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
void LnkData_Dump
  (
  LinkedData_t *link_p,                     /* Instance to dump */
  U8_t          type,                       /* Formatting type */
  FileDsc_t    *filed_p                     /* Which file to dump to */
  )
{
  FILE         *f;                          /* Temporary */
  LinkedData_t *temp;                       /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (link_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Linked Data structure\n");
    return;
    }

  DEBUG_ADDR (R_UTL, DB_LNKDTCREATE, link_p);
  fprintf (f, "Dump of Linked Data, type %d\n", type);
  for ( ; link_p != NULL; link_p = LnkData_Next_Get (link_p))
    {
    switch (type)
      {
      case LNKDT_CHORD:

        fprintf (f, "Chord: fundamental = %lu, end point = %lu,"
         " back chord flag = %d\n", Chord_Fundamental_Get (link_p), 
         Chord_End_Get (link_p), Chord_Isback_Get (link_p));
        break;

      case LNKDT_NODE:

        fprintf (f, "Node: id = %lu\n", Node_Id_Get (link_p));
        break;

      case LNKDT_PATH:

        fprintf (f, "Path: length = %lu, first = %p, last = %p\nNodes:",
          Path_Length_Get (link_p), Path_First_Get (link_p),
          Path_Last_Get (link_p));
        for (temp = Path_First_Get (link_p); temp != Path_Last_Get (link_p);
             temp = LnkData_Next_Get (temp))
          fprintf (f, " %lu", Node_Id_Get (temp));

        fprintf (f, " %lu\n", Node_Id_Get (temp));
        break;

      case LNKDT_ROOT:

        fprintf (f, "Root: id = %lu\n", Root_Id_Get (link_p));
        break;

      case LNKDT_VALID:

        fprintf (f, "Valid: id = %lu\n", Valid_Id_Get (link_p));
        break;

      default:

        fprintf (f, "Error in type of Linked Data structure, type is %d",
          type);

        return;
        break;
      }
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 Number2Char
*
*    This routine converts a binary unsigned number into a character string
*    in the C fashion.
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
*    N/A
*
******************************************************************************/
void Number2Char
  (
  U16_t         number,                     /* Number to convert */
  char         *output                      /* Where to write result */
  )
{
  U8_t          index;                      /* Where next digit goes */

  if (number > 9999)
    index = 5;
  else
    if (number > 999)
      index = 4;
  else
    if (number > 99)
      index = 3;
  else
    if (number > 9)
      index = 2;
  else
    {
    if (number == 0)
      output[0] = '0';

    index = 1;
    }

  for (output[index--] = '\0'; number > 0; index--)
    {
    output[index] = (number % 10) + '0';
    number /= 10;
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 PL1_Index
*
*    This function returns the index of the start of the first occurence of
*    a given substring, 0 if the substring is not contained within the string.
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
*    Starting index of substring
*    INFINITY - if the index string is not a substring of the input
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t PL1_Index
  (
  const char   *input,
  U16_t         ilen,
  const char   *index,
  U16_t         index_len
  )
{
  U16_t         i, j;                      /* Counters */

  for (i = 0, j = 0; i < ilen; i++)
    {
    if (input[i] == index[j])
      {
      for ( ; j < index_len && input[i + j] == index[j]; j++)
        /* Empty loop body */ ;

      if (j == index_len)
        return i;

      j = 0;
      }
    }

  return ((U16_t) INFINITY);
}

/****************************************************************************
*
*  Function Name:                 Stack_Copy
*
*    This function creates a new copy of the stack.  
*
*  Used to be:
*
*    N/A
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
*    Address of new stack copy
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Stack_t *Stack_Copy
  (
  Stack_t        *stack_p                     
  )
{
  Stack_t        *copystack_p;                    /* Temporary */
  StackElement_t *elem_p;
  StackElement_t *newelem_p;
  StackElement_t *back_p;

  if (stack_p == NULL)
    return NULL;

  copystack_p = Stack_Create (stack_p->type);

  if (copystack_p == NULL)
    IO_Exit_Error (R_UTL, X_LIBCALL,
      "No memory available for stack in Stack_Create");

  if (Stack_Size_Get (stack_p) != 0)
    {
    Stack_Size_Put (copystack_p, Stack_Size_Get (stack_p));
    elem_p = Stack_Top_Get (stack_p);
#ifdef _MIND_MEM_
    mind_malloc ("newelem_p", "utl{6}", &newelem_p, STACKELEMENTSIZE);
#else
    Mem_Alloc (StackElement_t *, newelem_p, STACKELEMENTSIZE, GLOBAL);
#endif
    Stack_Top_Put (copystack_p, newelem_p);
    *newelem_p = *elem_p;
    back_p = newelem_p;
    elem_p = StkElem_Prev_Get (elem_p);

    while (elem_p != NULL)
      {
#ifdef _MIND_MEM_
      mind_malloc ("newelem_p", "utl{6a}", &newelem_p, STACKELEMENTSIZE);
#else
      Mem_Alloc (StackElement_t *, newelem_p, STACKELEMENTSIZE, GLOBAL);
#endif
      StkElem_Prev_Put (back_p, newelem_p);
      *newelem_p = *elem_p;
      back_p = newelem_p;
      elem_p = StkElem_Prev_Get (elem_p);
      }

    StkElem_Prev_Put (newelem_p, NULL);
    Stack_Bottom_Put (copystack_p, newelem_p);
    }

  return (copystack_p);
}

/****************************************************************************
*
*  Function Name:                 Stack_Create
*
*    This function creates a stack for 'type' items.
*
*  Used to be:
*
*    N/A
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
*    Address of new Stack_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Stack_t *Stack_Create
  (
  U8_t          type                        /* Flag for type of stack */
  )
{
  Stack_t      *stack_p;                    /* Temporary */

  DEBUG (R_UTL, DB_STACKCREATE, TL_PARAMS, (outbuf,
    "Entering Stack_Create, type = %d", type));

#ifdef _MIND_MEM_
  mind_malloc ("stack_p", "utl{7}", &stack_p, STACKSIZE);
#else
  Mem_Alloc (Stack_t *, stack_p, STACKSIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_STACKCREATE, TL_MEMORY, (outbuf,
    "Allocated a Stack in Stack_Create at %p", stack_p));

  if (stack_p == NULL)
    IO_Exit_Error (R_UTL, X_LIBCALL,
      "No memory available for stack in Stack_Create");

  DEBUG_DO (DB_STACKCREATE, TL_MEMORY,
    {
    memset (stack_p, 0, STACKSIZE);
    });

  Stack_Size_Put (stack_p, 0);
  Stack_Top_Put (stack_p, NULL);
  Stack_Bottom_Put (stack_p, NULL);
  stack_p->type = type;

  DEBUG (R_UTL, DB_STACKCREATE, TL_PARAMS, (outbuf,
    "Leaving Stack_Create, stack = %p", stack_p));

  return stack_p;
}

/****************************************************************************
*
*  Function Name:                 Stack_Destroy
*
*    This routine deallocates a stack and destroys any elements still in the
*    stack.
*
*  Used to be:
*
*    N/A
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
void Stack_Destroy
  (
  Stack_t      *stack_p                     /* Stack to manipulate */
  )
{
  DEBUG (R_UTL, DB_STACKDESTROY, TL_PARAMS, (outbuf,
    "Entering Stack_Destroy, stack = %p", stack_p));

  if (stack_p == NULL)
    return;

  if (stack_p->type & ListM_IsList)
    while (List_Size_Get (stack_p) != 0)
      List_Remove (stack_p, NULL);
  else
    while (Stack_Size_Get (stack_p) != 0)
      Stack_Pop (stack_p);

#ifdef _MIND_MEM_
  mind_free ("stack_p", "utl", stack_p);
#else
  Mem_Dealloc (stack_p, STACKSIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_STACKDESTROY, TL_MEMORY, (outbuf,
    "Deallocated a Stack in Stack_Destroy at %p", stack_p));

  DEBUG (R_UTL, DB_STACKDESTROY, TL_PARAMS, (outbuf,
    "Leaving Stack_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Stack_Destroy_Safe
*
*    This routine deallocates a stack and without attempting to free the
*    values in the elements of the stack.
*
*  Used to be:
*
*    N/A
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
void Stack_Destroy_Safe
  (
  Stack_t      *stack_p                     /* Stack to manipulate */
  )
{
  StackElement_t *elem_p;
  StackElement_t *back_p;

  DEBUG (R_UTL, DB_STACKDESTROY, TL_PARAMS, (outbuf,
    "Entering Stack_Destroy_Safe, stack = %p", stack_p));

  if (stack_p == NULL)
    return;

  elem_p = Stack_Top_Get (stack_p);
#ifdef _MIND_MEM_
  while (elem_p != NULL)
    {
    back_p = elem_p;
    elem_p = StkElem_Prev_Get (elem_p);
    mind_free ("back_p", "utl", back_p);
    }

  mind_free ("stack_p", "utl", stack_p);
#else
  while (elem_p != NULL)
    {
    back_p = elem_p;
    elem_p = StkElem_Prev_Get (elem_p);
    Mem_Dealloc (back_p, STACKELEMENTSIZE, GLOBAL);
    }

  Mem_Dealloc (stack_p, STACKSIZE, GLOBAL);
#endif

  DEBUG (R_UTL, DB_STACKDESTROY, TL_MEMORY, (outbuf,
    "Deallocated a Stack in Stack_Destroy_Safe at %p", stack_p));

  DEBUG (R_UTL, DB_STACKDESTROY, TL_PARAMS, (outbuf,
    "Leaving Stack_Destroy_Safe, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Stack_Pop
*
*    This routine pops the top value off the stack and deallocates both the
*    value and the stack element descriptor.
*
*  Used to be:
*
*    N/A
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
void Stack_Pop
  (
  Stack_t      *stack_p                     /* Stack to manipulate */
  )
{
  StackElement_t *elem_tmp;                 /* Temporary */
  AtomArray_t  *atmarr_p;                   /* Temporary */

  if (Stack_Size_Get (stack_p) == 0)
    return;

  DEBUG (R_UTL, DB_STACKPOP, TL_PARAMS, (outbuf,
    "Entering Stack_Pop, stack = %p", stack_p));

  if (Stack_IsAtmArr (stack_p))
    {
    atmarr_p = (AtomArray_t *)Stack_Top_Get (stack_p);
    Stack_Top_Put (stack_p, (StackElement_t *)AtmArr_Next_Get (atmarr_p));
    AtmArr_Destroy (atmarr_p);
    }
  else
    {
    elem_tmp = Stack_Top_Get (stack_p);
#ifdef _MIND_MEM_
    if (Stack_IsNormal (stack_p))
      {
      mind_free ("StkElem_ValueAdd_Get(elem_tmp)", "utl", StkElem_ValueAdd_Get (elem_tmp));

      DEBUG (R_UTL, DB_STACKPOP, TL_MEMORY, (outbuf,
        "Deallocated value on Stack = %p in Stack_Pop at %p", stack_p,
        StkElem_ValueAdd_Get (elem_tmp)));
      }

    Stack_Top_Put (stack_p, StkElem_Prev_Get (elem_tmp));
    mind_free ("elem_tmp", "utl", elem_tmp);
#else
    if (Stack_IsNormal (stack_p))
      {
      Mem_Dealloc (StkElem_ValueAdd_Get (elem_tmp), INFINITY, GLOBAL);

      DEBUG (R_UTL, DB_STACKPOP, TL_MEMORY, (outbuf,
        "Deallocated value on Stack = %p in Stack_Pop at %p", stack_p,
        StkElem_ValueAdd_Get (elem_tmp)));
      }

    Stack_Top_Put (stack_p, StkElem_Prev_Get (elem_tmp));
    Mem_Dealloc (elem_tmp, STACKELEMENTSIZE, GLOBAL);
#endif

    DEBUG (R_UTL, DB_STACKPOP, TL_MEMORY, (outbuf,
      "Deallocated Stack Element on Stack = %p in Stack_Pop at %p",
      stack_p, elem_tmp));
    }

  Stack_Size_Put (stack_p, Stack_Size_Get (stack_p) - 1);
  if (Stack_Size_Get (stack_p) == 0)
    Stack_Bottom_Put (stack_p, NULL);

  DEBUG (R_UTL, DB_STACKPOP, TL_PARAMS, (outbuf,
    "Leaving Stack_Pop, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Stack_Pop_Save
*
*    This routine pops the top value off the stack, and deallocates only the
*    stack element descriptor.
*
*  Used to be:
*
*    N/A
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
void Stack_Pop_Save
  (
  Stack_t      *stack_p                     /* Stack to manipulate */
  )
{
  StackElement_t *elem_tmp;                 /* Temporary */
  AtomArray_t  *atmarr_p;                   /* Temporary */

  if (Stack_Size_Get (stack_p) == 0)
    return;

  DEBUG (R_UTL, DB_STACKPOP, TL_PARAMS, (outbuf,
    "Entering Stack_Pop_Save, stack = %p", stack_p));

  if (Stack_IsAtmArr (stack_p) == TRUE)
    {
    atmarr_p = (AtomArray_t *)Stack_Top_Get (stack_p);
    Stack_Top_Put (stack_p, (StackElement_t *)AtmArr_Next_Get (atmarr_p));
    }
  else
    {
    elem_tmp = Stack_Top_Get (stack_p);
    Stack_Top_Put (stack_p, StkElem_Prev_Get (elem_tmp));
#ifdef _MIND_MEM_
    mind_free ("elem_tmp", "utl", elem_tmp);
#else
    Mem_Dealloc (elem_tmp, STACKELEMENTSIZE, GLOBAL);
#endif

    DEBUG (R_UTL, DB_STACKPOP, TL_MEMORY, (outbuf,
      "Deallocated Stack Element on Stack = %p in Stack_Pop at %p",
      stack_p, elem_tmp));
    }

  Stack_Size_Put (stack_p, Stack_Size_Get (stack_p) - 1);
  if (Stack_Size_Get (stack_p) == 0)
    Stack_Bottom_Put (stack_p, NULL);

  DEBUG (R_UTL, DB_STACKPOP, TL_PARAMS, (outbuf,
    "Leaving Stack_Pop_Save, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Stack_Push
*
*    This routine pushes a new value on the stack.  It allocates a stack
*    element if necessary.
*
*  Used to be:
*
*    N/A
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
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
void Stack_Push
  (
  Stack_t      *stack_p,                    /* Stack to manipulate */
  StackVal_t    value                       /* Value to add */
  )
{
  StackElement_t *elem_tmp;                 /* Temporary */

  DEBUG (R_UTL, DB_STACKPUSH, TL_PARAMS, (outbuf,
    "Entering Stack_Push, stack = %p, item = %lu", stack_p, 
    StkVal_U32_Get (value)));

  if (Stack_IsAtmArr (stack_p))
    {
    AtmArr_Next_Put ((AtomArray_t *) StkVal_Add_Get (value), 
      Stack_Top_Get (stack_p));
    Stack_Top_Put (stack_p, StkVal_Add_Get (value));
    }
  else
    {
#ifdef _MIND_MEM_
    mind_malloc ("elem_tmp", "utl{8}", &elem_tmp, STACKELEMENTSIZE);
#else
    Mem_Alloc (StackElement_t *, elem_tmp, STACKELEMENTSIZE, GLOBAL);
#endif

    DEBUG (R_UTL, DB_STACKPUSH, TL_MEMORY, (outbuf,
      "Allocated a Stack Element in Stack_Push at %p", elem_tmp));

    if (elem_tmp == NULL)
      IO_Exit_Error (R_UTL, X_LIBCALL,
        "No memory for Stack Element in Stack_Push");

    StkElem_Value_Put (elem_tmp, value);
    StkElem_Prev_Put (elem_tmp, Stack_Top_Get (stack_p));
    Stack_Top_Put (stack_p, elem_tmp);
    }

  Stack_Size_Put (stack_p, Stack_Size_Get (stack_p) + 1);
  if (Stack_Size_Get (stack_p) == 1)
    Stack_Bottom_Put (stack_p, Stack_Top_Get (stack_p));

  DEBUG (R_UTL, DB_STACKPUSH, TL_PARAMS, (outbuf,
    "Leaving Stack_Push, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 Stack_Top
*
*    This function returns a pointer to the top value of the stack.
*
*  Used to be:
*
*    N/A
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
*    Value of top element on the stack
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
StackVal_t Stack_Top
  (
  Stack_t      *stack_p                     /* Stack to manipulate */
  )
{
  StackVal_t    temp_val;

  if ((stack_p != NULL) && (Stack_Size_Get (stack_p) > 0))
    if (Stack_IsAtmArr (stack_p) == TRUE)
      {
      StkVal_Add_Put (temp_val, Stack_Top_Get (stack_p));
      return temp_val;
      }
    else
      return StkElem_Value_Get (Stack_Top_Get (stack_p));
  else
    {
    StkVal_Add_Put (temp_val, NULL);
    return temp_val;
    }
}

/****************************************************************************
*
*  Function Name:                 String_Compare
*
*    This function returns a lexicographical difference indicator as to whether
*    the first string is less than, equal to, or greater than the second
*    string.
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
*    Difference flag
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
S8_t String_Compare
  (
  String_t      first,                     /* First string to look at */
  String_t      second                     /* Second string to look at */
  )
{
  S32_t         diff;                      /* Result */

  if (String_Length_Get (first) == 0)
    if (String_Length_Get (second) == 0)
      return 0;
    else
      return -1;
  else
    if (String_Length_Get (second) == 0)
      return 1;

  diff = memcmp (String_Value_Get (first), String_Value_Get (second),
    MIN (String_Length_Get (first), String_Length_Get (second)));

  if ((diff == 0) && (String_Length_Get (first) != String_Length_Get (second)))
    if (String_Length_Get (first) < String_Length_Get (second))
      diff = -1;
    else
      diff = 1;

  return (S8_t)diff;
}

/****************************************************************************
*
*  Function Name:                 String_Compare_c
*
*    This function returns a lexicographical difference indicator as to whether
*    the first string is less than, equal to, or greater than the second
*    string.
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
*    Difference in first byte that differs, or 0 if they are the same
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
S8_t String_Compare_c
  (
  String_t      first,                     /* First string to look at */
  const char   *comp_p                     /* String to compare against */
  )
{
  String_t      second;                    /* String format if char array */
 
  String_Make (second, comp_p);
 
  return String_Compare (first, second);
}

/****************************************************************************
*
*  Function Name:                 String_Concat
*
*    This routine concatenates a String onto another String.  It may 
*    allocate more memory for the string.
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
*    May allocate and deallocate memory
*    May call IO_Exit_Error
*
******************************************************************************/
void String_Concat
  (
  String_t     *str_p,                     /* String to concatenate onto */
  String_t      add                        /* String to add */
  )
{
  U32_t         length;                    /* # chars to copy */
  U32_t         new_len;                   /* For new memory */
  U8_t         *temp;                      /* For new memory */

  length = String_Length_Get (add);
  if (length + String_Length_Get (*str_p) + 1 > String_Alloc_Get (*str_p))
    {
/*
    new_len = (length + String_Length_Get (*str_p) + 64) / 64;
    new_len = new_len * 64 + String_Alloc_Get (*str_p);
*/
    new_len = length + String_Length_Get (*str_p) + 1;
#ifdef _MIND_MEM_
    mind_malloc ("temp", "utl{9}", &temp, new_len);

    DEBUG (R_UTL, DB_STRINGCONCAT, TL_MEMORY, (outbuf,
      "Allocated a larger String value in String_Concat at %p", temp));

    memcpy (temp, String_Value_Get (*str_p), String_Length_Get (*str_p));
    mind_free ("String_Value_Get(*str_p)", "utl", String_Value_Get (*str_p));
#else
    Mem_Alloc (U8_t *, temp, new_len, GLOBAL);

    DEBUG (R_UTL, DB_STRINGCONCAT, TL_MEMORY, (outbuf,
      "Allocated a larger String value in String_Concat at %p", temp));

    memcpy (temp, String_Value_Get (*str_p), String_Length_Get (*str_p));
    Mem_Dealloc (String_Value_Get (*str_p), String_Alloc_Get (*str_p), GLOBAL);
#endif

    DEBUG (R_UTL, DB_STRINGCONCAT, TL_MEMORY, (outbuf,
      "Deallocated an old String value in String_Concat at %p",
      String_Value_Get (*str_p)));

    String_Value_Put (*str_p, temp);
    String_Alloc_Put (*str_p, (U16_t) new_len);
    }

  memcpy (&str_p->value[String_Length_Get (*str_p)], String_Value_Get (add),
    length + 1);
  String_Length_Put (*str_p, String_Length_Get (*str_p) + (U16_t) length);

  return;
}

/****************************************************************************
*
*  Function Name:                 String_Concat_c
*
*    This function concatenates a character array (C string) onto a String.
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
*    N/A
*
******************************************************************************/
void String_Concat_c
  (
  String_t     *str_p,                     /* String to concatenate onto */
  const char   *add_p                      /* String to add */
  )
{
  String_t      temp;                      /* So can use another routine */

  String_Make (temp, add_p);
  String_Concat (str_p, temp);

  return;
}

/****************************************************************************
*
*  Function Name:                 String_Copy
*
*    This function makes a copy of a String.
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
*    Copy of String
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
String_t String_Copy
  (
  String_t      input                      /* String to copy */
  )
{
  U8_t         *tmp;                       /* For allocation */
  String_t      output;                    /* Result of copy */

  DEBUG (R_UTL, DB_STRINGCOPY, TL_PARAMS, (outbuf,
    "Entering String_Copy, string not shown"));

#ifdef _MIND_MEM_
  mind_malloc ("tmp", "utl{10}", &tmp, String_Alloc_Get (input));
#else
  Mem_Alloc (U8_t *, tmp, String_Alloc_Get (input), GLOBAL);
#endif
  String_Value_Put (output, tmp);

  DEBUG (R_UTL, DB_STRINGCOPY, TL_MEMORY, (outbuf,
    "Allocated a String value in String_Copy at %p",
    String_Value_Get (output)));

  memcpy (String_Value_Get (output), String_Value_Get (input),
    String_Alloc_Get (input));

  String_Length_Put (output, String_Length_Get (input));
  String_Alloc_Put (output, String_Alloc_Get (input));

  DEBUG (R_UTL, DB_STRINGCOPY, TL_PARAMS, (outbuf,
    "Leaving String_Copy, output not shown"));

  return output;
}

/****************************************************************************
*
*  Function Name:                 String_Create
*
*    This function creates and initializes a new String of alloc # characters.
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
*    Newly created String
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
String_t String_Create
  (
  const char   *init,                      /* Character array for init */
  U16_t         alloc                      /* # characters to allocate */
  )
{
  U8_t         *tmp;                       /* For allocation */
  U16_t         length;                    /* Length of initializer */
  String_t      output;                    /* String structure */

  DEBUG (R_UTL, DB_STRINGCREATE, TL_PARAMS, (outbuf,
    "Entering String_Create, init = %s, alloc = %d", init, alloc));

  if (init != NULL)
    length = strlen (init);
  else
    length = 0;

  alloc = MAX (alloc, length + 1);

#ifdef _MIND_MEM_
  mind_malloc ("tmp", "utl{11}", &tmp, alloc);
#else
  Mem_Alloc (U8_t *, tmp, alloc, GLOBAL);
#endif
  String_Value_Put (output, tmp);

  DEBUG (R_UTL, DB_STRINGCREATE, TL_MEMORY, (outbuf,
    "Allocated a String value in String_Create, at %p",
    String_Value_Get (output)));

  if (String_Value_Get (output) == NULL)
    IO_Exit_Error (R_UTL, X_LIBCALL,
      "No memory for String value in String_Creat");

  if (init != NULL)
    memcpy (String_Value_Get (output), init, length);

  String_Value_Get (output)[length] = '\0';
  String_Length_Put (output, length);
  String_Alloc_Put (output, alloc);

  DEBUG (R_UTL, DB_STRINGCREATE, TL_PARAMS, (outbuf,
    "Leaving String_Create, output not shown"));

  return output;
}

/****************************************************************************
*
*  Function Name:                 String_Create_nn
*
*    This function creates and initializes a new String from a non-null
*    terminated character array.
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
*    Newly created String
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
String_t String_Create_nn
  (
  const char   *init,                      /* Character array for init */
  U16_t         length                     /* # characters to copy */
  )
{
  U8_t         *tmp;                       /* For allocation */
  U16_t         alloc;                     /* Allocation size */
  String_t      output;                    /* String structure */

  DEBUG (R_UTL, DB_STRINGCREATE, TL_PARAMS, (outbuf,
    "Entering String_Create_nn, init = %s, length = %d", init, length));

  if (init == NULL)
    IO_Exit_Error (R_UTL, X_SYNERR,
      "Tried to create a String, but required initializer was NULL");

  alloc = length + 1;
#ifdef _MIND_MEM_
  mind_malloc ("tmp", "utl{12}", &tmp, alloc);
#else
  Mem_Alloc (U8_t *, tmp, alloc, GLOBAL);
#endif
  String_Value_Put (output, tmp);

  DEBUG (R_UTL, DB_STRINGCREATE, TL_MEMORY, (outbuf,
    "Allocated a String value in String_Create, at %p",
    String_Value_Get (output)));

  if (String_Value_Get (output) == NULL)
    IO_Exit_Error (R_UTL, X_LIBCALL,
      "No memory for String value in String_Create_nn");

  memcpy (String_Value_Get (output), init, length);

  String_Value_Get (output)[length] = '\0';
  String_Length_Put (output, length);
  String_Alloc_Put (output, alloc);

  DEBUG (R_UTL, DB_STRINGCREATE, TL_PARAMS, (outbuf,
    "Leaving String_Create_nn, output not shown"));

  return output;
}

/****************************************************************************
*
*  Function Name:                 String_Destroy
*
*    This routine destroys a String's allocation, but not the structure itself.
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
void String_Destroy
  (
  String_t       string                      /* String to destroy */
  )
{
  DEBUG (R_UTL, DB_STRINGDESTROY, TL_PARAMS, (outbuf,
    "Entering String_Destroy, string not shown"));

  if (String_Value_Get (string) == NULL /* || String_Length_Get (string) == 0 (This attacked a symptom, rather than
                                                                               the problem itself - the use of
                                                                               String_Make in place of String_Create
                                                                               in subgenr!  Now that that is fixed,
                                                                               this fix is obsolete. */)
    return;

#ifdef _MIND_MEM_
  mind_free ("String_Value_Get(string)", "utl", String_Value_Get (string));
#else
  Mem_Dealloc (String_Value_Get (string), String_Alloc_Get (string), GLOBAL);
#endif

  DEBUG (R_UTL, DB_STRINGDESTROY, TL_MEMORY, (outbuf,
    "Deallocated a String value in String_Destroy at %p",
    String_Value_Get (string)));

  DEBUG (R_UTL, DB_STRINGDESTROY, TL_PARAMS, (outbuf,
    "Leaving String_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 String_Index
*
*    This function uses the PL1_Index function, but accepts String data-types
*    as input.
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
*    Starting character index of substring in string
*    INFINITY if substring is not found
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t String_Index
  (
  String_t       string,                     /* String to search */
  String_t       substr                      /* Substring to search for */
  )
{
  return PL1_Index ((char *)String_Value_Get (string),
    String_Length_Get (string), (char *)String_Value_Get (substr),
    String_Length_Get (substr));
}

/****************************************************************************
*
*  Function Name:                 String_Index_c
*
*    This function uses the PL1_Index function, but accepts a String data-type
*    as input and a C-string.
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
*    Starting character index of substring in string
*    INFINITY if substring is not found
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t String_Index_c
  (
  String_t       string,                     /* String to search */
  const char    *substr_p                    /* Substring to search for */
  )
{
  return PL1_Index ((char *)String_Value_Get (string),
    String_Length_Get (string), substr_p, (U16_t)strlen (substr_p));
}

/****************************************************************************
*
*  Function Name:                 String_Encrypt
*
*    This function encrypts or decrypts a string.
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
*    Encrypted or decrypted copy of decrypted or encrypted string
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
String_t String_Encrypt
  (
  String_t       string,
  int            offset,
  int            recnum
  )
{
  String_t copy;
  int len;
  char *str;

  copy = String_Copy (string);
  len = String_Length_Get (copy);
  str = String_Value_Get (copy);
  if (str[len - 1] & 0x80) /* encrypted - must decrypt */
  {
#ifdef _DEBUG_
if (!test_for_encryption (str, len, TRUE, TRUE))
  printf ("offset = %d; recnum = %d\n", offset, recnum);
#endif
    str[len - 1] ^= 0xff;
    Mem_Encrypt (str, 25 * offset, recnum, len);
    Mem_Scramble (str, len - 1);
#ifdef _DEBUG_
if (!test_for_encryption (str, len, TRUE, FALSE))
  printf ("offset = %d; recnum = %d\n", offset, recnum);
#endif
  }
  else
  {
#ifdef _DEBUG_
if (!test_for_encryption (str, len, FALSE, TRUE))
  printf ("offset = %d; recnum = %d\n", offset, recnum);
#endif
    Mem_Scramble (str, len - 1);
    Mem_Encrypt (str, 25 * offset, recnum, len);
    str[len - 1] ^= 0xff;
#ifdef _DEBUG_
if (!test_for_encryption (str, len, FALSE, FALSE))
  printf ("offset = %d; recnum = %d\n", offset, recnum);
#endif
  }

  return (copy);
}

/****************************************************************************
*
*  Function Name:                 String_Copy_Encrypted
*
*    This function decrypts the source string and encrypts the copy for the destination.
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
*    Encrypted or decrypted copy of decrypted or encrypted string
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
String_t String_Copy_Encrypted
  (
  String_t       string,
  int            offset,
  int            source_recnum,
  int            dest_recnum
  )
{
  String_t source_copy, dest_copy;

  source_copy = String_Encrypt (string, offset, source_recnum);
  dest_copy = String_Encrypt (source_copy, offset, dest_recnum);
  String_Destroy (source_copy);

  return (dest_copy);
}

/****************************************************************************
*
*  Function Name:                 Strings_Encrypt
*
*    This function encrypts or decrypts an array of strings.
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
*    Encrypted or decrypted copy of decrypted or encrypted string
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
String_t *Strings_Encrypt (String_t *strs, int nstrs, int schn)
{
  String_t *enc_strs;
  int i;

  enc_strs = (String_t *) malloc (nstrs * sizeof (String_t));
  for (i = 0; i < nstrs; i++) enc_strs[i] = String_Encrypt (strs[i], i, schn);

  return (enc_strs);
}

/****************************************************************************
*
*  Function Name:                 Strings_Encrypt_and_Replace
*
*    This function encrypts or decrypts the individual members of a string array.
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
*    Encrypted or decrypted copy of decrypted or encrypted string
*
*  Side Effects:
*
*    N/A
*
****************************************************************************/
void Strings_Encrypt_and_Replace (String_t *strs, int nstrs, int schn)
{
  String_t enc_str;
  int i;

  for (i = 0; i < nstrs; i++)
  {
    enc_str = String_Encrypt (strs[i], i, schn);
    String_Destroy (strs[i]);
    strs[i] = enc_str;
  }
}

/* End of UTL.C */
