#ifndef _H_SYNHELP_
#define _H_SYNHELP_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook, Gerald A. Miller
*
*  Module Name:                     SYNHELP.H
*
*    This header contains the declarations needed by the module synhelp.c and
*    others that call it.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    21-Mar-2000
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


static Widget helpform, helpwin, helptxt;
static char helpbuf[8192];

void Help_Form_Create (Widget);
void Help_CB (Widget, XtPointer, XtPointer);
void HelpDismiss_CB (Widget, XtPointer, XtPointer);

#endif
