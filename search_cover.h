#ifndef _H_SEARCH_COVER_
#define _H_SEARCH_COVER_ 1
/******************************************************************************
*
*  Copyright (C) 1997 by Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SEARCH_COVER.H
*
*    This module contains all the types used for task execution.
*
*  Creation Date:
*
*    13-Nov-96
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xxx
*
******************************************************************************/

#ifndef _H_SLING_
#include "sling.h"
#endif

/*** Literal Values ***/

#define COVER_NOT_FOUND     (S32_t)-1

#define COVER_NOT_COVERED     (U8_t) 0
#define COVER_DEV_COVERED     (U8_t) 1
#define COVER_SOL_COVERED     (U8_t) 2
#define COVER_BOTH_COVERED    (U8_t) (COVER_DEV_COVERED | COVER_SOL_COVERED)

/*** Data Structures ***/

typedef struct cover_entry_s
  {
  Sling_t               sling;
  Boolean_t             is_covered;
  Boolean_t             was_solved;
  } CoverEntry_t;
#define COVERENTRY_SIZE sizeof(CoverEntry_t)
 
 
/*Macro Prototypes

Boolean_t       CoverEntry_IsCovered_Get  (CoverEntry_t);
void            CoverEntry_IsCovered_Put  (CoverEntry_t, Boolean_t);
Sling_t         CoverEntry_Sling_Get      (CoverEntry_t);
void            CoverEntry_Sling_Put      (CoverEntry_t, Sling_t);
Boolean_t       CoverEntry_WasSolved_Get  (CoverEntry_t);
void            CoverEntry_WasSolved_Put  (CoverEntry_t, Boolean_t);
*/
 
#define CoverEntry_IsCovered_Get(ce)\
  (ce).is_covered
#define CoverEntry_IsCovered_Put(ce, val)\
  (ce).is_covered = (val)
 
#define CoverEntry_Sling_Get(ce)\
  (ce).sling
#define CoverEntry_Sling_Put(ce, val)\
  (ce).sling = (val)

#define CoverEntry_WasSolved_Get(ce)\
  (ce).was_solved
#define CoverEntry_WasSolved_Put(ce, val)\
  (ce).was_solved = (val)



typedef struct cover_table_s
  {
  CoverEntry_t  *table;
  S32_t          upbound;
  S32_t          num_dev;
  S32_t          num_slv;
  S32_t		 num_dev_cover;
  S32_t		 num_slv_cover;
  } CoverTbl_t; 
#define COVERTBL_SIZE sizeof(CoverTbl_t)

/*Macro Prototypes

S32_t           CoverTbl_NumCoverDev_Get  (CoverTbl_t *);
void            CoverTbl_NumCoverDev_Put  (CoverTbl_t *, S32_t);
S32_t           CoverTbl_NumCoverSlv_Get  (CoverTbl_t *);
void            CoverTbl_NumCoverSlv_Put  (CoverTbl_t *, S32_t);
S32_t           CoverTbl_NumDev_Get       (CoverTbl_t *);
void            CoverTbl_NumDev_Put       (CoverTbl_t *, S32_t);
S32_t           CoverTbl_NumSlv_Get       (CoverTbl_t *);
void            CoverTbl_NumSlv_Put       (CoverTbl_t *, S32_t);
CoverEntry_t   *CoverTbl_Table_Get        (CoverTbl_t *);
void            CoverTbl_Table_Put        (CoverTbl_t *, CoverEntry_t *);
S32_t           CoverTbl_UpperBound_Get   (CoverTbl_t *);
void            CoverTbl_UpperBound_Put   (CoverTbl_t *, S32_t);

*/

#define CoverTbl_NumCoverDev_Get(ct_p)\
  (ct_p)->num_dev_cover
#define CoverTbl_NumCoverDev_Put(ct_p, val)\
  (ct_p)->num_dev_cover = (val)
#define CoverTbl_NumCoverSlv_Get(ct_p)\
  (ct_p)->num_slv_cover
#define CoverTbl_NumCoverSlv_Put(ct_p, val)\
  (ct_p)->num_slv_cover = (val)
#define CoverTbl_NumDev_Get(ct_p)\
  (ct_p)->num_dev
#define CoverTbl_NumDev_Put(ct_p, val)\
  (ct_p)->num_dev = (val)
#define CoverTbl_NumSlv_Get(ct_p)\
  (ct_p)->num_slv
#define CoverTbl_NumSlv_Put(ct_p, val)\
  (ct_p)->num_slv = (val)
#define CoverTbl_Table_Get(ct_p)\
  (ct_p)->table
#define CoverTbl_Table_Put(ct_p, val)\
  (ct_p)->table = (val)
#define CoverTbl_UpperBound_Get(ct_p)\
  (ct_p)->upbound
#define CoverTbl_UpperBound_Put(ct_p, val)\
  (ct_p)->upbound = (val)

/* Macro Prototypes

CoverEntry_t    CoverTbl_IthCover_Get      (CoverTbl_t *, S32_t);
void            CoverTbl_IthCover_Put      (CoverTbl_t *, S32_t, CoverEntry_t);
Boolean_t       CoverTbl_IthIsCovered_Get  (CoverTbl_t *, S32_t);
void            CoverTbl_IthIsCovered_Put  (CoverTbl_t *, S32_t, Boolean_t);
Sling_t         CoverTbl_IthSling_Get      (CoverTbl_t *, S32_t);
void            CoverTbl_IthSling_Put      (CoverTbl_t *, S32_t, Sling_t);
Boolean_t       CoverTbl_IthWasSolved_Get  (CoverTbl_t *, S32_t);
void            CoverTbl_IthWasSolved_Put  (CoverTbl_t *, S32_t, Boolean_t);
*/

#define CoverTbl_IthCover_Get(ct_p, ith)\
  (ct_p)->table[ith]
#define CoverTbl_IthCover_Put(ct_p, ith, val)\
  (ct_p)->table[ith] = (val)

#define CoverTbl_IthIsCovered_Get(ct_p, ith)\
  CoverEntry_IsCovered_Get ((ct_p)->table[ith])
#define CoverTbl_IthIsCovered_Put(ct_p, ith, val)\
  CoverEntry_IsCovered_Put ((ct_p)->table[ith], (val))

#define CoverTbl_IthSling_Get(ct_p, ith)\
  CoverEntry_Sling_Get ((ct_p)->table[ith])
#define CoverTbl_IthSling_Put(ct_p, ith, val)\
  CoverEntry_Sling_Put ((ct_p)->table[ith], (val))

#define CoverTbl_IthWasSolved_Get(ct_p, ith)\
  CoverEntry_WasSolved_Get ((ct_p)->table[ith])
#define CoverTbl_IthWasSolved_Put(ct_p, ith, val)\
  CoverEntry_WasSolved_Put ((ct_p)->table[ith], (val))

/* Routine Prototypes */

void       SearchCoverTable_Load   (char *, U16_t);
U8_t       SearchSpace_Cover       (Sling_t);

#endif

/* End of SEARCH_COVER.H */
