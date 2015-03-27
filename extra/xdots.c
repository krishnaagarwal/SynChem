/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     DOTS.C
*
*    Provides for the global dotting of the reaction library, such as following
*    the introduction of a new dotting algorithm.
*
*  Creation Date:
*
*    16-Nov-2000
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
#include "synio.h"
#include "rcb.h"
#include "tsd.h"
#include "xtr.h"
#include "tsd2xtr.h"
#include "reaction.h"
#include "persist.h"
#include "avldict.h"
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

void local_backup_rxnlib ()
{
  FILE *source, *destination;
  int c;

  source = fopen (FCB_SEQDIR_RXNS ("/rkbstd.inx"), "rb");
  destination = fopen (FCB_SEQDIR_RXNS ("/rkbstd.inx.bak"), "wb");
  while ((c = getc (source)) != EOF) putc(c, destination);
  fclose (source);
  fclose (destination);

  source = fopen (FCB_SEQDIR_RXNS (REACT_DATANAME), "rb");
  destination = fopen (FCB_SEQDIR_RXNS (REACT_DATANAME ".bak"), "wb");
  while ((c = getc (source)) != EOF) putc(c, destination);
  fclose (source);
  fclose (destination);

  source = fopen (FCB_SEQDIR_RXNS (REACT_TEXTNAME), "rb");
  destination = fopen (FCB_SEQDIR_RXNS (REACT_TEXTNAME ".bak"), "wb");
  while ((c = getc (source)) != EOF) putc(c, destination);
  fclose (source);
  fclose (destination);
}

main()
{
  Tsd_t *gtsd, *sgtsd;
  Xtr_t *gxtr, *sgxtr;
  React_Record_t *schema;
  React_Head_t *sch_head;
  U32_t roots[MX_ROOTS];
  int i, sch_num, sch_inx;
  char buffer[25600];
  Boolean_t is_temp;

  local_backup_rxnlib ();

  Debug_Init ();
  Trace_Init ();
  IO_Init ();
#ifdef _CYGWIN_
  IO_Init_Files ("NUL:", FALSE);
#else
  IO_Init_Files ("/dev/null", FALSE);
#endif

  React_Initialize ((U8_t *) concat ("R+W", FCB_SEQDIR_RXNS("")),
    Persist_Inx_OK (FCB_SEQDIR_RXNS ("/rkbstd.inx"), concat ("R+W", FCB_SEQDIR_RXNS(""))));

  Persist_Open (FCB_SEQDIR_RXNS ("/rkbstd.inx"), NULL, NULL, FALSE);

  for (sch_inx = 0; sch_inx < React_NumSchemas_Get (); sch_inx++)
    if (Persist_Legacy_Rxn (sch_inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp))
  {
    sch_num = Persist_Current_Rec (PER_STD, sch_inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp);
printf("\ndotting %d (%d)\n",sch_inx,sch_num);

    schema = React_Schema_Handle_Get (sch_num);
    sch_head=React_Head_Get(schema);

    gtsd = React_Goal_Get (schema);

    sgtsd = React_Subgoal_Get (schema);

    gxtr = Tsd2Xtr (gtsd);
    sgxtr = Tsd2Xtr (sgtsd);

    for (i = 0; i < MX_ROOTS; i++) roots[i]=React_Head_RootNode_Get(sch_head, i);

    buffer[0] = '\0'; /* must initialize each time!!! */
    dots (gxtr, sgxtr, roots, buffer);
printf("%s\n",buffer);

    Xtr_Destroy (sgxtr);
    Tsd_Destroy (gtsd);

    React_Goal_Put (schema, Xtr2Tsd (gxtr));

    Xtr_Destroy (gxtr);

    React_Schema_Write (sch_num);

printf("finished dotting %d (%d)\n\n",sch_inx,sch_num);
  }

  Persist_Close ();
}
