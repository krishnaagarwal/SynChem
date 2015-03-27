#ifndef _H_FUNCGROUP_FILE_
#define _H_FUNCGROUP_FILE_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     FUNCGROUP_FILE.H
*
*    This module is the abstraction of Functional Groups information file
*    data-structures.  These are the names and Slings of the Functional Groups
*    and are used to create the encoding strings for determining which Func.
*    Groups are in a molecule and for displaying information on them.  It
*    also contains the data-structure for Func. Group encoding.  This type
*    is designed so that the encoding is small and able to be manipulated
*    very fast.
*
*    Routines are found in FUNCGROUP_FILE.C
*
*  Creation Date:
*
*    01-Jan-1991
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
* 26-Feb-95  Krebsbach  Removed restriction that the name or sling index
*                       be less than the count stored in the header when
*                       the name or sling is ADDED with 
*                       FuncGrp_Rec_Name_Put or FuncGrp_Rec_Sling_Put in
*                       XTR_DEBUG mode. 
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

/*** Literal Values ***/

#define FG_MIN_LINEAR  8
#define FG_MX_NOTGRP   4
#define FG_MX_BUFSIZE  4096

#define FG_ATOM_IDX    0
#define FG_ATOMBOND    1

/*** Data Structures ***/

/* The encoding type is designed to sort on the atomid and bond fields with
   the help of a mask (see below).  The structure is also intended to take
   up only 32 bits which is a bit compiler/architecture dependent.
*/

#ifndef _WIN32
typedef union u_fgno
  {
  U32_t         whole;
  struct s_fgbv
    {
    unsigned atom    : 8;
    unsigned bond    : 3;
    unsigned entered : 1;
    unsigned xpanded : 3;
    unsigned valid   : 1;
    unsigned continu : 1;
    unsigned compres : 1;
    unsigned next    : 14;
    } bits;
  } FuncGrp_Encode_t;
#else
typedef union u_fgno
  {
  U32_t         whole;                     /* Whole structure */
  struct s_fgbv
    {
    unsigned next    : 14;                 /* Next table *or* FG# */
    unsigned compres : 1;                  /* Next base is compressed */
    unsigned continu : 1;                  /* Valid prefix, fg# is in base */
    unsigned valid   : 1;                  /* This is a match */
    unsigned xpanded : 3;                  /* Start of node expansion */
    unsigned entered : 1;                  /* Node is part of cycle */
    unsigned bond    : 3;                  /* Bond representation */
    unsigned atom    : 8;                  /* Atom representation */
    } bits;
  } FuncGrp_Encode_t;
#endif
#define FUNCGRPENCODESIZE sizeof (FuncGrp_Encode_t)

/* Hack alert!!! this is for masking the above structure so that only the
   atom, bond, entered and xpanded fields are considered.  The AtomBond 
   mask is for encoding, the Decode mask is (obviously) for decoding.
*/

#define FuncGrp_EncM_AtomBond 0xfffe0000
#define FuncGrp_EncM_Decode   0xfff00000

/** Field Access Macros for FuncGrp_Encode_t **/

/* Macro Prototypes
   U8_t      FuncGrp_Enc_Atomid_Get  (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Atomid_Put  (FuncGrp_Encode_t *, U8_t);
   U8_t      FuncGrp_Enc_Bond_Get    (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Bond_Put    (FuncGrp_Encode_t *, U8_t);
   Boolean_t FuncGrp_Enc_Compress_Get (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Compress_Put (FuncGrp_Encode_t *, Boolean_t);
   Boolean_t FuncGrp_Enc_Continue_Get (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Continue_Put (FuncGrp_Encode_t *, Boolean_t);
   Boolean_t FuncGrp_Enc_Entered_Get (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Entered_Put (FuncGrp_Encode_t *, Boolean_t);
   Boolean_t FuncGrp_Enc_Xpanded_Get (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Xpanded_Incr (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Xpanded_Put (FuncGrp_Encode_t *, Boolean_t);
   U16_t     FuncGrp_Enc_Next_Get    (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Next_Put    (FuncGrp_Encode_t *, U16_t);
   U32_t     FuncGrp_Enc_Size_Get    (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Size_Put    (FuncGrp_Encode_t *, U32_t);
   Boolean_t FuncGrp_Enc_Valid_Get   (FuncGrp_Encode_t *);
   void      FuncGrp_Enc_Valid_Put   (FuncGrp_Encode_t *, Boolean_t);
*/

#ifndef XTR_DEBUG
#define FuncGrp_Enc_Atomid_Get(fge_p)\
  (U8_t)(fge_p)->bits.atom

#define FuncGrp_Enc_Atomid_Put(fge_p, value)\
  (fge_p)->bits.atom = (value)

#define FuncGrp_Enc_Bond_Get(fge_p)\
  (U8_t)(fge_p)->bits.bond

#define FuncGrp_Enc_Bond_Put(fge_p, value)\
  (fge_p)->bits.bond = (value)

#define FuncGrp_Enc_Compress_Get(fge_p)\
  (Boolean_t)(fge_p)->bits.compres

#define FuncGrp_Enc_Compress_Put(fge_p, value)\
  (fge_p)->bits.compres = (value)

#define FuncGrp_Enc_Continue_Get(fge_p)\
  (Boolean_t)(fge_p)->bits.continu

#define FuncGrp_Enc_Continue_Put(fge_p, value)\
  (fge_p)->bits.continu = (value)

#define FuncGrp_Enc_Entered_Get(fge_p)\
  (Boolean_t)(fge_p)->bits.entered

#define FuncGrp_Enc_Entered_Put(fge_p, value)\
  (fge_p)->bits.entered = (value)

#define FuncGrp_Enc_Xpanded_Get(fge_p)\
  (Boolean_t)(fge_p)->bits.xpanded

#define FuncGrp_Enc_Xpanded_Incr(fge_p)\
  (fge_p)->bits.xpanded = (fge_p)->bits.xpanded + 1

#define FuncGrp_Enc_Xpanded_Put(fge_p, value)\
  (fge_p)->bits.xpanded = (value)

#define FuncGrp_Enc_Next_Get(fge_p)\
  (U16_t)(fge_p)->bits.next

#define FuncGrp_Enc_Next_Put(fge_p, value)\
  (fge_p)->bits.next = (value)

#define FuncGrp_Enc_Whole_Get(fge_p)\
  (fge_p)->whole

#define FuncGrp_Enc_Whole_Put(fge_p, value)\
  (fge_p)->whole = (value)

#define FuncGrp_Enc_Valid_Get(fge_p)\
  (Boolean_t)(fge_p)->bits.valid

#define FuncGrp_Enc_Valid_Put(fge_p, value)\
  (fge_p)->bits.valid = (value)
#else
#define FuncGrp_Enc_Atomid_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (U8_t)(fge_p)->bits.atom)

#define FuncGrp_Enc_Atomid_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.atom = (value); }

#define FuncGrp_Enc_Bond_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (U8_t)(fge_p)->bits.bond)

#define FuncGrp_Enc_Bond_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.bond = (value); }

#define FuncGrp_Enc_Compress_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (Boolean_t)(fge_p)->bits.compres)

#define FuncGrp_Enc_Compress_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.compres = (value); }

#define FuncGrp_Enc_Continue_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (Boolean_t)(fge_p)->bits.continu)

#define FuncGrp_Enc_Continue_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.continu = (value); }

#define FuncGrp_Enc_Entered_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (Boolean_t)(fge_p)->bits.entered)

#define FuncGrp_Enc_Entered_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.entered = (value); }

#define FuncGrp_Enc_Xpanded_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (Boolean_t)(fge_p)->bits.xpanded)

#define FuncGrp_Enc_Xpanded_Incr(fge_p)\
  (fge_p)->bits.xpanded = (fge_p)->bits.xpanded + 1

#define FuncGrp_Enc_Xpanded_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.xpanded = (value); }

#define FuncGrp_Enc_Next_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (U16_t)(fge_p)->bits.next)

#define FuncGrp_Enc_Next_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.next = (value); }

#define FuncGrp_Enc_Whole_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (fge_p)->whole)

#define FuncGrp_Enc_Whole_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->whole = (value); }

#define FuncGrp_Enc_Valid_Get(fge_p)\
  ((fge_p) < GBAddr ? HALT : (Boolean_t)(fge_p)->bits.valid)

#define FuncGrp_Enc_Valid_Put(fge_p, value)\
  { if ((fge_p) < GBAddr) HALT; else (fge_p)->bits.valid = (value); }
#endif

/** End of Field Access Macros for FuncGrp_Encode_t **/

/* Header for functional group description.  Each functional group has a set
   of names and a set of Slings that describe its possible variations.  There
   is also a unique number and some attribute flags and some other functional
   groups that may not exist rooted at the same atom.  These are call Not-
   Groups and are typically substructures of the given group.
*/

typedef struct s_fgirec
  {
  U32_t         fg_num;                    /* Functional group # */
  U16_t         not_grps[FG_MX_NOTGRP];    /* Groups not allowed */
  U16_t         flags;                     /* Flag bit-vector */
  U8_t          num_names;                 /* # names for this group */
  U8_t          num_slings;                /* # slings to describe group */
  } FuncGrp_Head_t;
#define FUNCGRPHEADSIZE sizeof (FuncGrp_Head_t)

#define FuncGrpM_Preserveable  0x1

/** Field Access Macros for FuncGrp_Header_t **/

/* Macro Prototypes
   U32_t     FuncGrp_Head_FGNum_Get     (FuncGrp_Head_t *);
   void      FuncGrp_Head_FGNum_Put     (FuncGrp_Head_t *, U32_t);
   U16_t     FuncGrp_Head_Flags_Get     (FuncGrp_Head_t *);
   void      FuncGrp_Head_Flags_Put     (FuncGrp_Head_t *, U16_t);
   Boolean_t FuncGrp_Head_FlagsPreserveable_Get (FuncGrp_Head_t *);
   void      FuncGrp_Head_FlagsPreserveable_Put (FuncGrp_Head_t *, Boolean_t);
   U16_t     FuncGrp_Head_NotGroup_Get  (FuncGrp_Head_t *, U8_t);
   void      FuncGrp_Head_NotGroup_Put  (FuncGrp_Head_t *, U8_t, U16_t);
   U8_t      FuncGrp_Head_NumNames_Get  (FuncGrp_Head_t *);
   void      FuncGrp_Head_NumNames_Put  (FuncGrp_Head_t *, U8_t);
   U8_t      FuncGrp_Head_NumSlings_Get (FuncGrp_Head_t *);
   void      FuncGrp_Head_NumSlings_Put (FuncGrp_Head_t *, U8_t);
*/

#ifndef XTR_DEBUG
#define FuncGrp_Head_FGNum_Get(head_p)\
  (head_p)->fg_num

#define FuncGrp_Head_FGNum_Put(head_p, value)\
  (head_p)->fg_num = (value)

#define FuncGrp_Head_Flags_Get(head_p)\
  (head_p)->flags

#define FuncGrp_Head_Flags_Put(head_p, value)\
  (head_p)->flags = (value)

#define FuncGrp_Head_FlagsPreserveable_Get(head_p)\
  ((head_p)->flags & FuncGrpM_Preserveable ? TRUE : FALSE)

#define FuncGrp_Head_FlagsPreserveable_Put(head_p, value)\
  if ((value) == TRUE)\
    (head_p)->flags |= FuncGrpM_Preserveable;\
  else\
    (head_p)->flags &= ~FuncGrpM_Preserveable

#define FuncGrp_Head_NotGroup_Get(head_p, idx)\
  (head_p)->not_grps[idx]

#define FuncGrp_Head_NotGroup_Put(head_p, idx, value)\
  (head_p)->not_grps[idx] = (value)

#define FuncGrp_Head_NumNames_Get(head_p)\
  (head_p)->num_names

#define FuncGrp_Head_NumNames_Put(head_p, value)\
  (head_p)->num_names = (value)

#define FuncGrp_Head_NumSlings_Get(head_p)\
  (head_p)->num_slings

#define FuncGrp_Head_NumSlings_Put(head_p, value)\
  (head_p)->num_slings = (value)
#else
#define FuncGrp_Head_FGNum_Get(head_p)\
  ((head_p) < GBAddr ? HALT : (head_p)->fg_num)

#define FuncGrp_Head_FGNum_Put(head_p, value)\
  { if ((head_p) < GBAddr) HALT; else (head_p)->fg_num = (value); }

#define FuncGrp_Head_Flags_Get(head_p)\
  ((head_p) < GBAddr ? HALT : (head_p)->flags)

#define FuncGrp_Head_Flags_Put(head_p, value)\
  { if ((head_p) < GBAddr) HALT; else (head_p)->flags = (value); }

#define FuncGrp_Head_FlagsPreserveable_Get(head_p)\
  ((head_p) < GBAddr ? HALT : ((head_p)->flags & FuncGrpM_Preserveable ?\
  TRUE : FALSE))

#define FuncGrp_Head_FlagsPreserveable_Put(head_p, value)\
  { if ((head_p) < GBAddr) HALT; else if ((value) == TRUE)\
    (head_p)->flags |= FuncGrpM_Preserveable;\
  else (head_p)->flags &= ~FuncGrpM_Preserveable; }

#define FuncGrp_Head_NotGroup_Get(head_p, idx)\
  ((head_p) < GBAddr || (idx) < 0 || (idx) >= FG_MX_NOTGRP ? HALT :\
 (head_p)->not_grps[idx])

#define FuncGrp_Head_NotGroup_Put(head_p, idx, value)\
  { if ((head_p) < GBAddr || (idx) < 0 || (idx) >= FG_MX_NOTGRP) HALT; else\
    (head_p)->not_grps[idx] = (value); }

#define FuncGrp_Head_NumNames_Get(head_p)\
  ((head_p) < GBAddr ? HALT : (head_p)->num_names)

#define FuncGrp_Head_NumNames_Put(head_p, value)\
  { if ((head_p) < GBAddr) HALT; else (head_p)->num_names = (value); }

#define FuncGrp_Head_NumSlings_Get(head_p)\
  ((head_p) < GBAddr ? HALT : (head_p)->num_slings)

#define FuncGrp_Head_NumSlings_Put(head_p, value)\
  { if ((head_p) < GBAddr) HALT; else (head_p)->num_slings = (value); }
#endif

/** End of Field Access Macros for FuncGrp_Header_t **/

/* Functional group descriptor.  Coupled with the header are pointers to
   arrays for the name strings and the Slings.  The key is here for mapping
   from the FG# to the key in the ISAM file.
*/

typedef struct s_fgrec
  {
  String_t     *names;                     /* Name array */
  Sling_t      *slings;                    /* Sling array */
  U32_t         key;                       /* Which ISAM key value */
  FuncGrp_Head_t headb;                    /* Header info */
  } FuncGrp_Record_t;
#define FUNCGRPRECORDSIZE sizeof (FuncGrp_Record_t)

/** Field Access Macros for FuncGrp_Record_t **/

/* Macro Prototypes
   FuncGrp_Head_t *FuncGrp_Rec_Head_Get (FuncGrp_Record_t *);
   U32_t     FuncGrp_Rec_Key_Get   (FuncGrp_Record_t *);
   void      FuncGrp_Rec_Key_Put   (FuncGrp_Record_t *, U32_t);
   String_t  FuncGrp_Rec_Name_Get  (FuncGrp_Record_t *, U16_t);
   void      FuncGrp_Rec_Name_Put  (FuncGrp_Record_t *, U16_t, String_t);
   Sling_t   FuncGrp_Rec_Sling_Get (FuncGrp_Record_t *, U16_t);
   void      FuncGrp_Rec_Sling_Put (FuncGrp_Record_t *, U16_t, Sling_t);
   String_t *FuncGrp_Rec_Name_Get  (FuncGrp_Record_t *);
   Sling_t  *FuncGrp_Rec_Sling_Get (FuncGrp_Record_t *);
*/

#ifndef XTR_DEBUG
#define FuncGrp_Rec_Head_Get(rec_p)\
  &(rec_p)->headb

#define FuncGrp_Rec_Key_Get(rec_p)\
  (rec_p)->key

#define FuncGrp_Rec_Key_Put(rec_p, value)\
  (rec_p)->key = (value)

#define FuncGrp_Rec_Name_Get(rec_p, idx)\
  (rec_p)->names[idx]

#define FuncGrp_Rec_Name_Put(rec_p, idx, value)\
  (rec_p)->names[idx] = (value)

#define FuncGrp_Rec_Sling_Get(rec_p, idx)\
  (rec_p)->slings[idx]

#define FuncGrp_Rec_Sling_Put(rec_p, idx, value)\
  (rec_p)->slings[idx] = (value)

#define FuncGrp_RecName_Get(rec_p)\
  (rec_p)->names

#define FuncGrp_RecSling_Get(rec_p)\
  (rec_p)->slings
#else
#define FuncGrp_Rec_Head_Get(rec_p)\
  ((rec_p) < GBAddr ? HALT : &(rec_p)->headb)

#define FuncGrp_Rec_Key_Get(rec_p)\
  ((rec_p) < GBAddr ? HALT : (rec_p)->key)

#define FuncGrp_Rec_Key_Put(rec_p, value)\
  { if ((rec_p) < GBAddr) HALT; else (rec_p)->key = (value); }

#define FuncGrp_Rec_Name_Get(rec_p, idx)\
  ((rec_p) < GBAddr || (idx) < 0 || (idx) >= (rec_p)->headb.num_names ? HALTST\
  : (rec_p)->names[idx])

#define FuncGrp_Rec_Name_Put(rec_p, idx, value)\
  { if ((rec_p) < GBAddr || (idx) < 0)\
  HALT; else (rec_p)->names[idx] = (value); }

#define FuncGrp_Rec_Sling_Get(rec_p, idx)\
  ((rec_p) < GBAddr || (idx) < 0 || (idx) >= (rec_p)->headb.num_slings ?\
  HALTSL : (Sling_t)(rec_p)->slings[idx])

#define FuncGrp_Rec_Sling_Put(rec_p, idx, value)\
  { if ((rec_p) < GBAddr || (idx) < 0)\
  HALT; else (rec_p)->slings[idx] = (value); }

#define FuncGrp_RecName_Get(rec_p)\
  ((rec_p) < GBAddr ? ((FuncGrp_Record_t *)HALTP)->names : (rec_p)->names)

#define FuncGrp_RecSling_Get(rec_p)\
  ((rec_p) < GBAddr ? ((FuncGrp_Record_t *)HALTP)->slings : (rec_p)->slings)
#endif

/** End of Field Access Macros for FuncGrp_Record_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void              FuncGrp_Rec_Destroy  (FuncGrp_Record_t *);
void              FuncGrp_Rec_Dump     (FuncGrp_Record_t *, FileDsc_t *);
FuncGrp_Record_t *FuncGrp_Rec_Read     (U32_t, Isam_Control_t *);
void              FuncGrp_Rec_Write    (FuncGrp_Record_t *, Isam_Control_t *);


FuncGrp_Encode_t *FGEncode_Read_Close  (FILE *);
void              FGEncode_Write_Close (FILE *, FuncGrp_Encode_t *, U32_t);
FILE             *FGEncode_Open        (char *, U8_t);


/*** Global Variables ***/

/* End of Funcgroup_File.H */
#endif
