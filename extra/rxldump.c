#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

#ifndef _H_REACT_FILE_
#include "react_file.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

/* Static variables */

static Isam_Control_t SIsam_CB;              /* ISAM control block */

main()
{
  FuncGrp_Record_t *fgrec_p;                 /* Buffer - reading FGINFO file */
  React_Record_t *drec_p;                    /* RKB data record */
  React_TextRec_t *trec_p;                   /* RKB text record */
  U16_t          i;                          /* Counter */
  U16_t          length;                     /* Length of file name input */
  char           ibuf[MX_INPUT_SIZE];        /* Input buffer */
  Isam_Control_t rkbtextcb;                  /* Second ISAM file for text */

Debug_Init ();
Trace_Init ();
IO_Init ();
GStdErr.handle = stdout;
GTraceFile.handle = stdout;

    strcpy (IO_FileName_Get (Isam_File_Get (&SIsam_CB)), "//D/SYNCHEM/testdata/rxnlib/rkbdata.isam");
    Isam_Open (&SIsam_CB, ISAM_TYPE_RKBDATA, ISAM_OPEN_READ);

    strcpy(ibuf,"//D/SYNCHEM/testdata/rxnlib/rkbtext.isam");
    length=strlen(ibuf);
    memcpy (IO_FileName_Get (Isam_File_Get (&rkbtextcb)), ibuf, length);
    Isam_Open (&rkbtextcb, ISAM_TYPE_RKBTEXT, ISAM_OPEN_READ);

    for (i = 0; i < Isam_NextKey_Get (&SIsam_CB); i++) if (i!=1075 && i!=1076 && i!=1077 && i!=1163)
      {
      drec_p = React_Read (i, &SIsam_CB);
      if (drec_p == NULL)
        printf ("Record with key %u does not exist\n", i);
      else
        {
        trec_p = React_TextRead (i, &rkbtextcb,
          React_Head_NumTests_Get (React_Head_Get (drec_p)));
        React_Text_Put (drec_p, trec_p);
        React_Dump (drec_p, &GStdOut);
        React_Destroy (drec_p);
        }
      }
}
