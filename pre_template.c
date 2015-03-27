/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     PRE_TEMPLATE.C
*
*    Extracts and stores the functional group lists making up a property-
*    defined template for mass entry of pretransform tests.
*
*  Creation Date:
*
*    16-Feb-2000
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

#include <stdio.h>
#ifdef _CYGWIN_
#include <string.h>
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "rcb.h"

#ifndef _H_XTRFUNGROUP_
#include "funcgroups.h"
#endif

#ifndef _H_PRE_TEMPLATE_
#include "pre_template.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

#ifdef _CYGWIN_
FILE *popen_fix_for_dos (char *command, char *mode)
{
  char new_command[256], *cygd;

  strcpy(new_command,command);
  cygd=strstr(new_command,"/cygdrive");
  strcpy(cygd,cygd+9);
  cygd[0]=cygd[1];
  cygd[1]=':';
#ifdef _DEBUG_
debug_print(new_command);
#endif
  return(popen(new_command,mode));
}
#endif

int PreTemplatesRead (char **tempfname, int max_temps)
{
  FILE *p;
  char tname[150];
  int i, len;

#ifdef _CYGWIN_
  p = popen_fix_for_dos (concat ("ls ", FCB_SEQDIR_FGTM ("/*.txt")), "r");
#else
  p = popen (concat ("ls ", FCB_SEQDIR_FGTM ("/*.txt")), "r");
#endif

  for (i=0; i < max_temps && fgets (tname, 149, p) != NULL; i++)
    {
    len=strlen(tname);
    tempfname[i]=(char *) malloc(len);
    tname[len-1]='\0';
    strcpy(tempfname[i], tname);
#ifdef _CYGWIN_
if (tempfname[i][strlen(tempfname[i])-1]<' ') tempfname[i][strlen(tempfname[i])-1]='\0';
printf("i=%d; tempfname[i]=\"%s\"\n",i,tempfname[i]);
#endif
    }
  pclose(p);
  return(i);
}

int PreTemplateWrite (char *tempfname, char *tempname, char *tempabbr, Boolean_t *template, Boolean_t overwrite)
{
  int i;
  FILE *f;
  char full_fname[150];

  if (tempfname[0]=='/') strcpy(full_fname,tempfname);
  else sprintf (full_fname,FCB_SEQDIR_FGTM ("/%s.txt"), tempfname);

  if (!overwrite)
    {
    f=fopen (full_fname,"r");
    if (f!=NULL)
      {
      fclose(f);
      return(PTFILE_EXISTS);
      }
    }
  f=fopen (full_fname,"w");
  if (f==NULL) return (PTFILE_ERROR);

  fprintf(f,"%s\n%s\n",tempname,tempabbr);
  for (i=0; i<MX_FUNCGROUPS; i++) if (template[i]) fprintf(f,"%d\n",i);
  fclose(f);
  return(PTFILE_OK);
}
