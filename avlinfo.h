#ifndef _H_AVLINFO_
#define _H_AVLINFO_ 1
/******************************************************************************
*
*  Copyright (C) 1993, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     AVLINFO.H
*
*    This module contains the definions for the abstraction of the 
*    Available Compounds Information Data Structures.  
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
* 25-Feb-95  Krebsbach  Added string lengths into info datastructure.
*
******************************************************************************/

/*** Literal Values ***/

#define  AVI_AVAIL_DFLT_CD       (U8_t) 0
#define  AVI_AVAIL_DFLT_STR      "default"       /* Default--no meaning */
#define  AVI_AVAIL_DIST_CD       (U8_t) 1
#define  AVI_AVAIL_DIST_STR      "distributor"   /* Chemical distributor */
#define  AVI_AVAIL_PROP_CD       (U8_t) 2
#define  AVI_AVAIL_PROP_STR      "proprietary"   /* Proprietary compound */
#define  AVI_AVAIL_SHELF_CD      (U8_t) 3
#define  AVI_AVAIL_SHELF_STR     "shelf"         /* Commonly kept material */
#define  AVI_AVAIL_SOLV_CD       (U8_t) 4
#define  AVI_AVAIL_SOLV_STR      "solution"      /* Solved by SYNCHEM */
#define  AVI_AVAIL_JUNK_CD       (U8_t) 255

#define  AVI_INFOREC_LENMAX       1024

#define  AVI_UNUSED_DFLT_CD       (U16_t) 0

#define  AVI_DATAFILE_ISAMINFO    "/avlcomp.isam_info"  
#define  AVI_VERSION_ISAMINFO     (U32_t) 2

/*** Data Structures ***/

/*  The information associated with each compound.  This is the
    structure that is created when the data is extracted from 
    a record in the runtime avlcomp information (Isam) file.
*/

typedef struct avi_cmp_info_s
  {
  char           *cat;                        /* catalog and number */
  char           *name;                       /* common name */
  char           *sling;                      /* sling */
  U16_t           cat_len;                    /* length of cat string  */
  U16_t           name_len;                   /* length of name string  */
  U16_t           sling_len;                  /* length of sling string  */
  U8_t            avail;                      /* availability of compound */
  U8_t            lib;                        /* From which library */
  } Avi_CmpInfo_t;
#define AVI_CMPINFO_SZ sizeof (Avi_CmpInfo_t)

/** Field Access Macros for Avi_CmpInfo_t **/

/* Macro Prototypes
   U8_t        Avi_CmpInfo_Avail_Get    (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_Avail_Put    (Avi_CmpInfo_t *, U8_t);
   char       *Avi_CmpInfo_Catalog_Get  (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_Catalog_Put  (Avi_CmpInfo_t *, char *);
   U16_t       Avi_CmpInfo_CatLen_Get   (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_CatLen_Put   (Avi_CmpInfo_t *, U16_t);
   U8_t        Avi_CmpInfo_Lib_Get      (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_Lib_Put      (Avi_CmpInfo_t *, U8_t);
   char       *Avi_CmpInfo_Name_Get     (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_Name_Put     (Avi_CmpInfo_t *, char *);
   U16_t       Avi_CmpInfo_NameLen_Get  (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_NameLen_Put  (Avi_CmpInfo_t *, U16_t);
   char       *Avi_CmpInfo_Sling_Get    (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_Sling_Put    (Avi_CmpInfo_t *, char *);
   U16_t       Avi_CmpInfo_SlgLen_Get   (Avi_CmpInfo_t *);
   void        Avi_CmpInfo_SlgLen_Put   (Avi_CmpInfo_t *, U16_t);
*/

#ifndef AVL_DEBUG
#define Avi_CmpInfo_Avail_Get(ci_p)\
  (ci_p)->avail

#define Avi_CmpInfo_Avail_Put(ci_p, val)\
  (ci_p)->avail = (val)

#define Avi_CmpInfo_Catalog_Get(ci_p)\
  (ci_p)->cat

#define Avi_CmpInfo_Catalog_Put(ci_p, val)\
  (ci_p)->cat = (val)

#define Avi_CmpInfo_CatLen_Get(ci_p)\
  (ci_p)->cat_len

#define Avi_CmpInfo_CatLen_Put(ci_p, val)\
  (ci_p)->cat_len = (val)

#define Avi_CmpInfo_Lib_Get(ci_p)\
  (ci_p)->lib

#define Avi_CmpInfo_Lib_Put(ci_p, val)\
  (ci_p)->lib = (val)

#define Avi_CmpInfo_Name_Get(ci_p)\
  (ci_p)->name

#define Avi_CmpInfo_Name_Put(ci_p, val)\
  (ci_p)->name = (val)

#define Avi_CmpInfo_NameLen_Get(ci_p)\
  (ci_p)->name_len

#define Avi_CmpInfo_NameLen_Put(ci_p, val)\
  (ci_p)->name_len = (val)

#define Avi_CmpInfo_Sling_Get(ci_p)\
  (ci_p)->sling

#define Avi_CmpInfo_Sling_Put(ci_p, val)\
  (ci_p)->sling = (val)

#define Avi_CmpInfo_SlgLen_Get(ci_p)\
  (ci_p)->sling_len

#define Avi_CmpInfo_SlgLen_Put(ci_p, val)\
  (ci_p)->sling_len = (val)

#else
#define Avi_CmpInfo_Avail_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->avail)

#define Avi_CmpInfo_Avail_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->avail = (val); }

#define Avi_CmpInfo_Catalog_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->cat)

#define Avi_CmpInfo_Catalog_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->cat = (val); }

#define Avi_CmpInfo_CatLen_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->cat_len)

#define Avi_CmpInfo_CatLen_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->cat_len = (val); }

#define Avi_CmpInfo_Lib_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->lib)

#define Avi_CmpInfo_Lib_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->lib = (val); }

#define Avi_CmpInfo_Name_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->name)

#define Avi_CmpInfo_Name_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->name = (val); }

#define Avi_CmpInfo_NameLen_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->name_len)

#define Avi_CmpInfo_NameLen_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->name_len = (val); }

#define Avi_CmpInfo_Sling_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->sling)

#define Avi_CmpInfo_Sling_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->sling = (val); }

#define Avi_CmpInfo_SlgLen_Get(ci_p)\
  ((ci_p) < GBAddr ? HALT : (ci_p)->sling_len)

#define Avi_CmpInfo_SlgLen_Put(ci_p, val)\
  { if ((ci_p) < GBAddr) HALT; else (ci_p)->sling_len = (val); }
#endif

/** End of  Field Access Macros for Avi_CmpInfo_t **/

/*  The header of the Isam information record.   It stores some of the 
    information, and other data necessary to extract the rest from the
    record.
*/

typedef struct avi_cmp_hdr_s
  {
  U16_t           cat_len;                    /* length of catalog */
  U16_t           name_len;                   /* length of common name */
  U16_t           slg_len;                    /* length of sling */
  U8_t            avail;                      /* availability of compound */
  U8_t            lib;                        /* From which library */
  } Avi_CmpHdr_t;
#define AVI_CMPHDR_SZ sizeof (Avi_CmpHdr_t)

/** Field Access Macros for Avi_CmpHdr_t **/

/* Macro Prototypes
   U8_t        Avi_CmpHdr_Avail_Get     (Avi_CmpHdr_t *);
   void        Avi_CmpHdr_Avail_Put     (Avi_CmpHdr_t *, U8_t);
   U16_t       Avi_CmpHdr_CatLen_Get    (Avi_CmpHdr_t *);
   void        Avi_CmpHdr_CatLen_Put    (Avi_CmpHdr_t *, U16_t);
   U8_t        Avi_CmpHdr_Lib_Get       (Avi_CmpHdr_t *);
   void        Avi_CmpHdr_Lib_Put       (Avi_CmpHdr_t *, U8_t);
   U16_t       Avi_CmpHdr_NameLen_Get   (Avi_CmpHdr_t *);
   void        Avi_CmpHdr_NameLen_Put   (Avi_CmpHdr_t *, U16_t);
   U16_t       Avi_CmpHdr_SlgLen_Get    (Avi_CmpHdr_t *);
   void        Avi_CmpHdr_SlgLen_Put    (Avi_CmpHdr_t *, U16_t);
*/

#ifndef AVL_DEBUG
#define Avi_CmpHdr_Avail_Get(ch_p)\
  (ch_p)->avail

#define Avi_CmpHdr_Avail_Put(ch_p, val)\
  (ch_p)->avail = (val)

#define Avi_CmpHdr_CatLen_Get(ch_p)\
  (ch_p)->cat_len

#define Avi_CmpHdr_CatLen_Put(ch_p, val)\
  (ch_p)->cat_len = (val)

#define Avi_CmpHdr_Lib_Get(ch_p)\
  (ch_p)->lib

#define Avi_CmpHdr_Lib_Put(ch_p, val)\
  (ch_p)->lib = (val)

#define Avi_CmpHdr_NameLen_Get(ch_p)\
  (ch_p)->name_len

#define Avi_CmpHdr_NameLen_Put(ch_p, val)\
  (ch_p)->name_len = (val)

#define Avi_CmpHdr_SlgLen_Get(ch_p)\
  (ch_p)->slg_len

#define Avi_CmpHdr_SlgLen_Put(ch_p, val)\
  (ch_p)->slg_len = (val)

#else
#define Avi_CmpHdr_Avail_Get(ch_p)\
  ((ch_p) < GBAddr ? HALT : (ch_p)->avail)

#define Avi_CmpHdr_Avail_Put(ch_p, val)\
  { if ((ch_p) < GBAddr) HALT; else (ch_p)->avail = (val); }

#define Avi_CmpHdr_CatLen_Get(ch_p)\
  ((ch_p) < GBAddr ? HALT : (ch_p)->cat_len)

#define Avi_CmpHdr_CatLen_Put(ch_p, val)\
  { if ((ch_p) < GBAddr) HALT; else (ch_p)->cat_len = (val); }

#define Avi_CmpHdr_Lib_Get(ch_p)\
  ((ch_p) < GBAddr ? HALT : (ch_p)->lib)

#define Avi_CmpHdr_Lib_Put(ch_p, val)\
  { if ((ch_p) < GBAddr) HALT; else (ch_p)->lib = (val); }

#define Avi_CmpHdr_NameLen_Get(ch_p)\
  ((ch_p) < GBAddr ? HALT : (ch_p)->name_len)

#define Avi_CmpHdr_NameLen_Put(ch_p, val)\
  { if ((ch_p) < GBAddr) HALT; else (ch_p)->name_len = (val); }

#define Avi_CmpHdr_SlgLen_Get(ch_p)\
  ((ch_p) < GBAddr ? HALT : (ch_p)->slg_len)

#define Avi_CmpHdr_SlgLen_Put(ch_p, val)\
  { if ((ch_p) < GBAddr) HALT; else (ch_p)->slg_len = (val); }
#endif

/** End of Field Access Macros for Avi_CmpHdr_t **/

/*** Macros ***/

/*** Routine Prototypes ***/

void            AviCmpInfo_Destroy (Avi_CmpInfo_t *);
Avi_CmpInfo_t  *AviCmpInfo_Extract (U8_t *);
U32_t           AviCmpInfo_Extrude (Avi_CmpInfo_t *, U8_t *);

/*** Global Variables ***/

/* End of AVLINFO.H */
#endif

