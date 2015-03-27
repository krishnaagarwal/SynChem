#ifndef _H_AVLDICT_
#define _H_AVLDICT_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     AVLDICT.H
*
*    This module is the ???
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

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

/*** Literal Values ***/

#define  AVD_COUNT_CUTOFF          2
#define  AVD_CANDS_MAX_NUM         1024
#define  AVD_CHARS_MAX_NUM         128
#define  AVD_EOS                   (U8_t) 0
#define  AVD_PATTS_MAX_NUM         255
#define  AVD_SLING_LENMAX          256
#define  AVD_SLGS_MAX_NUM          8192
#define  AVD_PATT_TBL_MAX_SZ       4096
#define  AVD_PATT_TBL_MAX_OFFSET   255
#define  AVD_PATT_NODE_INIT_VAL    0
#define  AVD_SLG_NODE_INIT_VAL     1

#define  AVD_DATAFILE_CSLGKEY      "avlcomp.cslgkey"
#define  AVD_DATAFILE_DICT_TBL     "avlcomp.dict_tbl"
#define  AVD_VERSION_CSLGKEY       (U32_t) 2
#define  AVD_VERSION_DICT_TBL      (U32_t) 1

/*** Data Structures ***/

/*  A node structure defined both for use in constructing the 
    slings trie and the dictionary trie.
*/

typedef struct avd_trie_node_s
  {
  struct avd_trie_node_s  *list;              /* rest of trie list at level n */
  struct avd_trie_node_s  *next;              /* trie list at next level n+1 */
  S32_t           value;                      /* value stored in trie node */
  U8_t            ch;                         /* char stored in trie node */
  }  Avd_Trie_Node_t;
#define  AVD_TRIE_NODE_SZ  sizeof (Avd_Trie_Node_t)

/** Field Access Macros for Avd_Trie_Node_t **/

/* Macro Prototypes
   U8_t        Avd_TrieNode_Char_Get    (Avd_Trie_Node_t *);
   void        Avd_TrieNode_Char_Put    (Avd_Trie_Node_t *, U8_t);
   Avd_Trie_Node_t *Avd_TrieNode_List_Get (Avd_Trie_Node_t *);
   void        Avd_TrieNode_List_Put    (Avd_Trie_Node_t *, Avd_Trie_Node_t *);
   Avd_Trie_Node_t *Avd_TrieNode_Next_Get (Avd_Trie_Node_t *);
   void        Avd_TrieNode_Next_Put    (Avd_Trie_Node_t *, Avd_Trie_Node_t *);
   S32_t       Avd_TrieNode_Value_Get   (Avd_Trie_Node_t *);
   void        Avd_TrieNode_Value_Put   (Avd_Trie_Node_t *, S32_t);
*/

#ifndef AVL_DEBUG
#define Avd_TrieNode_Char_Get(tn_p)\
  (tn_p)->ch

#define Avd_TrieNode_Char_Put(tn_p, val)\
  (tn_p)->ch = (val)

#define Avd_TrieNode_List_Get(tn_p)\
  (tn_p)->list

#define Avd_TrieNode_List_Put(tn_p, val)\
  (tn_p)->list = (val)

#define Avd_TrieNode_Next_Get(tn_p)\
  (tn_p)->next

#define Avd_TrieNode_Next_Put(tn_p, val)\
  (tn_p)->next = (val)

#define Avd_TrieNode_Value_Get(tn_p)\
  (tn_p)->value

#define Avd_TrieNode_Value_Put(tn_p, val)\
  (tn_p)->value = (val)
#else
#define Avd_TrieNode_Char_Get(tn_p)\
  ((tn_p) < GBAddr ? HALT : (tn_p)->ch)

#define Avd_TrieNode_Char_Put(tn_p, val)\
  { if ((tn_p) < GBAddr) HALT; else (tn_p)->ch = (val); }

#define Avd_TrieNode_List_Get(tn_p)\
  ((tn_p) < GBAddr ? HALT : (tn_p)->list)

#define Avd_TrieNode_List_Put(tn_p, val)\
  { if ((tn_p) < GBAddr) HALT; else (tn_p)->list = (val); }

#define Avd_TrieNode_Next_Get(tn_p)\
  ((tn_p) < GBAddr ? HALT : (tn_p)->next)

#define Avd_TrieNode_Next_Put(tn_p, val)\
  { if ((tn_p) < GBAddr) HALT; else (tn_p)->next = (val); }

#define Avd_TrieNode_Value_Get(tn_p)\
  ((tn_p) < GBAddr ? HALT : (tn_p)->value)

#define Avd_TrieNode_Value_Put(tn_p, val)\
  { if ((tn_p) < GBAddr) HALT; else (tn_p)->value = (val); }
#endif

/** End of Field Access Macros for Avd_Trie_Node_t **/

/*  The table of prefixes, taken from the sling trie, from which 
    candidates are chosen.  The change flag is set if a merge trie
    operation or a pattern extraction operation changes the subtrie
    from where this prefix originated.
*/

typedef struct avd_pref_tbl_rec_s
  {
  U8_t            prefix [AVD_SLING_LENMAX];  /* prefix from sling tree */ 
  S32_t           save;                       /* estimated savings */
  Boolean_t       chg;                        /* sling subtree changed? */ 
  }  Avd_Prefix_Table_t;
#define AVD_PREFIX_TABLE_SZ sizeof (Avd_Prefix_Tabl_t)

/** Field Access Macros for Avd_Prefix_Table_t **/

/* Macro Prototypes
   Boolean_t   Avd_PreTbl_Change_Get    (Avd_Prefix_Table_t);
   void        Avd_PreTbl_Change_Put    (Avd_Prefix_Table_t, Boolean_t);
   U8_t       *Avd_PreTbl_Prefix_Get    (Avd_Prefix_Table_t);
   void        Avd_PreTbl_Prefix_Put    (Avd_Prefix_Table_t, U8_t *);
   S32_t       Avd_PreTbl_Save_Get      (Avd_Prefix_Table_t);
   void        Avd_PreTbl_Save_Put      (Avd_Prefix_Table_t, S32_t);
*/

#ifndef AVL_DEBUG
#define Avd_PreTbl_Change_Get(pt)\
  (pt).chg

#define Avd_PreTbl_Change_Put(pt, val)\
  (pt).chg = (val)

#define Avd_PreTbl_Prefix_Get(pt)\
  (pt).prefix

#define Avd_PreTbl_Prefix_Put(pt, val)\
  (pt).prefix = (val)

#define Avd_PreTbl_Save_Get(pt)\
  (pt).save

#define Avd_PreTbl_Save_Put(pt, val)\
  (pt).save = (val)
#else
#define Avd_PreTbl_Change_Get(pt)\
  ((pt) < GBAddr ? HALT : (pt).chg)

#define Avd_PreTbl_Change_Put(pt, val)\
  { if ((pt) < GBAddr) HALT; else (pt).chg = (val); }

#define Avd_PreTbl_Prefix_Get(pt)\
  ((pt) < GBAddr ? HALT : (pt).prefix)

#define Avd_PreTbl_Prefix_Put(pt, val)\
  { if ((pt) < GBAddr) HALT; else (pt).prefix = (val); }

#define Avd_PreTbl_Save_Get(pt)\
  ((pt) < GBAddr ? HALT : (pt).save)

#define Avd_PreTbl_Save_Put(pt, val)\
  { if ((pt) < GBAddr) HALT; else (pt).save = (val); }
#endif

/** End of Field Access Macros for Avd_Prefix_Table_t **/

/*  This list of candidates are those prefixes with the highest
    estimated savings.  Candidates are removed from the sling trie
    whenever they float to the top of the trie due to a merge operation.
*/

typedef struct avd_candidate_s
  {
  S32_t           save;                       /* estimated savings */
  U8_t           *patt;                       /* prefix pattern */
  }  Avd_Candidate_t;
#define AVD_CANDIDATE_SZ sizeof (Avd_Candidate_t)

/** Field Access Macros for Avd_Candidate_t **/

/* Macro Prototypes
   U8_t       *Avd_Candidate_Patt_Get  (Avd_Candidate_t);
   void        Avd_Candidate_Patt_Put  (Avd_Candidate_t, U8_t *);
   S32_t       Avd_Candidate_Save_Get  (Avd_Candidate_t);
   void        Avd_Candidate_Save_Put  (Avd_Candidate_t, S32_t);
*/

#ifndef AVL_DEBUG
#define Avd_Candidate_Patt_Get(cd)\
  (cd).patt

#define Avd_Candidate_Patt_Put(cd, val)\
  (cd).patt = (val)

#define Avd_Candidate_Save_Get(cd)\
  (cd).save

#define Avd_Candidate_Save_Put(cd, val)\
  (cd).save = (val)
#else
#define Avd_Candidate_Patt_Get(cd)\
  ((cd) < GBAddr ? HALT : (cd).patt)

#define Avd_Candidate_Patt_Put(cd, val)\
  { if ((cd) < GBAddr) HALT; else (cd).patt = (val); }

#define Avd_Candidate_Save_Get(cd)\
  ((cd) < GBAddr ? HALT : (cd).save)

#define Avd_Candidate_Save_Put(cd, val)\
  { if ((cd) < GBAddr) HALT; else (cd).save = (val); }
#endif

/** End of Field Access Macros for Avd_Candidate_t **/

/*  The queue is used to convert the dictionary trie from a linked listed
    implementation to an array with offsets implementation.
*/
    
typedef struct avd_queue_node_s
  {
  Avd_Trie_Node_t *trielist_p;                /* The rest of the trie list */
  Avc_Trie_Table_t *tb_entry_p;               /* Table entry for this node */
  struct avd_queue_node_s *queue_next_p;      /* Next queue entry */
  }  Avd_Queue_Node_t;
#define AVD_QUEUE_NODE_SZ sizeof (Avd_Queue_Node_t)

/** Field Access Macros for Avd_Queue_Node_t **/

/* Macro Prototypes
   Avd_Queue_Node_t *Avd_QueueNode_Next_Get    (Avd_Queue_Node_t *);
   void              Avd_QueueNode_Next_Put    (Avd_Queue_Node_t *, 
     Avd_Queue_Node_t *);
   Avc_Trie_Table_t  *Avd_QueueNode_TbEnt_Get  (Avd_Queue_Node_t *);
   void               Avd_QueueNode_TbEnt_Put  (Avd_Queue_Node_t *, 
     Avc_Trie_Table_t *);
   Avd_Trie_Node_t   *Avd_QueueNode_TrList_Get (Avd_Queue_Node_t *);
   void               Avd_QueueNode_TrList_Put (Avd_Queue_Node_t *, 
     Avd_Trie_Node_t *);
*/

#ifndef AVL_DEBUG
#define Avd_QueueNode_Next_Get(qn_p)\
  (qn_p)->queue_next_p

#define Avd_QueueNode_Next_Put(qn_p, val)\
  (qn_p)->queue_next_p = (val)

#define Avd_QueueNode_TbEnt_Get(qn_p)\
  (qn_p)->tb_entry_p

#define Avd_QueueNode_TbEnt_Put(qn_p, val)\
  (qn_p)->tb_entry_p = (val)

#define Avd_QueueNode_TrList_Get(qn_p)\
  (qn_p)->trielist_p

#define Avd_QueueNode_TrList_Put(qn_p, val)\
  (qn_p)->trielist_p = (val)
#else
#define Avd_QueueNode_Next_Get(qn_p)\
  ((qn_p) < GBAddr ? HALT : (qn_p)->queue_next_p)

#define Avd_QueueNode_Next_Put(qn_p, val)\
  { if ((qn_p) < GBAddr) HALT; else (qn_p)->queue_next_p = (val); }

#define Avd_QueueNode_TbEnt_Get(qn_p)\
  ((qn_p) < GBAddr ? HALT : (qn_p)->tb_entry_p)

#define Avd_QueueNode_TbEnt_Put(qn_p, val)\
  { if ((qn_p) < GBAddr) HALT; else (qn_p)->tb_entry_p = (val); }

#define Avd_QueueNode_TrList_Get(qn_p)\
  ((qn_p) < GBAddr ? HALT : (qn_p)->trielist_p)

#define Avd_QueueNode_TrList_Put(qn_p, val)\
  { if ((qn_p) < GBAddr) HALT; else (qn_p)->trielist_p = (val); }
#endif

/** End of Field Access Macros for Avd_Queue_Node_t **/

/*  A structure to store a sling, its compressed form, and its key
    into the information file.
*/

typedef struct avd_slg_key_s
  {
  U8_t           *sling;                      /* sling */
  Avc_Slg_Code_t *comp;                       /* compressed sling */
  U32_t           key;                        /* key into ISAM info file */
  U16_t           clen;                       /* length of comp slg */
  }   Avd_Slg_Key_t;
#define AVD_SLINGKEY_SZ sizeof (Avd_Slg_Key_t)

/** Field Access Macros for a Avd_Slg_Key_t **/

/* Macro Prototypes
   U16_t       Avd_SlgKey_CLen_Get     (Avd_Slg_Key_t);
   void        Avd_SlgKey_CLen_Put     (Avd_Slg_Key_t, U16_t);
   Avc_Slg_Code_t *Avd_SlgKey_CSlg_Get (Avd_Slg_Key_t);
   void        Avd_SlgKey_CSlg_Put     (Avd_Slg_Key_t, Avc_Slg_Code_t *);
   U32_t       Avd_SlgKey_Key_Get      (Avd_Slg_Key_t);
   void        Avd_SlgKey_Key_Put      (Avd_Slg_Key_t, U32_t);
   U8_t       *Avd_SlgKey_Sling_Get    (Avd_Slg_Key_t);
   void        Avd_SlgKey_Sling_Put    (Avd_Slg_Key_t, U8_t *);
*/

#ifndef AVL_DEBUG
#define Avd_SlgKey_CLen_Get(sk)\
  (sk).clen

#define Avd_SlgKey_CLen_Put(sk, val)\
  (sk).clen = (val)

#define Avd_SlgKey_CSlg_Get(sk)\
  (sk).comp

#define Avd_SlgKey_CSlg_Put(sk, val)\
  (sk).comp = (val)

#define Avd_SlgKey_Key_Get(sk)\
  (sk).key

#define Avd_SlgKey_Key_Put(sk, val)\
  (sk).key = (val)

#define Avd_SlgKey_Sling_Get(sk)\
  (sk).sling

#define Avd_SlgKey_Sling_Put(sk, val)\
  (sk).sling = (val)
#else
#define Avd_SlgKey_CLen_Get(sk)\
  ((sk) < GBAddr ? HALT : (sk).clen)

#define Avd_SlgKey_CLen_Put(sk, val)\
  { if ((sk) < GBAddr) HALT; else (sk).clen = (val); }

#define Avd_SlgKey_CSlg_Get(sk)\
  ((sk) < GBAddr ? HALT : (sk).comp)

#define Avd_SlgKey_CSlg_Put(sk, val)\
  { if ((sk) < GBAddr) HALT; else (sk).comp = (val); }

#define Avd_SlgKey_Key_Get(sk)\
  ((sk) < GBAddr ? HALT : (sk).key)

#define Avd_SlgKey_Key_Put(sk, val)\
  { if ((sk) < GBAddr) HALT; else (sk).key = (val); }

#define Avd_SlgKey_Sling_Get(sk)\
  ((sk) < GBAddr ? HALT : (sk).sling)

#define Avd_SlgKey_Sling_Put(sk, val)\
  { if ((sk) < GBAddr) HALT; else (sk).sling = (val); }
#endif

/** End of Field Access Macros for Avd_Slg_Key_t **/

/*  A structure to store the static variables of the module--that is,
    those for which it was deemed easier to access directly rather than
    pass as parameters.
*/

typedef struct avd_dict_ctrl_s
  {
  Isam_Control_t  info_b;                     /* runtime slg info file */
  FileDsc_t       pattbl_b;                   /* patt table file */
  FileDsc_t       cslgkey_b;                  /* comp slg/key file */
  Avc_RecZero_t   infozrec_b;                 /* zero record for info file */
  Avc_RecZero_t   pattzrec_b;                 /* zero record for pattbl file */
  Avc_RecZero_t   cslgzrec_b;                 /* zero record for cslg file */
  Avd_Prefix_Table_t tbltops[AVD_CHARS_MAX_NUM]; /* top row of sling trie */
  Avd_Candidate_t  candidates[AVD_CANDS_MAX_NUM]; /* list of candidates */
  U8_t             extract_patt[AVD_SLING_LENMAX]; /* the extracted pattern */
  U8_t            *extpat_p;                  /* the extracted pattern */
  U8_t            *topprefix_p;               /* current best prefix */
  S16_t            depth;                     /* depth of trie traversal */
  S32_t            cursave;                   /* current estimated savings */
  Boolean_t        cand_sorted;               /* cands been sorted? */
  } Avd_Dict_Control_t;

/** Field Access Macros for Avd_Dict_Control_t **/

/* Macro Prototypes
   Avd_Candidate_t *Avd_DictCtrl_Cands_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_Cands_Put (Avd_Dict_Control_t, 
                 Avd_Candidate_t *);
   Boolean_t   Avd_DictCtrl_CandSort_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_CandSort_Put (Avd_Dict_Control_t, Boolean_t);
   FileDsc_t   Avd_DictCtrl_CslgKeyFC_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_CslgKeyFC_Put (Avd_Dict_Control_t, FileDsc_t);
   Avc_RecZero_t Avd_DictCtrl_CslgKeyZR_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_CslgKeyZR_Put (Avd_Dict_Control_t, Avc_RecZero_t);
   S32_t       Avd_DictCtrl_CurSave_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_CurSave_Put (Avd_Dict_Control_t, S32_t);
   S16_t       Avd_DictCtrl_Depth_Get   (Avd_Dict_Control_t);
   void        Avd_DictCtrl_Depth_Put   (Avd_Dict_Control_t, S16_t);
   U8_t       *Avd_DictCtrl_ExtPatPtr_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_ExtPatPtr_Put (Avd_Dict_Control_t, U8_t *);
   U8_t       *Avd_DictCtrl_ExtPatt_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_ExtPatt_Put (Avd_Dict_Control_t, U8_t *);
   Isam_Control_t Avd_DictCtrl_InfoFC_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_InfoFC_Put (Avd_Dict_Control_t, Isam_Control_t);
   Avc_RecZero_t Avd_DictCtrl_InfoZR_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_InfoZR_Put (Avd_Dict_Control_t, Avc_RecZero_t);
   FileDsc_t   Avd_DictCtrl_PatTblFC_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_PatTblFC_Put (Avd_Dict_Control_t, FileDsc_t);
   Avc_RecZero_t Avd_DictCtrl_PatTblZR_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_PatTblZR_Put (Avd_Dict_Control_t, Avc_RecZero_t);
   Avd_Prefix_Table_t *Avd_DictCtrl_TblTops_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_TblTops_Put (Avd_Dict_Control_t, 
                 Avd_Prefix_Table_t *);
   U8_t       *Avd_DictCtrl_TopPref_Get (Avd_Dict_Control_t);
   void        Avd_DictCtrl_TopPref_Put (Avd_Dict_Control_t, U8_t *);
*/

#ifndef AVL_DEBUG
#define Avd_DictCtrl_Cands_Get(dc)\
  (dc).candidates

#define Avd_DictCtrl_Cands_Put(dc, val)\
  (dc).candidates = (val)

#define Avd_DictCtrl_CandSort_Get(dc)\
  (dc).cand_sorted

#define Avd_DictCtrl_CandSort_Put(dc, val)\
  (dc).cand_sorted = (val)

#define Avd_DictCtrl_CslgKeyFC_Get(dc)\
  (dc).cslgkey_b

#define Avd_DictCtrl_CslgKeyFC_Put(dc, val)\
  (dc).cslgkey_b = (val)

#define Avd_DictCtrl_CslgKeyZR_Get(dc)\
  (dc).cslgzrec_b

#define Avd_DictCtrl_CslgKeyZR_Put(dc, val)\
  (dc).cslgzrec_b = (val)

#define Avd_DictCtrl_CurSave_Get(dc)\
  (dc).cursave

#define Avd_DictCtrl_CurSave_Put(dc, val)\
  (dc).cursave = (val)

#define Avd_DictCtrl_Depth_Get(dc)\
  (dc).depth

#define Avd_DictCtrl_Depth_Put(dc, val)\
  (dc).depth = (val)

#define Avd_DictCtrl_ExtPatPtr_Get(dc)\
  (dc).extpat_p

#define Avd_DictCtrl_ExtPatPtr_Put(dc, val)\
  (dc).extpat_p = (val)

#define Avd_DictCtrl_ExtPatt_Get(dc)\
  (dc).extract_patt

#define Avd_DictCtrl_ExtPatt_Put(dc, val)\
  (dc).extract_patt = (val)

#define Avd_DictCtrl_InfoFC_Get(dc)\
  (dc).info_b

#define Avd_DictCtrl_InfoFC_Put(dc, val)\
  (dc).info_b = (val)

#define Avd_DictCtrl_InfoZR_Get(dc)\
  (dc).infozrec_b

#define Avd_DictCtrl_InfoZR_Put(dc, val)\
  (dc).infozrec_b = (val)

#define Avd_DictCtrl_PatTblFC_Get(dc)\
  (dc).pattbl_b

#define Avd_DictCtrl_PatTblFC_Put(dc, val)\
  (dc).pattbl_b = (val)

#define Avd_DictCtrl_PatTblZR_Get(dc)\
  (dc).pattzrec_b

#define Avd_DictCtrl_PatTblZR_Put(dc, val)\
  (dc).pattzrec_b = (val)

#define Avd_DictCtrl_TblTops_Get(dc)\
  (dc).tbltops

#define Avd_DictCtrl_TblTops_Put(dc, val)\
  (dc).tbltops = (val)

#define Avd_DictCtrl_TopPref_Get(dc)\
  (dc).topprefix_p

#define Avd_DictCtrl_TopPref_Put(dc, val)\
  (dc).topprefix_p = (val)
#else
#define Avd_DictCtrl_Cands_Get(dc)\
  (&(dc) < GBAddr ? ((Avd_Dict_Control_t *)HALTP)->candidates : (dc).candidates)

#define Avd_DictCtrl_Cands_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).candidates = (val); }

#define Avd_DictCtrl_CandSort_Get(dc)\
  (&(dc) < GBAddr ? HALT : (dc).cand_sorted)

#define Avd_DictCtrl_CandSort_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).cand_sorted = (val); }

#define Avd_DictCtrl_CslgKeyFC_Get(dc)\
  (&(dc) < GBAddr ? ((Avd_Dict_Control_t *)HALTP)->cslgkey_b : (dc).cslgkey_b)

#define Avd_DictCtrl_CslgKeyFC_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).cslgkey_b = (val); }

#define Avd_DictCtrl_CslgKeyZR_Get(dc)\
  (&(dc) < GBAddr ? ((Avd_Dict_Control_t *)HALTP)->cslgzrec_b : (dc).cslgzrec_b)

#define Avd_DictCtrl_CslgKeyZR_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).cslgzrec_b = (val); }

#define Avd_DictCtrl_CurSave_Get(dc)\
  (&(dc) < GBAddr ? HALT : (dc).cursave)

#define Avd_DictCtrl_CurSave_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).cursave = (val); }

#define Avd_DictCtrl_Depth_Get(dc)\
  (&(dc) < GBAddr ? HALT : (dc).depth)

#define Avd_DictCtrl_Depth_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).depth = (val); }

#define Avd_DictCtrl_ExtPatPtr_Get(dc)\
  (&(dc) < GBAddr ? HALTP : (dc).extpat_p)

#define Avd_DictCtrl_ExtPatPtr_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).extpat_p = (val); }

#define Avd_DictCtrl_ExtPatt_Get(dc)\
  (&(dc) < GBAddr ? HALTP : (dc).extract_patt)

#define Avd_DictCtrl_ExtPatt_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).extract_patt = (val); }

#define Avd_DictCtrl_InfoFC_Get(dc)\
  (&(dc) < GBAddr ? ((Avd_Dict_Control_t *)HALTP)->info_b : (dc).info_b)

#define Avd_DictCtrl_InfoFC_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).info_b = (val); }

#define Avd_DictCtrl_InfoZR_Get(dc)\
  (&(dc) < GBAddr ? HALT : (dc).infozrec_b)

#define Avd_DictCtrl_InfoZR_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).infozrec_b = (val); }

#define Avd_DictCtrl_PatTblFC_Get(dc)\
  (&(dc) < GBAddr ? ((Avd_Dict_Control_t *)HALTP)->pattbl_b : (dc).pattbl_b)

#define Avd_DictCtrl_PatTblFC_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).pattbl_b = (val); }

#define Avd_DictCtrl_PatTblZR_Get(dc)\
  (&(dc) < GBAddr ? ((Avd_Dict_Control_t *)HALTP)->pattzrec_b : (dc).pattzrec_b)

#define Avd_DictCtrl_PatTblZR_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).pattzrec_b = (val); }

#define Avd_DictCtrl_TblTops_Get(dc)\
  (&(dc) < GBAddr ? HALTP : (dc).tbltops)

#define Avd_DictCtrl_TblTops_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).tbltops = (val); }

#define Avd_DictCtrl_TopPref_Get(dc)\
  (&(dc) < GBAddr ? HALTP : (dc).topprefix_p)

#define Avd_DictCtrl_TopPref_Put(dc, val)\
  { if (&(dc) < GBAddr) HALT; else (dc).topprefix_p = (val); }
#endif

/** End of Field Access Macros for Avd_Dict_Control_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

/*** Global Variables ***/

/* End of AVLDICT.H */
#endif
