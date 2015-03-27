#define _H_PRE_TEMPLATE_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     PRE_TEMPLATE.H
*
*    This module defines the mnemonics and functions used in the management of
*    the pretransform test templates.
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
*
******************************************************************************/

#define PTFILE_EXISTS 1
#define PTFILE_ERROR 2
#define PTFILE_OK 3

int PreTemplatesRead (char **, int);
int PreTemplateWrite (char *, char *, char *, Boolean_t *, Boolean_t);
