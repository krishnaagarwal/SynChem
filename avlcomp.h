#ifndef _H_AVLCOMP_
#define _H_AVLCOMP_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     AVLCOMP.H
*
*    This module contains the definions for the abstraction of the 
*    Available Compounds Library.  
*    
*     
*    
*
*  Creation Date:
*
*    01-Aug-1993
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita	xxx
*
******************************************************************************/

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif

/*** Literal Values ***/

#define AVC_COMPARE_EQUAL        (S16_t) 0
#define AVC_COMPARE_GREATER      (S16_t) 1
#define AVC_COMPARE_LESS         (S16_t) -1
 
#define AVC_INDEX_NOT_FOUND      (U32_t) -1

#define AVC_INIT_EXISTS          (U8_t)  0
#define AVC_INIT_INFO            (U8_t)  1

#define AVC_KEY_NOT_FOUND        (U32_t) 0


/*** Data Structures ***/

/* The dictionary used for compressing the slings is stored as a 
   trie.  The dictionary trie has been transformed into a simple 
   table to avoid the use of pointers in a linked list implementation.
*/

typedef S8_t  Avc_Trie_Table_t;

typedef S8_t  Avc_Slg_Char_t;
#define AVC_TBL_CHAR_SZ      (sizeof (Avc_Slg_Char_t))
#define AVC_NULL_CHAR        (Avc_Slg_Char_t)  '\0'

typedef U8_t  Avc_Slg_Code_t;
#define AVC_TBL_CODE_SZ      (sizeof (Avc_Slg_Code_t))
#define AVC_NULL_CODE        (Avc_Slg_Code_t)   0

typedef U8_t  Avc_Slg_Next_t;
#define AVC_TBL_NEXT_SZ      (sizeof (Avc_Slg_Next_t))
#define AVC_NULL_NEXT        (Avc_Slg_Next_t)   0

#define AVC_TBL_ENTRY_LEN\
        (AVC_TBL_CHAR_SZ + AVC_TBL_NEXT_SZ + AVC_TBL_CODE_SZ)
#define AVC_TBL_CHAR_OFF      0
#define AVC_TBL_NEXT_OFF      AVC_TBL_CHAR_SZ
#define AVC_TBL_CODE_OFF      (AVC_TBL_CHAR_SZ + AVC_TBL_NEXT_SZ)

/** Field Access Macros for Avc_Trie_Table_t **/

/* Macro Prototypes
   NYI
*/

#ifndef AVL_DEBUG
#define Avc_Trie_Char_Get(entry_p)\
 *((Avc_Slg_Char_t *) (entry_p))

#define Avc_Trie_Code_Get(entry_p)\
 *((Avc_Slg_Code_t *) (entry_p + AVC_TBL_CODE_OFF))

#define Avc_Trie_Next_Get(entry_p)\
 *((Avc_Slg_Next_t *) (entry_p + AVC_TBL_NEXT_OFF))
#else
#define Avc_Trie_Char_Get(entry_p)\
  ((entry_p) < GBAddr ? HALT : *((Avc_Slg_Char_t *) (entry_p)))

#define Avc_Trie_Code_Get(entry_p)\
  ((entry_p) < GBAddr ? HALT : *((Avc_Slg_Code_t *)\
  (entry_p + AVC_TBL_CODE_OFF)))

#define Avc_Trie_Next_Get(entry_p)\
  ((entry_p) < GBAddr ? HALT : *((Avc_Slg_Next_t *)\
  (entry_p + AVC_TBL_NEXT_OFF)))
#endif

/** End of Field Access Macros for Trie_Table_t **/

/*  The data structures that are used by the Available Compounds
    Library routines to search for a sling and retrieve information
    about the compound represented by that sling are accessed through
    this structure.  
*/

typedef struct avc_control_s
  {
  char             *dirpath;                  /* directory path */
  Avc_Trie_Table_t *dict_tbl_p;               /* Dictionary trie table */
  Avc_Slg_Code_t  **cslgs_p;                  /* List of compressed slings */
  U32_t            *keys_p;                   /* Parallel List of keys */
  U32_t             slg_count;                /* Total number of slings */
  Isam_Control_t   *infof_p;                  /* Isam file for sling info */
  } Avc_Control_t;
#define AVC_CONTROL_SZ (sizeof (Avc_Control_t))

/** Field Access Macros for Avc_Control_t **/

/* Macro Prototypes
   Avc_Slg_Code_t  **Avc_Control_CSlgs_Get (Avc_Control_t);
   void        Avc_Control_CSlgs_Put (Avc_Control_t, Avc_Slg_Code_t  **);
   Avc_Trie_Table_t *Avc_Control_DictTbl_Get  (Avc_Control_t);
   void        Avc_Control_DictTbl_Put  (Avc_Control_t, Avc_Trie_Table_t *);
   char       *Avc_Control_Dir_Get      (Avc_Control_t);
   void        Avc_Control_Dir_Put      (Avc_Control_t, char *);
   Isam_Control_t *Avc_Control_InfoFICB_Get (Avc_Control_t);
   void        Avc_Control_InfoFICB_Put (Avc_Control_t, Isam_Control_t *);
   U32_t      *Avc_Control_Keys_Get     (Avc_Control_t);
   void        Avc_Control_Keys_Put     (Avc_Control_t, U32_t *);
   U32_t       Avc_Control_SlgCnt_Get   (Avc_Control_t);
   void        Avc_Control_SlgCnt_Put   (Avc_Control_t, U32_t);
*/

#ifndef AVL_DEBUG
#define Avc_Control_CSlgs_Get(tf_p)\
  (tf_p).cslgs_p

#define Avc_Control_CSlgs_Put(tf_p, val)\
  (tf_p).cslgs_p = (val)

#define Avc_Control_DictTbl_Get(tf_p)\
  (tf_p).dict_tbl_p

#define Avc_Control_DictTbl_Put(tf_p, val)\
  (tf_p).dict_tbl_p = (val)

#define Avc_Control_Dir_Get(tf_p)\
  (tf_p).dirpath

#define Avc_Control_Dir_Put(tf_p, val)\
  (tf_p).dirpath = (val)

#define Avc_Control_InfoFICB_Get(tf_p)\
  (tf_p).infof_p

#define Avc_Control_InfoFICB_Put(tf_p, val)\
  (tf_p).infof_p = (val)

#define Avc_Control_Keys_Get(tf_p)\
  (tf_p).keys_p

#define Avc_Control_Keys_Put(tf_p, val)\
  (tf_p).keys_p = (val)

#define Avc_Control_SlgCnt_Get(tf_p)\
  (tf_p).slg_count

#define Avc_Control_SlgCnt_Put(tf_p, val)\
  (tf_p).slg_count = (val)
#else
#define Avc_Control_CSlgs_Get(tf_p)\
  (&(tf_p) < GBAddr ? HALT : (tf_p).cslgs_p)

#define Avc_Control_CSlgs_Put(tf_p, val)\
  { if (&(tf_p) < GBAddr) HALT; else (tf_p).cslgs_p = (val); }

#define Avc_Control_DictTbl_Get(tf_p)\
  (&(tf_p) < GBAddr ? (Avc_Trie_Table_t *)HALTP : (tf_p).dict_tbl_p)

#define Avc_Control_DictTbl_Put(tf_p, val)\
  { if (&(tf_p) < GBAddr) HALT; else (tf_p).dict_tbl_p = (val); }

#define Avc_Control_Dir_Get(tf_p)\
  (&(tf_p) < GBAddr ? HALT : (tf_p).dirpath)

#define Avc_Control_Dir_Put(tf_p, val)\
  { if (&(tf_p) < GBAddr) HALT; else (tf_p).dirpath = (val); }

#define Avc_Control_InfoFICB_Get(tf_p)\
  (&(tf_p) < GBAddr ? HALTP : (tf_p).infof_p)

#define Avc_Control_InfoFICB_Put(tf_p, val)\
  { if (&(tf_p) < GBAddr) HALT; else (tf_p).infof_p = (val); }

#define Avc_Control_Keys_Get(tf_p)\
  (&(tf_p) < GBAddr ? (U32_t *)HALTP : (tf_p).keys_p)

#define Avc_Control_Keys_Put(tf_p, val)\
  { if (&(tf_p) < GBAddr) HALT; else (tf_p).keys_p = (val); }

#define Avc_Control_SlgCnt_Get(tf_p)\
  (&(tf_p) < GBAddr ? HALT : (tf_p).slg_count)

#define Avc_Control_SlgCnt_Put(tf_p, val)\
  { if (&(tf_p) < GBAddr) HALT; else (tf_p).slg_count = (val); }
#endif

/** End of Field Access Macros for Avc_Control_t **/

/*  Record Zero of the runtime avlcomp data file stores the version number
    and either the number of information records (or compressed slings/keys)
    or the size of the dictionary table.
*/

typedef struct avc_rec_zero_s
  {
  U32_t           cnt_sz;                     /* count or size */
  U32_t           vers_num;                   /* version number */
  } Avc_RecZero_t;
#define AVC_RECZERO_SZ sizeof (Avc_RecZero_t)

/** Field Access Macros for Avc_RecZero_t **/

/* Macro Prototypes
   U32_t       Avc_RecZero_CntSz_Get    (Avc_RecZero_t);
   void        Avc_RecZero_CntSz_Put    (Avc_RecZero_t, U32_t);
   U32_t       Avc_RecZero_VerNum_Get   (Avc_RecZero_t);
   void        Avc_RecZero_VerNum_Put   (Avc_RecZero_t, U32_t);
*/

#ifndef AVL_DEBUG
#define Avc_RecZero_CntSz_Get(zr)\
  (zr).cnt_sz

#define Avc_RecZero_CntSz_Put(zr, val)\
  (zr).cnt_sz = (val)

#define Avc_RecZero_VerNum_Get(zr)\
  (zr).vers_num

#define Avc_RecZero_VerNum_Put(zr, val)\
  (zr).vers_num = (val)
#else
#define Avc_RecZero_CntSz_Get(zr)\
  (&(zr) < GBAddr ? HALT : (zr).cnt_sz)

#define Avc_RecZero_CntSz_Put(zr, val)\
  { if (&(zr) < GBAddr) HALT; else (zr).cnt_sz = (val); }

#define Avc_RecZero_VerNum_Get(zr)\
  (&(zr) < GBAddr ? HALT : (zr).vers_num)

#define Avc_RecZero_VerNum_Put(zr, val)\
  { if (&(zr) < GBAddr) HALT; else (zr).vers_num = (val); }
#endif

/** End of Field Access Macros for Avc_RecZero_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void            AvcLib_Control_Destroy (void);
U32_t           AvcLib_Exists          (char *);
Avi_CmpInfo_t  *AvcLib_Info            (U32_t);
void            AvcLib_Init            (char *, U8_t);
U32_t           AvcLib_Key             (char *);
void            AvcLib_Sling_Encode    (char *, Avc_Slg_Code_t *,
  Avc_Trie_Table_t *);

/*** Global Variables ***/

/* End of AVLCOMP.H */
#endif
