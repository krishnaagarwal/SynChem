#ifndef _H_SGLSHOT_
#define _H_SGLSHOT_ 1
/******************************************************************************
*
*  Copyright (C) 1995-1996, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SGLSHOT.H
*
*    This header contains the constants and data structures shared
*    by the different modules of GUI for SYNCHEM.
*
*  Data Structures:
*
*    SShotGenSGs_t
*    SShotInfo_t
*
*  Creation Date:
*
*    23-Aug-1995
*
*  Authors:
*
*    Daren Krebsbach
*    Shu Cheung
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xxx
*
******************************************************************************/

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/*** Literal Values ***/

#define SSV_GENSG_MAXNUM       255 /* must fit into U8_t - reason unknown */

#define SSV_PTEST_NONE         0
#define SSV_PTEST_FALSE        1
#define SSV_PTEST_FAIL         2
#define SSV_PTEST_PASS         3

/*** Data Structures ***/

typedef struct sshot_info_s
  {
  Sling_t       *slings;             /* Array of precursor slings */
  U8_t          *test_results;       /* Posttran tests results */ 
  U8_t           num_mols;           /* Number of precursor mols */ 
  U8_t           num_tests;          /* Number of tests */ 
  Boolean_t      all_post;           /* Passed all posttran tests? */
  Boolean_t      comp_ok;            /* Passed comp_ok? */
  Boolean_t      max_nonid;          /* Passed max nonident? */
  Boolean_t      mrt_update;         /* Passed merit update? */
  Boolean_t      num_apply;          /* Passed num of applications? */
  U32_t        **target_maps;
  U16_t          tmaprow_count;
  U16_t          tmapcol_count;
  S16_t          merit_ea; /* ease adj. based on application */
  S16_t          merit_ya; /* yld adj. based on application */
  S16_t          merit_ca; /* conf adj. based on application */
  }  SShotInfo_t;
#define SSV_SSHOTINFO_SIZE  (sizeof (SShotInfo_t))

/* Macro Prototypes

  Boolean_t       SShotInfo_AllPostT_Get  (SShotInfo_t *);
  void            SShotInfo_AllPostT_Put  (SShotInfo_t *, Boolean_t);
  Boolean_t       SShotInfo_CompOk_Get    (SShotInfo_t *);
  void            SShotInfo_CompOk_Put    (SShotInfo_t *, Boolean_t);
  Sling_t         SShotInfo_IthSling_Get  (SShotInfo_t *, U8_t);
  void            SShotInfo_IthSling_Put  (SShotInfo_t *, U8_t, Sling_t);
  U8_t            SShotInfo_IthTRslt_Get  (SShotInfo_t *, U8_t);
  void            SShotInfo_IthTRslt_Put  (SShotInfo_t *, U8_t, U8_t);
  Boolean_t       SShotInfo_MaxNonid_Get  (SShotInfo_t *);
  void            SShotInfo_MaxNonid_Put  (SShotInfo_t *, Boolean_t);
  Boolean_t       SShotInfo_MrtUpdate_Get (SShotInfo_t *);
  void            SShotInfo_MrtUpdate_Put (SShotInfo_t *, Boolean_t);
  Boolean_t       SShotInfo_NumApply_Get  (SShotInfo_t *);
  void            SShotInfo_NumApply_Put  (SShotInfo_t *, Boolean_t);
  U8_t            SShotInfo_NumMols_Get   (SShotInfo_t *);
  void            SShotInfo_NumMols_Put   (SShotInfo_t *, U8_t);
  U8_t            SShotInfo_NumTests_Get  (SShotInfo_t *);
  void            SShotInfo_NumTests_Put  (SShotInfo_t *, U8_t);
  Sling_t        *SShotInfo_Slings_Get    (SShotInfo_t *);
  void            SShotInfo_Slings_Put    (SShotInfo_t *, Sling_t *);
  U8_t           *SShotInfo_TResults_Get  (SShotInfo_t *);
  void            SShotInfo_TResults_Put  (SShotInfo_t *, U8_t *);
  U32_t         **SShotInfo_TargMaps_Get   (SShotInfo_t *);
  void            SShotInfo_TargMaps_Put   (SShotInfo_t *, U32_t **);
  U16_t           SShotInfo_TMapRowCnt_Get (SShotInfo_t *);
  void            SShotInfo_TMapRowCnt_Put (SShotInfo_t *, U16_t);
  U16_t           SShotInfo_TMapColCnt_Get (SShotInfo_t *);
  void            SShotInfo_TMapColCnt_Put (SShotInfo_t *, U16_t);
  U16             SShotInfo_TMapTargAtom   (U32_t **, U16_t, U16_t);
  Boolean_t       SShotInfo_TMapValid      (U32_t **, U16_t, U16_t);
  U16_t           SShotInfo_TMapSubgNum    (U32_t **, U16_t, U16_t);
  U16_t           SShotInfo_TMapPattAtom   (U32_t **, U16_t, U16_t);
  U16_t           SShotInfo_TMapSubgAtom   (U32_t **, U16_t, U16_t);
  S16_t           SShotInfo_MeritEA_Get   (SShotInfo_t *);
  void            SShotInfo_MeritEA_Put   (SShotInfo_t *, S16_t);
  S16_t           SShotInfo_MeritYA_Get   (SShotInfo_t *);
  void            SShotInfo_MeritYA_Put   (SShotInfo_t *, S16_t);
  S16_t           SShotInfo_MeritCA_Get   (SShotInfo_t *);
  void            SShotInfo_MeritCA_Put   (SShotInfo_t *, S16_t);
*/

#define SShotInfo_AllPostT_Get(sip)\
  (sip)->all_post
#define SShotInfo_AllPostT_Put(sip, value)\
  (sip)->all_post = (value)
#define SShotInfo_CompOk_Get(sip)\
  (sip)->comp_ok
#define SShotInfo_CompOk_Put(sip, value)\
  (sip)->comp_ok = (value)
#define SShotInfo_IthSling_Get(sip, ith)\
  (sip)->slings[ith]
#define SShotInfo_IthSling_Put(sip, ith, value)\
  (sip)->slings[ith] = (value)
#define SShotInfo_IthTRslt_Get(sip, ith)\
  (sip)->test_results[ith]
#define SShotInfo_IthTRslt_Put(sip, ith, value)\
  (sip)->test_results[ith] = (value)
#define SShotInfo_MaxNonid_Get(sip)\
  (sip)->max_nonid
#define SShotInfo_MaxNonid_Put(sip, value)\
  (sip)->max_nonid = (value)
#define SShotInfo_MrtUpdate_Get(sip)\
  (sip)->mrt_update
#define SShotInfo_MrtUpdate_Put(sip, value)\
  (sip)->mrt_update = (value)
#define SShotInfo_NumApply_Get(sip)\
  (sip)->num_apply
#define SShotInfo_NumApply_Put(sip, value)\
  (sip)->num_apply = (value)
#define SShotInfo_NumMols_Get(sip)\
  (sip)->num_mols
#define SShotInfo_NumMols_Put(sip, value)\
  (sip)->num_mols = (value)
#define SShotInfo_NumTests_Get(sip)\
  (sip)->num_tests
#define SShotInfo_NumTests_Put(sip, value)\
  (sip)->num_tests = (value)
#define SShotInfo_Slings_Get(sip)\
  (sip)->slings
#define SShotInfo_Slings_Put(sip, value)\
  (sip)->slings = (value)
#define SShotInfo_TResults_Get(sip)\
  (sip)->test_results
#define SShotInfo_TResults_Put(sip, value)\
  (sip)->test_results = (value)
#define SShotInfo_TargMaps_Get(sip)\
  (sip)->target_maps
#define SShotInfo_TargMaps_Put(sip, value)\
  (sip)->target_maps = (value)
#define SShotInfo_TMapRowCnt_Get(sip)\
  (sip)->tmaprow_count
#define SShotInfo_TMapRowCnt_Put(sip, value)\
  (sip)->tmaprow_count = (value)
#define SShotInfo_TMapColCnt_Get(sip)\
  (sip)->tmapcol_count
#define SShotInfo_TMapColCnt_Put(sip, value)\
  (sip)->tmapcol_count = (value)
#define SShotInfo_TMapTargAtom(tmap, row, col)\
  ((tmap)[(row)][(col)] & 0xff)
#define SShotInfo_TMapValid(tmap, row, col)\
  ((tmap)[(row)][(col)] >> 16 != XTR_INVALID)
#define SShotInfo_TMapSubgNum(tmap, row, col)\
  (((tmap)[(row)][(col)] >>8) & 0xff)
#define SShotInfo_TMapPattAtom(tmap, row, col)\
  (((tmap)[(row)][(col)] >> 16) & 0xff)
#define SShotInfo_TMapSubgAtom(tmap, row, col)\
  ((tmap)[(row)][(col)] >> 24)
#define SShotInfo_MeritEA_Get(sip)\
  (sip)->merit_ea
#define SShotInfo_MeritEA_Put(sip, value)\
  (sip)->merit_ea = (value)
#define SShotInfo_MeritYA_Get(sip)\
  (sip)->merit_ya
#define SShotInfo_MeritYA_Put(sip, value)\
  (sip)->merit_ya = (value)
#define SShotInfo_MeritCA_Get(sip)\
  (sip)->merit_ca
#define SShotInfo_MeritCA_Put(sip, value)\
  (sip)->merit_ca = (value)

typedef struct sshot_gensgs_s
  {
  SShotInfo_t   *results[SSV_GENSG_MAXNUM];
  U8_t           num_results;        /* Number of generated subgoals */ 
  Boolean_t      pretran;            /* Passed pretransform tests? */
  char           pretran_buffer[1024];
  }  SShotGenSGs_t;
#define SSV_SSHOTGENSGS_SIZE  (sizeof (SShotGenSGs_t))

/* Macro Prototypes

  SShotInfo_t **SShotGenSG_Infos_Get      (SShotGenSGs_t *);
  SShotInfo_t  *SShotGenSG_IthInfo_Get    (SShotGenSGs_t *, U8_t);
  void          SShotGenSG_IthInfo_Put    (SShotGenSGs_t *, U8_t, SShotInfo_t *);
  U8_t          SShotGenSG_NumInfos_Get   (SShotGenSGs_t *);
  void          SShotGenSG_NumInfos_Put   (SShotGenSGs_t *, U8_t);
  Boolean_t     SShotGenSG_PreTrans_Get   (SShotGenSGs_t *);
  void          SShotGenSG_PreTrans_Put   (SShotGenSGs_t *, Boolean_t);
  char         *SShotGenSG_PreTBuff_Get   (SShotGenSGs_t *);
*/

#define SShotGenSG_Infos_Get(sgp)\
  (sgp)->results
#define SShotGenSG_IthInfo_Get(sgp, ith)\
  (sgp)->results[ith]
#define SShotGenSG_IthInfo_Put(sgp, ith, value)\
  (sgp)->results[ith] = (value)
#define SShotGenSG_NumInfos_Get(sgp)\
  (sgp)->num_results
#define SShotGenSG_NumInfos_Put(sgp, value)\
  (sgp)->num_results = (value)
#define SShotGenSG_PreTrans_Get(sgp)\
  (sgp)->pretran
#define SShotGenSG_PreTrans_Put(sgp, value)\
  (sgp)->pretran = (value)
#define SShotGenSG_PreTBuff_Get(sgp)\
  (sgp)->pretran_buffer

/*** Routine Prototypes ***/

void         SingleShot_Apply  (SShotGenSGs_t *, Sling_t, U32_t, Boolean_t,
               Boolean_t);
SShotInfo_t *SShotInfo_Create  (U8_t);
void         SShotInfo_Destroy (SShotInfo_t *);
void         SShotInfo_Slings_Save (SShotInfo_t *, Sling_t *, U16_t);
void         SingleShot_Dups_Prune (SShotGenSGs_t *);
void         SingleShot_CompSling_Create (SShotInfo_t *, String_t *);

#ifdef _GLOBAL_DEF_
Boolean_t sshot_embedded = FALSE;
#else
extern Boolean_t sshot_embedded;
#endif

#endif
/*  End of SGLSHOT.H  */
