#ifndef _H_UTL_
#define _H_UTL_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     UTL.H
*
*    This module contains the abstraction for the AtomArray_t, Stack_t,
*    List_t, LinkedData_t and String_t data-structures.  These are the
*    generically useful data-structures besides arrays (which have their own
*    module).
*
*    Routines can be found in UTL.C
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Daren Krebsbach
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 10-Oct-96  Krebsbach  Added union for stack and list values.
* 23-Sep-96  Krebsbach  Moved Timer_t to timer.h.
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#define ATMARR_NOBONDS_WORD  5
#define ATMARR_NOBONDS_BYTE  6
#define ATMARR_BONDS_BYTE    7
#define ATMARR_BONDS_BIT     8

/* Stack/List type values */

#define STACK_ATMARR     5
#define STACK_NORMAL     6
#define STACK_SCALAR     7

#define LIST_ATMARR      (U8_t)(5 | ListM_IsList)
#define LIST_NORMAL      (U8_t)(6 | ListM_IsList)
#define LIST_SCALAR      (U8_t)(7 | ListM_IsList)

#define LIST_SCALAR_S32   (U8_t)(8)
#define LIST_SCALAR_S16   (U8_t)(9)
#define LIST_SCALAR_S8    (U8_t)(10)
#define LIST_SCALAR_U32   (U8_t)(11)
#define LIST_SCALAR_U16   (U8_t)(12)
#define LIST_SCALAR_U8    (U8_t)(13)

/*** Data Structures ***/

/* Linked array type.  This is used to supplement the cycle and vector types
   from the ring and cycle routines for the XTR.
*/

typedef struct s_lnarr
  {
  struct s_lnarr *next;                    /* For linked list of arrays */
  union u_multipurpose
    {
    Boolean_t   carbocyclic;               /* For Ring Definition struct */
    } x;                                   /* eXtension to make more useful */
  Array_t       arrayb;                    /* Atom (1-d word), or bond
                                              (2-d byte) array */
  } AtomArray_t;
#define ATOMARRAYSIZE sizeof (AtomArray_t)

/** Field Access Macros for AtomArray_t **/

/* Macro Prototypes
   Array_t     *AtmArr_Array_Get    (AtomArray_t *);
   AtomArray_t *AtmArr_Next_Get     (AtomArray_t *);
   void         AtmArr_Next_Put     (AtomArray_t *, AtomArray_t *);
   U16_t        AtmArr_NumAtoms_Get (AtomArray_t *);
*/

#ifndef UTL_DEBUG
#define AtmArr_Array_Get(atmarr_p)\
  &((AtomArray_t *)(atmarr_p))->arrayb

#define AtmArr_Next_Get(atmarr_p)\
  ((AtomArray_t *)(atmarr_p))->next

#define AtmArr_Next_Put(atmarr_p, value)\
  ((AtomArray_t *)(atmarr_p))->next = (AtomArray_t *)(value)

#define AtmArr_NumAtoms_Get(atmarr_p)\
  (Array_Rows_Get (AtmArr_Array_Get (atmarr_p)) != 0 ? Array_Rows_Get (\
  AtmArr_Array_Get (atmarr_p)) : Array_Columns_Get (AtmArr_Array_Get (\
  atmarr_p)))
#else
#define AtmArr_Array_Get(atmarr_p)\
  ((atmarr_p) < GBAddr ? HALT : &((AtomArray_t *)(atmarr_p))->arrayb)

#define AtmArr_Next_Get(atmarr_p)\
  ((atmarr_p) < GBAddr ? HALT : ((AtomArray_t *)(atmarr_p))->next)

#define AtmArr_Next_Put(atmarr_p, value)\
  { if ((atmarr_p) < GBAddr) HALT; else ((AtomArray_t *)(atmarr_p))->next =\
  (value); }

#define AtmArr_NumAtoms_Get(atmarr_p)\
  ((atmarr_p) < GBAddr ? HALT : Array_Rows_Get (AtmArr_Array_Get (atmarr_p))\
  != 0 ? Array_Rows_Get (AtmArr_Array_Get (atmarr_p)) :\
  Array_Columns_Get (AtmArr_Array_Get (atmarr_p)))
#endif

/** End of AtomArray_t Field Access Macros **/

/* Stack element for generalized stack */

typedef union values_u
    {
    Address       add_val;
    U32_t         u32_val;
    S32_t         s32_val;
    U16_t         u16_val;
    S16_t         s16_val;
    U8_t          u8_val;
    S8_t          s8_val;
    }  StackVal_t;                         /* Union of possible data values */
#define STACKVALSIZE sizeof (StackVal_t)

typedef StackVal_t ListVal_t;
#define LISTVALSIZE STACKVALSIZE

/** Field Access Macros for StackVal_t, ListVal_t **/

/* Macro Prototypes
   Address         LstVal_Add_Get (ListVal_t);
   void            LstVal_Add_Put (ListVal_t, Address);
   S32_t           LstVal_S32_Get (ListVal_t);
   void            LstVal_S32_Put (ListVal_t, S32_t);
   S16_t           LstVal_S16_Get (ListVal_t);
   void            LstVal_S16_Put (ListVal_t, S16_t);
   S8_t            LstVal_S8_Get  (ListVal_t);
   void            LstVal_S8_Put  (ListVal_t, S8_t);
   U32_t           LstVal_U32_Get (ListVal_t);
   void            LstVal_U32_Put (ListVal_t, U32_t);
   U16_t           LstVal_U16_Get (ListVal_t);
   void            LstVal_U16_Put (ListVal_t, U16_t);
   U8_t            LstVal_U8_Get  (ListVal_t);
   void            LstVal_U8_Put  (ListVal_t, U8_t);
   Address         StkVal_Add_Get (StackVal_t);
   void            StkVal_Add_Put (StackVal_t, Address);
   S32_t           StkVal_S32_Get (StackVal_t);
   void            StkVal_S32_Put (StackVal_t, S32_t);
   S16_t           StkVal_S16_Get (StackVal_t);
   void            StkVal_S16_Put (StackVal_t, S16_t);
   S8_t            StkVal_S8_Get  (StackVal_t);
   void            StkVal_S8_Put  (StackVal_t, S8_t);
   U32_t           StkVal_U32_Get (StackVal_t);
   void            StkVal_U32_Put (StackVal_t, U32_t);
   U16_t           StkVal_U16_Get (StackVal_t);
   void            StkVal_U16_Put (StackVal_t, U16_t);
   U8_t            StkVal_U8_Get  (StackVal_t);
   void            StkVal_U8_Put  (StackVal_t, U8_t);
*/

#define LstVal_Add_Get StkVal_Add_Get
#define LstVal_Add_Put StkVal_Add_Put
#define LstVal_S32_Get StkVal_S32_Get
#define LstVal_S32_Put StkVal_S32_Put
#define LstVal_S16_Get StkVal_S16_Get
#define LstVal_S16_Put StkVal_S16_Put
#define LstVal_S8_Get  StkVal_S8_Get
#define LstVal_S8_Put  StkVal_S8_Put
#define LstVal_U32_Get StkVal_U32_Get
#define LstVal_U32_Put StkVal_U32_Put
#define LstVal_U16_Get StkVal_U16_Get
#define LstVal_U16_Put StkVal_U16_Put
#define LstVal_U8_Get  StkVal_U8_Get
#define LstVal_U8_Put  StkVal_U8_Put

#define StkVal_Add_Get(sv)\
  (sv).add_val
#define StkVal_Add_Put(sv, sval)\
  (sv).add_val = (sval)
#define StkVal_S32_Get(sv)\
  (sv).s32_val
#define StkVal_S32_Put(sv, sval)\
  (sv).s32_val = (sval)
#define StkVal_S16_Get(sv)\
  (sv).s16_val
#define StkVal_S16_Put(sv, sval)\
  { (sv).u32_val = 0;  (sv).s16_val = (sval); }
#define StkVal_S8_Get(sv)\
  (sv).s8_val
#define StkVal_S8_Put(sv, sval)\
  { (sv).u32_val = 0;  (sv).s8_val = (sval); }
#define StkVal_U32_Get(sv)\
  (sv).u32_val
#define StkVal_U32_Put(sv, sval)\
  (sv).u32_val = (sval)
#define StkVal_U16_Get(sv)\
  (sv).u16_val
#define StkVal_U16_Put(sv, sval)\
  { (sv).u32_val = 0;  (sv).u16_val = (sval); }
#define StkVal_U8_Get(sv)\
  (sv).u8_val
#define StkVal_U8_Put(sv, sval)\
  { (sv).u32_val = 0;  (sv).u8_val = (sval); }



typedef struct s_stackl
  {
  struct s_stackl *prev;                   /* Address of previous element */
  StackVal_t       value;
  } StackElement_t;
#define STACKELEMENTSIZE sizeof (StackElement_t)

typedef StackElement_t ListElement_t;
#define LISTELEMENTSIZE STACKELEMENTSIZE

/** Field Access Macros for Stack/ListElement_t **/

/* Macro Prototypes
   ListElement_t  *LstElem_Next_Get     (ListElement_t *);
   void            LstElem_Next_Put     (ListElement_t *, ListElement_t *);
   ListVal_t       LstElem_Value_Get    (ListElement_t *);
   void            LstElem_Value_Put    (ListElement_t *, ListVal_t);
   Address         LstElem_ValueAdd_Get (ListElement_t *);
   void            LstElem_ValueAdd_Put (ListElement_t *, Address);
   S32_t           LstElem_ValueS32_Get (ListElement_t *);
   void            LstElem_ValueS32_Put (ListElement_t *, S32_t);
   S16_t           LstElem_ValueS16_Get (ListElement_t *);
   void            LstElem_ValueS16_Put (ListElement_t *, S16_t);
   S8_t            LstElem_ValueS8_Get  (ListElement_t *);
   void            LstElem_ValueS8_Put  (ListElement_t *, S8_t);
   U32_t           LstElem_ValueU32_Get (ListElement_t *);
   void            LstElem_ValueU32_Put (ListElement_t *, U32_t);
   U16_t           LstElem_ValueU16_Get (ListElement_t *);
   void            LstElem_ValueU16_Put (ListElement_t *, U16_t);
   U8_t            LstElem_ValueU8_Get  (ListElement_t *);
   void            LstElem_ValueU8_Put  (ListElement_t *, U8_t);
   StackElement_t *StkElem_Prev_Get     (StackElement_t *);
   void            StkElem_Prev_Put     (StackElement_t *, StackElement_t *);
   StackVal_t      StkElem_Value_Get    (StackElement_t *);
   void            StkElem_Value_Put    (StackElement_t *, StackVal_t);
   Address         StkElem_ValueAdd_Get (StackElement_t *);
   void            StkElem_ValueAdd_Put (StackElement_t *, Address);
   S32_t           StkElem_ValueS32_Get (StackElement_t *);
   void            StkElem_ValueS32_Put (StackElement_t *, S32_t);
   S16_t           StkElem_ValueS16_Get (StackElement_t *);
   void            StkElem_ValueS16_Put (StackElement_t *, S16_t);
   S8_t            StkElem_ValueS8_Get  (StackElement_t *);
   void            StkElem_ValueS8_Put  (StackElement_t *, S8_t);
   U32_t           StkElem_ValueU32_Get (StackElement_t *);
   void            StkElem_ValueU32_Put (StackElement_t *, U32_t);
   U16_t           StkElem_ValueU16_Get (StackElement_t *);
   void            StkElem_ValueU16_Put (StackElement_t *, U16_t);
   U8_t            StkElem_ValueU8_Get  (StackElement_t *);
   void            StkElem_ValueU8_Put  (StackElement_t *, U8_t);
*/

#define LstElem_Next_Get     StkElem_Prev_Get
#define LstElem_Next_Put     StkElem_Prev_Put
#define LstElem_Value_Get    StkElem_Value_Get
#define LstElem_Value_Put    StkElem_Value_Put
#define LstElem_ValueAdd_Get StkElem_ValueAdd_Get
#define LstElem_ValueAdd_Put StkElem_ValueAdd_Put
#define LstElem_ValueS32_Get StkElem_ValueS32_Get
#define LstElem_ValueS32_Put StkElem_ValueS32_Put
#define LstElem_ValueS16_Get StkElem_ValueS16_Get
#define LstElem_ValueS16_Put StkElem_ValueS16_Put
#define LstElem_ValueS8_Get  StkElem_ValueS8_Get
#define LstElem_ValueS8_Put  StkElem_ValueS8_Put
#define LstElem_ValueU32_Get StkElem_ValueU32_Get
#define LstElem_ValueU32_Put StkElem_ValueU32_Put
#define LstElem_ValueU16_Get StkElem_ValueU16_Get
#define LstElem_ValueU16_Put StkElem_ValueU16_Put
#define LstElem_ValueU8_Get  StkElem_ValueU8_Get
#define LstElem_ValueU8_Put  StkElem_ValueU8_Put

#define StkElem_Prev_Get(se_p)\
  (se_p)->prev

#define StkElem_Prev_Put(se_p, value)\
  (se_p)->prev = (value)

#define StkElem_Value_Get(se_p)\
  (se_p)->value

#define StkElem_Value_Put(se_p, valu)\
  (se_p)->value = (valu)

#define StkElem_ValueAdd_Get(se_p)\
  StkVal_Add_Get ((se_p)->value)

#define StkElem_ValueAdd_Put(se_p, valu)\
  StkVal_Add_Put ((se_p)->value, (valu))

#define StkElem_ValueS32_Get(se_p)\
  StkVal_S32_Get ((se_p)->value)

#define StkElem_ValueS32_Put(se_p, valu)\
  LstVal_S32_Put ((se_p)->value, (valu))

#define StkElem_ValueS16_Get(se_p)\
  StkVal_S16_Get ((se_p)->value)

#define StkElem_ValueS16_Put(se_p, valu)\
  StkVal_S16_Put ((se_p)->value, (valu))

#define StkElem_ValueS8_Get(se_p)\
  StkVal_S8_Get ((se_p)->value)

#define StkElem_ValueS8_Put(se_p, valu)\
  StkVal_S8_Put ((se_p)->value, (valu))

#define StkElem_ValueU32_Get(se_p)\
  StkVal_U32_Get ((se_p)->value)

#define StkElem_ValueU32_Put(se_p, valu)\
  LstVal_U32_Put ((se_p)->value, (valu))

#define StkElem_ValueU16_Get(se_p)\
  LstVal_U16_Get ((se_p)->value)

#define StkElem_ValueU16_Put(se_p, valu)\
  LstVal_U16_Put ((se_p)->value, (valu))

#define StkElem_ValueU8_Get(se_p)\
  StkVal_U8_Get ((se_p)->value)

#define StkElem_ValueU8_Put(se_p, valu)\
  StkVal_U8_Put ((se_p)->value, (valu))


/** End of Field Access Macros for Stack/ListElement_t **/

/* Stack structure of stack elements */

typedef struct s_stack
  {
  StackElement_t *top;                     /* Address of top stack element */
  StackElement_t *bottom;                  /* Address of bot. stack element */
  U32_t         size;                      /* Number of entries in the stack */
  U8_t          type;                      /* What type of stack */
} Stack_t;
#define STACKSIZE sizeof (Stack_t)

typedef Stack_t List_t;                    /* Linked list looks like stack */
#define LISTSIZE STACKSIZE

/* List flag */

#define ListM_IsList       0x80
#define ListM_Type         0x7f

/** Field Access Macros for Stack/List_t **/

/* Macro Prototypes
   void     *List_Front_Get (List_t *);
   void      List_Front_Put (List_t *, void *);
   Boolean_t List_IsAtmArr  (List_t *);
   Boolean_t List_IsNormal  (List_t *);
   Boolean_t List_IsScalar  (List_t *);
   U32_t     List_Size_Get  (List_t *);
   void      List_Size_Put  (List_t *, U32_t);
   void     *List_Tail_Get  (List_t *);
   void      List_Tail_Put  (List_t *, void *);
   void     *Stack_Bottom_Get (Stack_t *);
   void      Stack_Bottom_Put (Stack_t *, void *);
   Boolean_t Stack_IsAtmArr (Stack_t *);
   Boolean_t Stack_IsNormal (Stack_t *);
   Boolean_t Stack_IsScalar (Stack_t *);
   U32_t     Stack_Size_Get (Stack_t *);
   void      Stack_Size_Put (Stack_t *, U32_t);
   void     *Stack_Top_Get  (Stack_t *);
   void      Stack_Top_Put  (Stack_t *, void *);
*/

#ifndef UTL_DEBUG
#define List_Front_Get Stack_Top_Get

#define List_Front_Put Stack_Top_Put

#define List_IsAtmArr(list_p)\
  (((list_p)->type == LIST_ATMARR) ? TRUE : FALSE)

#define List_IsNormal(list_p)\
  ((list_p)->type == LIST_NORMAL ? TRUE : FALSE)

#define List_IsScalar(list_p)\
  ((list_p)->type == LIST_SCALAR ? TRUE : FALSE)

#define List_Size_Get Stack_Size_Get

#define List_Size_Put Stack_Size_Put

#define List_Tail_Get Stack_Bottom_Get

#define List_Tail_Put Stack_Bottom_Put

#define Stack_Bottom_Get(stack_p)\
  (stack_p)->bottom

#define Stack_Bottom_Put(stack_p, value)\
  (stack_p)->bottom = (StackElement_t *)(value)

#define Stack_IsAtmArr(stack_p)\
  (((stack_p)->type & ListM_Type) == STACK_ATMARR ? TRUE : FALSE)

#define Stack_IsNormal(stack_p)\
  (((stack_p)->type & ListM_Type) == STACK_NORMAL ? TRUE : FALSE)

#define Stack_IsScalar(stack_p)\
  (((stack_p)->type & ListM_Type) == STACK_SCALAR ? TRUE : FALSE)

#define Stack_Size_Get(stack_p)\
  (stack_p)->size

#define Stack_Size_Put(stack_p, value)\
  (stack_p)->size = (value)

#define Stack_Top_Get(stack_p)\
  (stack_p)->top

#define Stack_Top_Put(stack_p, value)\
  (stack_p)->top = (StackElement_t *)(value)
#else
#define List_Front_Get Stack_Top_Get

#define List_Front_Put Stack_Top_Put

#define List_IsAtmArr(list_p)\
  ((list_p) < GBAddr ? HALT : (list_p)->type == LIST_ATMARR ? TRUE : FALSE)

#define List_IsNormal(list_p)\
  ((list_p) < GBAddr ? HALT : (list_p)->type == LIST_NORMAL ? TRUE : FALSE)

#define List_IsScalar(list_p)\
  ((list_p) < GBAddr ? HALT : (list_p)->type == LIST_SCALAR ? TRUE : FALSE)

#define List_Size_Get Stack_Size_Get

#define List_Size_Put Stack_Size_Put

#define List_Tail_Get Stack_Bottom_Get

#define List_Tail_Put Stack_Bottom_Put

#define Stack_Bottom_Get(stack_p)\
  ((stack_p) < GBAddr ? HALT : (stack_p)->bottom)

#define Stack_Bottom_Put(stack_p, value)\
  { if ((stack_p) < GBAddr) HALT; else (stack_p)->bottom = (StackElement_t *)\
  (value); }

#define Stack_IsAtmArr(stack_p)\
  ((stack_p) < GBAddr ? HALT : (stack_p)->type & ListM_Type == STACK_ATMARR ?\
  TRUE : FALSE)

#define Stack_IsNormal(stack_p)\
  ((stack_p) < GBAddr ? HALT : (stack_p)->type & ListM_Type == STACK_NORMAL ?\
  TRUE : FALSE)

#define Stack_IsScalar(stack_p)\
  ((stack_p) < GBAddr ? HALT : (stack_p)->type & ListM_Type == STACK_SCALAR ?\
  TRUE : FALSE)

#define Stack_Size_Get(stack_p)\
  ((stack_p) < GBAddr ? HALT : (stack_p)->size)

#define Stack_Size_Put(stack_p, value)\
  { if ((stack_p) < GBAddr) HALT; else (stack_p)->size = (value); }

#define Stack_Top_Get(stack_p)\
  ((stack_p) < GBAddr ? HALT : (stack_p)->top)

#define Stack_Top_Put(stack_p, value)\
  { if ((stack_p) < GBAddr) HALT; else (stack_p)->top = (StackElement_t *)\
  (value); }
#endif

/** End of Stack/List_t Field Access Macros **/

/* Linked data-structure for prime cycle finding */

typedef struct s_linkl
  {
  struct s_linkl *next;                    /* Link to next item in list */
  union u_ll1
    {
    U32_t       node;                      /* Node identifier */
    U32_t       root;                      /* Root identifier */
    U32_t       valid;                     /* Valid identifier */
    struct s_ll1
      {
      struct s_linkl *first_node;          /* Address of first node */
      struct s_linkl *last_node;           /* Address of last node */
      U32_t     length;                    /* Length of path */
      } pathway;                           /* Pathway data-structure */
    struct s_ll2
      {
      U32_t     fundamental_point;         /* One end of chord */
      U32_t     end_point;                 /* Other end of chord */
      Boolean_t back_chord;                /* Does this chord go backwards */
      } chord;                             /* Cord data-structure */
    } e;                                   /* Element in linked list */
  } LinkedData_t;
#define LINKEDDATASIZE sizeof (LinkedData_t)

#define LNKDT_CHORD    1
#define LNKDT_NODE     2
#define LNKDT_PATH     3
#define LNKDT_ROOT     4
#define LNKDT_VALID    5

/** Field Access Macros for a LinkedData_t **/

/* Macro Prototypes
   U32_t         Chord_End_Get    (LinkedData_t *);
   void          Chord_End_Put    (LinkedData_t *, U32_t);
   U32_t         Chord_Fundamental_Get (LinkedData_t *);
   void          Chord_Fundamental_Put (LinkedData_t *, U32_t);
   Boolean_t     Chord_Isback_Get (LinkedData_t *);
   void          Chord_Isback_Put (LinkedData_t *, Boolean_t);
   LinkedData_t *LnkData_Next_Get (LinkedData_t *);
   void          LnkData_Next_Put (LinkedData_t *, LinkedData_t *);
   U32_t         Node_Id_Get      (LinkedData_t *);
   void          Node_Id_Put      (LinkedData_t *, U32_t);
   LinkedData_t *Path_First_Get   (LinkedData_t *);
   void          Path_First_Put   (LinkedData_t *, LinkedData_t *);
   LinkedData_t *Path_Last_Get    (LinkedData_t *);
   void          Path_Last_Put    (LinkedData_t *, LinkedData_t *);
   U32_t         Path_Length_Get  (LinkedData_t *);
   void          Path_Length_Put  (LinkedData_t *, U32_t);
   U32_t         Root_Id_Get      (LinkedData_t *);
   void          Root_Id_Put      (LinkedData_t *, U32_t);
   U32_t         Valid_Id_Get     (LinkedData_t *);
   void          Valid_Id_Put     (LinkedData_t *, U32_t);
*/

#ifndef UTL_DEBUG
#define Chord_End_Get(chord_p)\
  (chord_p)->e.chord.end_point

#define Chord_End_Put(chord_p, value)\
  (chord_p)->e.chord.end_point = (value)

#define Chord_Fundamental_Get(chord_p)\
  (chord_p)->e.chord.fundamental_point

#define Chord_Fundamental_Put(chord_p, value)\
  (chord_p)->e.chord.fundamental_point = (value)

#define Chord_Isback_Get(chord_p)\
  (chord_p)->e.chord.back_chord

#define Chord_Isback_Put(chord_p, value)\
  (chord_p)->e.chord.back_chord = (value)

#define LnkData_Next_Get(link_p)\
  (link_p)->next

#define LnkData_Next_Put(link_p, value)\
  (link_p)->next = (value)

#define Node_Id_Get(link_p)\
  (link_p)->e.node

#define Node_Id_Put(link_p, value)\
  (link_p)->e.node = (value)

#define Path_First_Get(link_p)\
  (link_p)->e.pathway.first_node

#define Path_First_Put(link_p, value)\
  (link_p)->e.pathway.first_node = (value)

#define Path_Last_Get(link_p)\
  (link_p)->e.pathway.last_node

#define Path_Last_Put(link_p, value)\
  (link_p)->e.pathway.last_node = (value)

#define Path_Length_Get(link_p)\
  (link_p)->e.pathway.length

#define Path_Length_Put(link_p, value)\
  (link_p)->e.pathway.length = (value)

#define Root_Id_Get(link_p)\
  (link_p)->e.root

#define Root_Id_Put(link_p, value)\
  (link_p)->e.root = (value)

#define Valid_Id_Get(link_p)\
  (link_p)->e.valid

#define Valid_Id_Put(link_p, value)\
  (link_p)->e.valid = (value)
#else
#define Chord_End_Get(chord_p)\
  ((chord_p) < GBAddr ? HALT : (chord_p)->e.chord.end_point)

#define Chord_End_Put(chord_p, value)\
  { if ((chord_p) < GBAddr) HALT; else (chord_p)->e.chord.end_point =\
 (value); }

#define Chord_Fundamental_Get(chord_p)\
  ((chord_p) < GBAddr ? HALT : (chord_p)->e.chord.fundamental_point)

#define Chord_Fundamental_Put(chord_p, value)\
  { if ((chord_p) < GBAddr) HALT; else (chord_p)->e.chord.fundamental_point =\
  (value); }

#define Chord_Isback_Get(chord_p)\
  ((chord_p) < GBAddr ? HALT : (chord_p)->e.chord.back_chord)

#define Chord_Isback_Put(chord_p, value)\
  { if ((chord_p) < GBAddr) HALT; else (chord_p)->e.chord.back_chord =\
  (value); }

#define LnkData_Next_Get(link_p)\
  ((link_p) < GBAddr ? HALT : (link_p)->next)

#define LnkData_Next_Put(link_p, value)\
  { if ((link_p) < GBAddr) HALT; else (link_p)->next = (value); }

#define Node_Id_Get(link_p)\
  ((link_p) < GBAddr ? HALT : (link_p)->e.node)

#define Node_Id_Put(link_p, value)\
  { if ((link_p) < GBAddr) HALT; else (link_p)->e.node = (value); }

#define Path_First_Get(link_p)\
  ((link_p) < GBAddr ? HALT : (link_p)->e.pathway.first_node)

#define Path_First_Put(link_p, value)\
  { if ((link_p) < GBAddr) HALT; else (link_p)->e.pathway.first_node =\
  (value); }

#define Path_Last_Get(link_p)\
  ((link_p) < GBAddr ? HALT : (link_p)->e.pathway.last_node)

#define Path_Last_Put(link_p, value)\
  { if ((link_p) < GBAddr) HALT; else (link_p)->e.pathway.last_node =\
  (value); }

#define Path_Length_Get(link_p)\
  ((link_p) < GBAddr ? HALT : (link_p)->e.pathway.length)

#define Path_Length_Put(link_p, value)\
  { if ((link_p) < GBAddr) HALT; else (link_p)->e.pathway.length = (value); }

#define Root_Id_Get(link_p)\
  ((link_p) < GBAddr ? HALT : (link_p)->e.root)

#define Root_Id_Put(link_p, value)\
  { if ((link_p) < GBAddr) HALT; else (link_p)->e.root = (value); }

#define Valid_Id_Get(link_p)\
  ((link_p) < GBAddr ? HALT : (link_p)->e.valid)

#define Valid_Id_Put(link_p, value)\
  { if ((link_p) < GBAddr) HALT; else (link_p)->e.valid = (value); }
#endif

/** End of Field Access Macros for LinkedData_t **/

/* String definition, might as well start on the right foot. */

typedef struct s_string
  {
  U8_t           *value;                     /* Data in string */
  U16_t           length;                    /* Length of the string */
  U16_t           alloc;                     /* Amount of storage allocated */
  } String_t;
#define STRINGSIZE sizeof (String_t)

/** Field Access Macros for a String_t **/

/* Macro Prototypes
   U16_t String_Alloc_Get  (String_t);
   void  String_Alloc_Put  (String_t, U16_t);
   U8_t *String_Value_Get  (String_t);
   void  String_Value_Put  (String_t, U8_t *);
   U16_t String_Length_Put (String_t);
   void  String_Length_Put (String_t, U16_t);
*/

#ifndef UTL_DEBUG
#define String_Alloc_Get(str)\
  (str).alloc

#define String_Alloc_Put(str, value)\
  (str).alloc = (value)

#define String_Value_Get(str)\
  (str).value

#define String_Value_Put(str, val)\
  (str).value = (U8_t *)(val)

#define String_Length_Get(str)\
  (str).length

#define String_Length_Put(str, value)\
  (str).length = (value)
#else
#define String_Alloc_Get(str)\
  ((str).value < GBAddr ? HALT : (str).alloc)

#define String_Alloc_Put(str, value)\
  { if (&(str) < GBAddr) HALT; else (str).alloc = (value); }

#define String_Value_Get(str)\
  ((str).value < GBAddr ? HALT : (str).value)

#define String_Value_Put(str, val)\
  { if (&(str) < GBAddr) HALT; else (str).value = (U8_t *)(val); }

#define String_Length_Get(str)\
  ((str).value < GBAddr ? HALT : (str).length)

#define String_Length_Put(str, val)\
  { if (&(str) < GBAddr) HALT; else (str).length = (val); }
#endif

/** End of Field Access Macros for String_t **/

/*** Macros ***/

/* Macro prototypes
  List_t   *List_Create     (U8_t);
  void      List_Destroy    (List_t *);
  ListVal_t List_First      (List_t *);
  Address   List_FirstAdd   (List_t *);
  S32_t     List_FirstS32   (List_t *);
  S16_t     List_FirstS16   (List_t *);
  S8_t      List_FirstS8    (List_t *);
  U32_t     List_FirstU32   (List_t *);
  U16_t     List_FirstU16   (List_t *);
  U8_t      List_FirstU8    (List_t *);
  void      List_InsertAdd  (List_t *, void *, Address);
  void      List_InsertS32  (List_t *, void *, S32_t);
  void      List_InsertS16  (List_t *, void *, S16_t);
  void      List_InsertS8   (List_t *, void *, S8_t);
  void      List_InsertU32  (List_t *, void *, U32_t);
  void      List_InsertU16  (List_t *, void *, U16_t);
  void      List_InsertU8   (List_t *, void *, U8_t);
  void      Stack_PushAdd   (Stack_t *, Address);
  void      Stack_PushS32   (Stack_t *, S32_t);
  void      Stack_PushS16   (Stack_t *, S16_t);
  void      Stack_PushS8    (Stack_t *, S8_t);
  void      Stack_PushU32   (Stack_t *, U32_t);
  void      Stack_PushU16   (Stack_t *, U16_t);
  void      Stack_PushU8    (Stack_t *, U8_t);
  Address   Stack_TopAdd    (Stack_t *);
  S32_t     Stack_TopS32    (Stack_t *);
  S16_t     Stack_TopS16    (Stack_t *);
  S8_t      Stack_TopS8     (Stack_t *);
  U32_t     Stack_TopU32    (Stack_t *);
  U16_t     Stack_TopU16    (Stack_t *);
  U8_t      Stack_TopU8     (Stack_t *);
  void      String_Discard  (String_t, U16_t);
  void      String_Make     (String_t, char *);
  void      String_Truncate (String_t, U16_t);
*/

/* ??? No DEBUG version of macros (general Utl macros) */
#define List_Create       Stack_Create
#define List_Destroy      Stack_Destroy
#define List_First        Stack_Top

#define List_FirstAdd     Stack_TopAdd
#define List_FirstS32     Stack_TopS32
#define List_FirstS16     Stack_TopS16
#define List_FirstS8      Stack_TopS8
#define List_FirstU32     Stack_TopU32
#define List_FirstU16     Stack_TopU16
#define List_FirstU8      Stack_TopU8

#define List_InsertAdd(ls_p, prev_p, val)\
  { ListVal_t lval; LstVal_Add_Put (lval, val);\
    List_Insert (ls_p, prev_p, lval); }

#define List_InsertS32(ls_p, prev_p, val)\
  { ListVal_t lval; LstVal_S32_Put (lval, val);\
    List_Insert (ls_p, prev_p, lval); }

#define List_InsertS16(ls_p, prev_p, val)\
  { ListVal_t lval; LstVal_S16_Put (lval, val);\
    List_Insert (ls_p, prev_p, lval); }

#define List_InsertS8(ls_p, prev_p, val)\
  { ListVal_t lval; LstVal_S8_Put (lval, val);\
    List_Insert (ls_p, prev_p, lval); }

#define List_InsertU32(ls_p, prev_p, val)\
  { ListVal_t lval; LstVal_U32_Put (lval, val);\
    List_Insert (ls_p, prev_p, lval); }

#define List_InsertU16(ls_p, prev_p, val)\
  { ListVal_t lval; LstVal_U16_Put (lval, val);\
    List_Insert (ls_p, prev_p, lval); }

#define List_InsertU8(ls_p, prev_p, val)\
  { ListVal_t lval; LstVal_U8_Put (lval, val);\
    List_Insert (ls_p, prev_p, lval); }

#define Stack_PushAdd(sk_p, val)\
  { StackVal_t sval; StkVal_Add_Put (sval, val);\
    Stack_Push (sk_p, sval); }

#define Stack_PushS32(sk_p, val)\
  { StackVal_t sval; StkVal_S32_Put (sval, val);\
    Stack_Push (sk_p, sval); }

#define Stack_PushS16(sk_p, val)\
  { StackVal_t sval; StkVal_S16_Put (sval, val);\
    Stack_Push (sk_p, sval); }

#define Stack_PushS8(sk_p, val)\
  { StackVal_t sval; StkVal_S8_Put (sval, val);\
    Stack_Push (sk_p, sval); }

#define Stack_PushU32(sk_p, val)\
  { StackVal_t sval; StkVal_U32_Put (sval, val);\
    Stack_Push (sk_p, sval); }

#define Stack_PushU16(sk_p, val)\
  { StackVal_t sval; StkVal_U16_Put (sval, val);\
    Stack_Push (sk_p, sval); }

#define Stack_PushU8(sk_p, val)\
  { StackVal_t sval; StkVal_U8_Put (sval, val);\
    Stack_Push (sk_p, sval); }

#define Stack_TopAdd(sk_p)\
  StkVal_Add_Get (Stack_Top (sk_p))

#define Stack_TopS32(sk_p)\
  StkVal_S32_Get (Stack_Top (sk_p))

#define Stack_TopS16(sk_p)\
  StkVal_S16_Get (Stack_Top (sk_p))

#define Stack_TopS8(sk_p)\
  StkVal_S8_Get (Stack_Top (sk_p))

#define Stack_TopU32(sk_p)\
  StkVal_U32_Get (Stack_Top (sk_p))

#define Stack_TopU16(sk_p)\
  StkVal_U16_Get (Stack_Top (sk_p))

#define Stack_TopU8(sk_p)\
  StkVal_U8_Get (Stack_Top (sk_p))


#define String_Discard(str, num_lost)\
  String_Length_Put (str, String_Length_Get (str) - (num_lost));\
  String_Value_Put (str, String_Value_Get (str) + (num_lost))

#define String_Make(str, carr)\
  String_Value_Put (str, carr);\
  String_Length_Put (str, strlen (carr));\
  String_Alloc_Put (str, String_Length_Get (str) + 1)

#define String_Truncate(str, num_lost)\
  String_Length_Put (str, String_Length_Get (str) - (num_lost));\
  (str).value[String_Length_Get (str)] = '\0'

/*** Routine prototypes ***/

AtomArray_t  *AtmArr_Copy      (AtomArray_t *);
AtomArray_t  *AtmArr_Create    (U16_t, U8_t, U16_t);
void          AtmArr_Destroy   (AtomArray_t *);
void          AtmArr_Dump      (AtomArray_t *, FileDsc_t *);
void	      List_Dump        (List_t *, FileDsc_t *, const char *, U8_t);
void          List_Insert      (List_t *, void *, ListVal_t);
void          List_Remove      (List_t *, void *);
LinkedData_t *LnkData_Copy     (LinkedData_t *);
LinkedData_t *LnkData_Create   (void);
void          LnkData_Destroy  (LinkedData_t *);
void          LnkData_Dump     (LinkedData_t *, U8_t, FileDsc_t *);
void          Number2Char      (U16_t, char *);
U16_t         PL1_Index        (const char *, U16_t, const char *, U16_t);
Stack_t      *Stack_Copy       (Stack_t *);
Stack_t      *Stack_Create     (Boolean_t);
void          Stack_Destroy    (Stack_t *);
void          Stack_Destroy_Safe (Stack_t *);
void          Stack_Pop        (Stack_t *);
void          Stack_Pop_Save   (Stack_t *);
void          Stack_Push       (Stack_t *, StackVal_t);
StackVal_t    Stack_Top        (Stack_t *);
S8_t          String_Compare   (String_t, String_t);
S8_t          String_Compare_c (String_t, const char *);
void          String_Concat    (String_t *, String_t);
void          String_Concat_c  (String_t *, const char *);
String_t      String_Copy      (String_t);
String_t      String_Create    (const char *, U16_t);
String_t      String_Create_nn (const char *, U16_t);
void          String_Destroy   (String_t);
U16_t         String_Index     (String_t, String_t);
U16_t         String_Index_c   (String_t, const char *);

/*** Global Variables ***/

/* End of Utl.H */
#endif
