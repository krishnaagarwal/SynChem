/******************************************************************************
*
*  Copyright (C) 1995-1996 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SGLSHOT.C
*
*    This module contains the functions needed to execute SingleShot,
*    the application of subgoalgeneration on a reaction and a target 
*    compound in order to get the results of the matching and post-transform 
*    tests.
*
*
*  Routines:
*
*    SingleShot_Apply
*    SShotInfo_Create
*    SShotInfo_Destroy
*    SShotInfo_Slings_Save
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
* dd-mmm-yy  Fred       xxx
* 12-Sep-95  Cheung	Added routine List_Dump
* 02-Nov-99  Miller     Pruned duplicate subgoals
*
******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "synchem.h"
#include "synio.h"
#include "debug.h"

#ifndef _H_ARRAY_
#include "array.h"
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

#ifndef _H_SGLSHOT_
#define _GLOBAL_DEF_
#include "sglshot.h"
#undef _GLOBAL_DEF_
#endif

#ifndef _H_SUBGOALGENERATION_
#include "subgoalgeneration.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif


/****************************************************************************
*
*  Function Name:                 SingleShot_Apply
*
*    This routine stores the results of the pretransform and post-
*    transform tests when  subgoalgeneration is applied to the given
*    target compound and reaction.
*
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

void SingleShot_Apply
  (
  SShotGenSGs_t  *ssgensg_p, 
  Sling_t	 sling,
  U32_t          schema,
  Boolean_t      stereo,
  Boolean_t      preserve
  )
{
  U16_t          num_rings;
  U16_t          ring_idx;
  Array_t        strategic_bonds;
  Array_t        stereo_options;
  Xtr_t		 *xtr_p;

  xtr_p = Sling_CanonicalName2Xtr (sling);
  Xtr_Attr_ResonantBonds_Set (xtr_p);
  Xtr_Aromat_Set (xtr_p);
 
  num_rings = Xtr_Rings_NumRingSys_Get (xtr_p);
  for (ring_idx = 0; ring_idx < num_rings; ring_idx++)
    Xtr_Ringdef_Set (xtr_p, ring_idx);
 
  Xtr_Atoms_Set (xtr_p);
  Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));
 
  if (React_Schema_IsOk_Buffer (xtr_p, schema, SShotGenSG_PreTBuff_Get (ssgensg_p)) == TRUE)
    {
#ifdef _MIND_MEM_
    mind_Array_2d_Create ("&strategic_bonds", "sglshot{1}", &strategic_bonds, Xtr_NumAtoms_Get (xtr_p),
      MX_NEIGHBORS, BITSIZE);
    mind_Array_1d_Create ("&stereo_options", "sglshot{1}", &stereo_options, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
#else
    Array_2d_Create (&strategic_bonds, Xtr_NumAtoms_Get (xtr_p),
      MX_NEIGHBORS, BITSIZE);
    Array_1d_Create (&stereo_options, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
#endif
 
    Array_Set (&strategic_bonds, TRUE);
 
    Array_Set (&stereo_options, stereo);
 
    subgenr_interactive = TRUE;
    SubGenr_Subgoals_Generate (xtr_p, &strategic_bonds, 
      &stereo_options, schema, stereo, preserve, ssgensg_p);
    subgenr_interactive = FALSE;

    if (SShotGenSG_NumInfos_Get (ssgensg_p) > 0) sshot_embedded = TRUE;

#ifdef _DEBUG_
printf("before SS_D_P\n");
#endif
    SingleShot_Dups_Prune (ssgensg_p);
#ifdef _DEBUG_
printf("after SS_D_P\n");
#endif
 
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&strategic_bonds", "sglshot", &strategic_bonds);
    mind_Array_Destroy ("&stereo_options", "sglshot", &stereo_options);
#else
    Array_Destroy (&strategic_bonds);
    Array_Destroy (&stereo_options);
#endif
    }
  else
    SShotGenSG_PreTrans_Put (ssgensg_p, FALSE);

  Xtr_Destroy (xtr_p);
#ifdef _DEBUG_
printf("exiting SS_A\n");
#endif
}


/****************************************************************************
*
*  Function Name:                 SShotInfo_Slings_Save
*
*    This routine saves an array of copied slings for a particular
*    application of a reaction.
*
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
void SShotInfo_Slings_Save
  (
  SShotInfo_t  *sshotinfo_p,
  Sling_t      *comp_slings_p,              /* sorted slings */
  U16_t         num_compounds               /* # non-duplicate compounds */
  )

{
  Sling_t      *slings;
  U16_t         comp_i;                     /* Ith compound */


  if (num_compounds > 0)
    {
#ifdef _MIND_MEM_
    mind_malloc ("slings", "sglshot{2}", &slings, num_compounds * SLINGSIZE);
#else
    slings = (Sling_t *) malloc (num_compounds * SLINGSIZE);
#endif
    SShotInfo_Slings_Put (sshotinfo_p, slings);
    for (comp_i = 0; comp_i < num_compounds; comp_i++)
      {
      SShotInfo_IthSling_Put (sshotinfo_p, comp_i, 
        Sling_CopyTrunc (comp_slings_p[comp_i]));
      }
    }
  else 
    SShotInfo_Slings_Put (sshotinfo_p, NULL);

  SShotInfo_NumMols_Put (sshotinfo_p, num_compounds);
  return ;
}
/*  End of SShotInfo_Slings_Save  */



/****************************************************************************
*
*  Function Name:                 SShotInfo_Create
*
*    This routine creates and initializes a singleshot information
*    data structure.
*
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
SShotInfo_t *SShotInfo_Create
  (
  U8_t           num_tests
  )
{
  SShotInfo_t   *ssi_p;
  U8_t          *tests_p;
  U8_t           test_i;

#ifdef _MIND_MEM_
  mind_malloc ("ssi_p", "sglshot{3}", &ssi_p, SSV_SSHOTINFO_SIZE);
  mind_malloc ("tests_p", "sglshot{3}", &tests_p, num_tests * sizeof (U8_t));
#else
  ssi_p = (SShotInfo_t *) malloc (SSV_SSHOTINFO_SIZE);
  tests_p = (U8_t *) malloc (num_tests * sizeof (U8_t));
#endif

  SShotInfo_NumTests_Put (ssi_p, num_tests);
  SShotInfo_TResults_Put (ssi_p, tests_p);
  for (test_i = 0; test_i < num_tests; test_i++)
    SShotInfo_IthTRslt_Put (ssi_p, test_i, SSV_PTEST_NONE);

  SShotInfo_NumMols_Put (ssi_p, 0);
  SShotInfo_Slings_Put (ssi_p, NULL);
  SShotInfo_AllPostT_Put (ssi_p, TRUE);
  SShotInfo_CompOk_Put (ssi_p, TRUE);
  SShotInfo_MaxNonid_Put (ssi_p, TRUE);
  SShotInfo_MrtUpdate_Put (ssi_p, TRUE);
  SShotInfo_NumApply_Put (ssi_p, TRUE);
  SShotInfo_TargMaps_Put (ssi_p, NULL);
  SShotInfo_TMapRowCnt_Put (ssi_p, 0);
  SShotInfo_TMapColCnt_Put (ssi_p, 0);
  return (ssi_p);
}
/*  End of SShotInfo_Create  */


/****************************************************************************
*
*  Function Name:                 SShotInfo_Destroy
*
*    This routine frees the memory for the test results array, the sling
*    array, and the singleshot information data structure itself.
*
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
void SShotInfo_Destroy
  (
  SShotInfo_t   *ssi_p
  )
{
  U8_t           sling_i;
  U16_t          map_i;
  U32_t        **tmaps;

#ifdef _MIND_MEM_
  char varname[100];

  if (ssi_p == NULL)
    return ;

  if (SShotInfo_Slings_Get (ssi_p) != NULL)
    {
    for (sling_i = 0; sling_i < SShotInfo_NumMols_Get (ssi_p); sling_i++)
      Sling_Destroy (SShotInfo_IthSling_Get (ssi_p, sling_i));

    mind_free ("SShotInfo_Slings_Get(ssi_p)", "sglshot", SShotInfo_Slings_Get (ssi_p));
    }

  if (SShotInfo_TResults_Get (ssi_p) != NULL)
    mind_free ("SShotInfo_TResults_Get(ssi_p)", "sglshot", SShotInfo_TResults_Get (ssi_p));

  if ((tmaps = SShotInfo_TargMaps_Get (ssi_p)) != NULL)
    {
    for (map_i = 0; map_i < SShotInfo_TMapRowCnt_Get (ssi_p); map_i++)
      {
      sprintf (varname, "tmaps[%d]", map_i);
      mind_free (varname, "sglshot", tmaps[map_i]);
      }

    mind_free ("tmaps", "sglshot", tmaps);
    }

  mind_free ("ssi_p", "sglshot", ssi_p);
#else
  if (ssi_p == NULL)
    return ;

  if (SShotInfo_Slings_Get (ssi_p) != NULL)
    {
    for (sling_i = 0; sling_i < SShotInfo_NumMols_Get (ssi_p); sling_i++)
      Sling_Destroy (SShotInfo_IthSling_Get (ssi_p, sling_i));

    free (SShotInfo_Slings_Get (ssi_p));
    }

  if (SShotInfo_TResults_Get (ssi_p) != NULL)
    free (SShotInfo_TResults_Get (ssi_p));

  if ((tmaps = SShotInfo_TargMaps_Get (ssi_p)) != NULL)
    {
    for (map_i = 0; map_i < SShotInfo_TMapRowCnt_Get (ssi_p); map_i++)
      free (tmaps[map_i]);

    free (tmaps);
    }

  free (ssi_p);
#endif

  return ;
}
/*  End of SShotInfo_Destroy  */

void SingleShot_Dups_Prune (SShotGenSGs_t *ssgensg_p)
{
  int i, j, k;
  String_t *slings;
  Boolean_t found;
  SShotInfo_t *info;

#ifdef _DEBUG_
printf("SS_D_P entered\n");
#endif
#ifdef _MIND_MEM_
  mind_malloc ("slings", "sglshot{4}", &slings, SShotGenSG_NumInfos_Get (ssgensg_p) * sizeof (String_t));
#else
  slings = (String_t *) malloc (SShotGenSG_NumInfos_Get (ssgensg_p) * sizeof (String_t));
#endif
  for (i=0; i<SShotGenSG_NumInfos_Get (ssgensg_p); i++) do
    {
#ifdef _DEBUG_
printf("i=%d SShotGenSG_NumInfos_Get(%p)=%d\n",i,ssgensg_p,SShotGenSG_NumInfos_Get (ssgensg_p));
#endif
    info = SShotGenSG_IthInfo_Get (ssgensg_p, i);
    SingleShot_CompSling_Create (info, slings + i);
#ifdef _DEBUG_
printf("slings[i]=\"%s\"\n",String_Value_Get(slings[i]));
#endif

    for (j=0, found=FALSE; j<i && !found; j++) if (strcmp (String_Value_Get (slings[i]), String_Value_Get (slings[j])) == 0)
      {
#ifdef _DEBUG_
printf("j=%d\n",j);
#endif
      found=TRUE;
      String_Destroy (slings[i]);
      SShotInfo_Destroy (info);

      for (k=i+1; k<SShotGenSG_NumInfos_Get (ssgensg_p); k++)
        SShotGenSG_IthInfo_Put (ssgensg_p, k-1, SShotGenSG_IthInfo_Get (ssgensg_p, k));

      SShotGenSG_NumInfos_Put (ssgensg_p, SShotGenSG_NumInfos_Get (ssgensg_p) - 1);
#ifdef _DEBUG_
printf("end j=%d: found=%d\n",j,found);
#endif
      }
#ifdef _DEBUG_
printf("end i=%d\n",i);
#endif
    }
  while (found && i<SShotGenSG_NumInfos_Get (ssgensg_p));
#ifdef _MIND_MEM_
  mind_free ("slings", "sglshot", slings);
#else
  free (slings);
#endif
}

void SingleShot_CompSling_Create (SShotInfo_t *info, String_t *string)
{
  int i, j, len, map[100], temp;
  Boolean_t sorted;
  char temp_string[2000];

  for (i=0; i<100; i++) map[i]=i;

  for (i=0, sorted=FALSE; i<SShotInfo_NumMols_Get (info) - 1 && !sorted; i++)
    {
#ifdef _DEBUG_
printf("SS_CS_C i=%d num mols=%d\n",i,SShotInfo_NumMols_Get(info));
#endif
    sorted=TRUE;
    for (j = SShotInfo_NumMols_Get(info) - 1; j > i; j--)
      if (strcmp (Sling_Name_Get (SShotInfo_IthSling_Get (info, map[j - 1])),
        Sling_Name_Get (SShotInfo_IthSling_Get (info, map[j]))) > 0)
      {
      sorted=FALSE;
      temp=map[j];
      map[j]=map[j-1];
      map[j-1]=temp;
      }
#ifdef _DEBUG_
printf("end i=%d\n",i);
#endif
    }

  len = 0;
  temp_string[0] = '\0';

  for (i=0; i<SShotInfo_NumMols_Get (info); i++)
    {
#ifdef _DEBUG_
printf("i=%d\n",i);
#endif
    sprintf(temp_string+len,"%s%%",Sling_Name_Get (SShotInfo_IthSling_Get (info, map[i])));
    len=strlen(temp_string);
#ifdef _DEBUG_
printf("end i=%d\n",i);
#endif
    }

#ifdef _MIND_MEM_
  mind_malloc ("String_Value_Get(*string)", "sglshot{5}", &String_Value_Get (*string), len + 1);
#else
  String_Value_Get (*string) = (char *) malloc (len + 1);
#endif
  String_Alloc_Put (*string, len + 1);
  String_Length_Put (*string, len);
  strcpy (String_Value_Get (*string), temp_string);
}
