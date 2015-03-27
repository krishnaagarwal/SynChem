#ifndef _H_EXTERN_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook, Gerald A. Miller
*
*  Module Name:                     EXTERN.H
*
*    This header contains some declarations common to main modules, as well as
*    the extern declarations needed by other modules to reference some of these.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    02-Oct-2000
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xxx
*
******************************************************************************/


#define _H_EXTERN_

#ifdef _GLOBAL_DEF_
#undef _GLOBAL_DEF_
#define _GLOBAL_EXT_DEF_ 1
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

#ifdef _GLOBAL_EXT_DEF_
#undef _GLOBAL_EXT_DEF_

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_SYN_VIEW_
/* The following are needed to satisfy modules that reside in syn_view.c when linking other main programs */
void PstView_Handle_Grab ()
{
}

void SynView_Handle_Grab ()
{
}

void SynView_Exit ()
{
}
#endif

#ifndef _H_DRAWPAD_
void DrawPad_Drawn_Sling (char *sling)
{
}
#endif

Avd_Dict_Control_t   SDictControl;     /* dict control structure */
int slg_count;
Avi_CmpInfo_t *avinfo_p[50000];
char session_code[10];

/* The following are needed to satisfy extern declarations in other modules */
char sshot_comp[512];
char *fgname[1000];
Boolean_t fgprsv[1000];
int fgend;
Boolean_t write_fail_fatal = TRUE;
char write_fail_str[256];
char *glob_main_dir = NULL;
void *toplevel; /* No need to include X11 or Xt definitions for non-gui code */
Date_t glob_run_date = PER_TIME_ANY;
long glob_run_time = PER_TIME_ANY;
Boolean_t glob_rxlform = TRUE;
Boolean_t glob_special = FALSE;
Compound_t *glob_sel_cmp = NULL;

char *concat (char *first, char *second)
{
  static int num_concats = 0;
  static char *concats[100];
  char tconcat[256];
  int i;

  if (glob_main_dir == NULL)
  {
    glob_main_dir = getenv ("SYNDATA");
    if (glob_main_dir == NULL)
    {
      fprintf (stderr, "ERROR: SYNDATA environment variable undefined - can't continue.\n");
      exit(1);
    }
    if (first==NULL) first=glob_main_dir; /* Needed because of pass-by-value! */
  }

  sprintf (tconcat, "%s%s", first, second);

  for (i = 0; i < num_concats; i++) if (strcmp (concats[i], tconcat) == 0) return (concats[i]);

  if (num_concats == 100)
  {
    fprintf (stderr, "ERROR; Table overflow in concat() (see \"extern.h\").\n");
    exit(1);
  }

  concats[num_concats] = (char *) malloc (strlen (tconcat) + 1);
  strcpy (concats[num_concats], tconcat);
  num_concats++;
  return (concats[num_concats - 1]);
}

#else

#ifdef _H_AVLFORM_
extern Avd_Dict_Control_t   SDictControl;     /* dict control structure */
extern int slg_count;
extern Avi_CmpInfo_t *avinfo_p[50000];
#endif

extern char sshot_comp[512];
extern char *fgname[1000];
extern Boolean_t fgprsv[1000];
extern int fgend;
extern Boolean_t write_fail_fatal;
extern char write_fail_str[256];
extern char *glob_main_dir;
extern char session_code[10];
extern void *toplevel;
extern Date_t glob_run_date;
extern long   glob_run_time;
extern Boolean_t glob_rxlform;
extern Boolean_t glob_special;
extern Compound_t *glob_sel_cmp;

char *concat (char *, char *);
#endif

#endif
