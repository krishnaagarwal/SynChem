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
  char          search_sling[512],*bar;
  Sling_t       sling_st;
  Xtr_t         *xtr;
  Boolean_t     found;
  char          ans[16];
  U16_t         rec_len;
  int           status;

IO_Init();
#ifdef _CYGWIN_
IO_Init_Files ("NUL:", FALSE);
#else
IO_Init_Files ("/dev/null", FALSE);
#endif
/*
IO_FileHandle_Put (&GTraceFile,stderr);
Debug_Init ();
GTrace[DB_ISAMWRITE].params=TL_ALWAYS;
Debug_Init ();
Trace_Init ();
IO_Init ();
IO_Init_Files ("/dev/null", FALSE);
*/

  search_sling[0]=0;
  if (argc==2)
  {
    Sling_Name_Put (sling_st, argv[1]);
    Sling_Length_Put (sling_st, strlen(argv[1]));
    xtr = Sling2Xtr_PlusHydrogen (sling_st);
    Sling_Destroy (sling_st);
    Xtr_Attr_ResonantBonds_Set (xtr);
    Xtr_Name_Set (xtr, NULL);
    sling_st = Name_Sling_Get (xtr, TRUE);
    Xtr_Destroy(xtr);
    strncpy(search_sling,Sling_Name_Get(sling_st),Sling_Length_Get(sling_st));
    search_sling[Sling_Length_Get(sling_st)]=0;
    Sling_Destroy (sling_st);
    bar=strstr(search_sling,"|");
    if (bar!=NULL) *bar=0;
printf("%s\n",search_sling);
  }

  /*  Parse command line and open files. */

  dp_p = "//D/SYNCHEM/testdata/tstavl/";
  dir_len = strlen (dp_p);
  fn_p = IO_FileName_Get (Isam_File_Get (&Avd_DictCtrl_InfoFC_Get
    (SDictControl)));
  strncpy (fn_p, dp_p, dir_len);
  strncpy (fn_p + dir_len, AVI_DATAFILE_ISAMINFO, 
	   MX_FILENAME - (dir_len + 1));
  Isam_Open (&Avd_DictCtrl_InfoFC_Get (SDictControl), ISAM_TYPE_AVLCOMP, 
    ISAM_OPEN_READ);
printf("dataend=%ld 0%lo 0x%lx\nnextkey=%ld\nmax=%d\ncurpos=%d\n",Isam_Dataend_Get(&Avd_DictCtrl_InfoFC_Get (SDictControl)),Isam_Dataend_Get(&Avd_DictCtrl_InfoFC_Get (SDictControl)),Isam_Dataend_Get(&Avd_DictCtrl_InfoFC_Get (SDictControl)),Isam_NextKey_Get(&Avd_DictCtrl_InfoFC_Get (SDictControl)),Isam_MaxRecKey_Get(&Avd_DictCtrl_InfoFC_Get (SDictControl)),Isam_Curpos_Get(&Avd_DictCtrl_InfoFC_Get (SDictControl)));

  /* Read in the number of slings. */

  num_slings = SNSlings_Get ();
  fprintf (stdout, "avldump:  total number of slings:  %1lu.\n", num_slings);

  for (i=1, found=FALSE; i<=num_slings; i++)
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
    if (search_sling[0]==0 || strcmp(sling,search_sling)==0)
    {
      printf("%d: %s %s %s\n",i,cat,name,sling);
      found=TRUE;
    }
  }
  Isam_Close (&Avd_DictCtrl_InfoFC_Get (SDictControl));

  if (!found)
  {
  Isam_Open (&Avd_DictCtrl_InfoFC_Get (SDictControl), ISAM_TYPE_AVLCOMP, 
    ISAM_OPEN_WRITE);
    printf("Not found - do you want to enter info? ");
    fgets(ans,15,stdin);
    if (ans[0]=='y' || ans[0]=='Y')
    {
      printf("Name: ");
      fgets(name,255,stdin);
      name[strlen(name)-1]=0;
      printf("Cat#: ");
      fgets(cat,63,stdin);
      cat[strlen(cat)-1]=0;
      Avi_CmpInfo_Avail_Put (avinfo_p, AVI_AVAIL_DFLT_CD);
      Avi_CmpInfo_Catalog_Put (avinfo_p, cat);
      Avi_CmpInfo_Lib_Put (avinfo_p, 1);
      Avi_CmpInfo_Name_Put (avinfo_p, name);
      Avi_CmpInfo_Sling_Put (avinfo_p, search_sling);
      Avi_CmpInfo_CatLen_Put (avinfo_p, strlen(cat));
      Avi_CmpInfo_NameLen_Put (avinfo_p, strlen(name));
      Avi_CmpInfo_SlgLen_Put (avinfo_p, strlen(search_sling));
      rec_len = AviCmpInfo_Extrude (avinfo_p, buff);
printf("%d: ",rec_len);
for (j=0; j<rec_len; j++) if (buff[j]>=' ' && buff[j]<='~') putchar(buff[j]);
else printf("[%d]",buff[j]);
putchar('\n');
/*
fflush(IO_FileHandle_Get (Isam_File_Get (&Avd_DictCtrl_InfoFC_Get (SDictControl))));
*/
/*
      Isam_Flush (&Avd_DictCtrl_InfoFC_Get (SDictControl));
*/
      status=Isam_Write (&Avd_DictCtrl_InfoFC_Get (SDictControl), i, buff, rec_len);
if (status) printf("record not written: %d\n",status);
printf("before flush\n");
      Isam_Flush (&Avd_DictCtrl_InfoFC_Get (SDictControl));

printf("before rec zero\n");
      Avc_RecZero_CntSz_Put (Avd_DictCtrl_InfoZR_Get (SDictControl), i);
printf("after rec zero\n");
      status=Isam_Write (&Avd_DictCtrl_InfoFC_Get (SDictControl), 0, &Avd_DictCtrl_InfoZR_Get (SDictControl), AVC_RECZERO_SZ);
if (status) printf("header not written: %d\n",status);
      Isam_Flush (&Avd_DictCtrl_InfoFC_Get (SDictControl));
    }
  }
  Isam_Close (&Avd_DictCtrl_InfoFC_Get (SDictControl));
  exit (0);

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
