/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RXL_EXPORT.C
*
*    This utility creates an ASCII dump of the reaction library for use with the
*    RXL_IMPORT utility in constructing a replica of the library.
*
*  Creation Date:
*
*    10-Oct-2000
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

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"

#include "avldict.h"

#include "reaction.h"

#include "persist.h"

#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

main ()
{
	int i, j;
	char buf[32768];

	Persist_Dump_RxnInx (stdout);
	React_Init ("/home/synchem/data/rxnlib");
	i = React_NumSchemas_Get ();
	for (j = 0; j < i; j++)
	{
		React_Schema_Export (j, buf);
		printf (buf);
	}
}
