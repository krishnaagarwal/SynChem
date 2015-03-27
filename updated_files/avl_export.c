/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     AVL_EXPORT.C
*
*    This utility creates an ASCII dump of the available compounds library for
*    use with the AVL_IMPORT utility in constructing a replica of the library.
*
*  Creation Date:
*
*    11-Oct-2000
*
*  Authors:
*
*    Gerald A. Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

/*  Static Function Prototypes  */

static U32_t   SNSlings_Get          ();

void main
  (
  int argc,
  char *argv[]
  )
{
  char       *fn_p;                       /* temp filename ptr */
  char       *dp_p;                       /* dirpath ptr */
  size_t      dir_len;                    /* length of dirpath */
  U32_t       i,j;                        /* index */
  U32_t       num_slings;                 /* number of slings */
  U8_t          buff[AVI_INFOREC_LENMAX];   /* comp info record buffer */
  Avi_CmpInfo_t *avinfo_p;
  char cat[64], name[256], sling[512];
  Sling_t       sling_st;
  Xtr_t         *xtr;
  U16_t         rec_len;
  int           status;
  U8_t          avl, lib;

  IO_Init();
#ifdef _CYGWIN_
  IO_Init_Files ("NUL:", FALSE);
#else
  IO_Init_Files ("/dev/null", FALSE);
#endif

  dp_p = "./data/avlcomp/";
  dir_len = strlen (dp_p);
  fn_p = IO_FileName_Get (Isam_File_Get (&Avd_DictCtrl_InfoFC_Get
    (SDictControl)));
  strncpy (fn_p, dp_p, dir_len);
  strncpy (fn_p + dir_len, AVI_DATAFILE_ISAMINFO, 
	   MX_FILENAME - (dir_len + 1));
  Isam_Open (&Avd_DictCtrl_InfoFC_Get (SDictControl), ISAM_TYPE_AVLCOMP, 
    ISAM_OPEN_READ);

  /* Read in the number of slings. */

  num_slings = SNSlings_Get ();
  fprintf (stdout, "avl_export: %d slings\n", num_slings);

  for (i=1; i<=num_slings; i++)
  {
    Isam_Read_Nobuffer (&Avd_DictCtrl_InfoFC_Get (SDictControl), i, 
      buff, AVI_INFOREC_LENMAX);
    avinfo_p = AviCmpInfo_Extract(buff);
    strncpy (cat, Avi_CmpInfo_Catalog_Get (avinfo_p), Avi_CmpInfo_CatLen_Get (avinfo_p));
    cat[Avi_CmpInfo_CatLen_Get (avinfo_p)]=0;
    strncpy (name, Avi_CmpInfo_Name_Get (avinfo_p), Avi_CmpInfo_NameLen_Get (avinfo_p));
    name[Avi_CmpInfo_NameLen_Get (avinfo_p)]=0;
    strncpy (sling, Avi_CmpInfo_Sling_Get (avinfo_p), Avi_CmpInfo_SlgLen_Get (avinfo_p));
    sling[Avi_CmpInfo_SlgLen_Get (avinfo_p)]=0;
    avl=Avi_CmpInfo_Avail_Get (avinfo_p);
    lib=Avi_CmpInfo_Lib_Get (avinfo_p);
    printf("%d: %d %d %s\n\t%s\n\t%s\n",i,(int)avl,(int)lib,sling,name,cat);
  }
  Isam_Close (&Avd_DictCtrl_InfoFC_Get (SDictControl));
}  
/* End of main */

/****************************************************************************
*
*  Function Name:                 SNSlings_Get
*
*    Verifies the version number of the Isam sling info file.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - info_b
*
*  Implicit Outputs:
*
*    SDictControl:
*      - info_b
*      - infozrec_b
*
*  Return Values:
*
*    Number of slings read in.
*
*  Side Effects:
*
*    Allocates memory for the slings:  may exit on error.
*    Closes the Isam sling info file.
*
******************************************************************************/
U32_t SNSlings_Get
  (
  )
{

  U32_t           slg_count;                  /* number of slings in file */

  /*  Read in zero record of Isam sling info file.  Verify version
      number and get number of slings.  Read in the slings.
  */

  Isam_Read_Nobuffer (&Avd_DictCtrl_InfoFC_Get (SDictControl), 0, 
    &Avd_DictCtrl_InfoZR_Get (SDictControl), AVC_RECZERO_SZ);

  if (Avc_RecZero_VerNum_Get (Avd_DictCtrl_InfoZR_Get (SDictControl))
      != AVI_VERSION_ISAMINFO)
    {
    fprintf (stderr, "\nSSlings_Get:  incorrect Isam info file version.\n");
    exit (-1);
    }

  slg_count = Avc_RecZero_CntSz_Get (Avd_DictCtrl_InfoZR_Get (SDictControl));

  return slg_count;
} 
/* End of SNSlings_Get  */
