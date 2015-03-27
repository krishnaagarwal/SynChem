#ifndef _H_DEBUG_
#define _H_DEBUG_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     DEBUG.H
*
*    This module is the description of all of the pieces of the debugging
*    and tracing system.
*
*    In the tracing structure, the options field is for tracing, the
*    params field is for debugging.  This is key!
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

/*** Literal Values ***/

#define DB_RESERVED       0       /* Reserved for future use */
#define DB_MAIN           1       /* Main routine */
#define DB_CHEMISTRY      4       /* Chemical tracing stuff */
#define DB_INIT           5       /* Any initialization routine */

/* SYNIO */

#define DB_IO_GET         10      /* IO input routines */
#define DB_IO_PUT         11      /* IO output routines */
#define DB_IO_DUMP        12      /* IO dump routines */

/* ARRAY */

#define DB_ARRAY1DCREATE  21      /* 1D Array create */
#define DB_ARRAY2DCREATE  22      /* 2D Array create */
#define DB_ARRAY3DCREATE  23      /* 3D Array create */
#define DB_ARRAYDESTROY   24      /* Array destroy */
#define DB_ARRAYCOPY      25      /* Array copy */
#define DB_ARRAYCOPYCONTENTS 26   /* Copy array contents, (same size) */

/* UTL */

#define DB_ATMARRCOPY     31      /* Copy and Atom array */
#define DB_ATMARRCREATE   32      /* Atom array create */
#define DB_ATMARRDESTROY  33      /* Atom array destroy */
#define DB_STACKCREATE    34      /* Create a stack */
#define DB_STACKDESTROY   35      /* Destroy a stack */
#define DB_STACKPOP       36      /* Pop a value off a stack */
#define DB_STACKPUSH      37      /* Push a value on a stack */
#define DB_LNKDTCOPY      38      /* Copy a Linked Data structure */
#define DB_LNKDTCREATE    39      /* Create a Linked Data structure */
#define DB_LNKDTDESTROY   40      /* Destroy a Linked Data structure */
#define DB_STRINGCONCAT   41      /* Concatenate two strings */
#define DB_STRINGCOPY     42      /* Copy a String */
#define DB_STRINGCREATE   42      /* Create a String */
#define DB_STRINGDESTROY  43      /* Destroy a String */
#define DB_LISTINSERT     44      /* Insert an item into a list */
#define DB_LISTREMOVE     45      /* Remove an item from a list */

/* AROMATICS */

#define DB_AROMCOPY       60      /* Copy an Aromatics structure */
#define DB_AROMCREATE     61      /* Create an Aromatics structure */
#define DB_AROMDESTROY    62      /* Destroy an Aromatics structure */
#define DB_AROMSET        63      /* Set the aromatic bonds */
#define DB_AROMFINDBONDS  64      /* Find aromatic bonds */

/* ATOMS */

#define DB_ATOMCOPY       66      /* Copy an Atoms structure */
#define DB_ATOMCREATE     67      /* Create an Atoms structure */
#define DB_ATOMDESTROY    68      /* Destroy an Atoms structure */
#define DB_ATOMSET        69      /* Calculate the Atoms information */

/* AVLCOMP */

#define DB_AVCPINFDESTROY 70      /* Destroy an avail. comp. info. structure */
#define DB_AVCPINFEXTRACT 71      /* Read in an avail. comp. info. structure */
#define DB_AVCPINFEXTRUDE 72      /* Write out avail. comp. info. structure */

/* FUNCGROUPS */

#define DB_FNGRPCOPY      90      /* Copy a Func. Group stucture */
#define DB_FNGRPCREATE    91      /* Create a Func. Group structure */
#define DB_FNGRPDESTROY   92      /* Destroy a Func. Group structure */
#define DB_FNGRPSUBINSTOK 93      /* Check if substructure instance is ok */
#define DB_FNGRPINIT      94      /* Func. Groups initializer */
#define DB_FNGRPSTATIC    95      /* Static assist routines */

/* NAME */

#define DB_NAMECOPY       101     /* Copy a Name */
#define DB_NAMECREATE     102     /* Create a Name */
#define DB_NAMEDESTROY    103     /* Destroy a Name */
#define DB_NAMESET        104     /* Calculate the Name information */
#define DB_NAMESLINGGET   105     /* Fetch the Sling from the Name */
#define DB_NAMESLINGMAP   106     /* Fetch Sling and Map of Name */

/* PST */

#define DB_PSTBROTHERGET  110     /* Get brother node in PST */
#define DB_PSTBROTHERPUT  111     /* Set brother node in PST */
#define DB_PSTCOMPCREATE  112     /* Create new compound node */
#define DB_PSTFATHERGET   113     /* Get father node in PST */
#define DB_PSTFATHERPUT   114     /* Set father node in PST */
#define DB_PSTSONGET      115     /* Get son node in PST */
#define DB_PSTSONPUT      116     /* Set son node in PST */
#define DB_PSTNUMSONS     117     /* Get number of sons of a node */
#define DB_PSTSUBGCREATE  118     /* Create a new subgoal node */
#define DB_PSTCOMPMERITSET 119    /* Set new shelved merit */
#define DB_PSTROOTSET     120     /* Set initial compound in PST */
#define DB_PSTSYMTABCREATE 121    /* Symbol table entry create */
#define DB_PSTSUBGINSERT  122     /* Insert new subgoal into PST */

/* REACTION */

#define DB_REACTDESTROY   130     /* Reaction record destroy */
#define DB_REACTREAD      131     /* Reaction data file record input */
#define DB_REACTXTREAD    132     /* Reaction text file record input */
#define DB_REACTWRITE     133     /* Reaction data file record output */
#define DB_REACTXTWRITE   134     /* Reaction text file record output */
#define DB_REACTINIT      135     /* Reaction library initialization */
#define DB_REACTSCHEMAOK  136     /* Check schema/XTR are compatible */

/* RESONANTBONDS */

#define DB_RESBONDSFIND   160     /* Resonant bond finding */
#define DB_RESBONDSFINDSPIRO 161  /* Resonant bond finding, incl. spiro */
#define DB_RESBONDSTATIC  162     /* Static routine - finding resonant bonds */
#define DB_RESBONDSFIX    163     /* Resonant bond fixing, "unfinding" */

/* RINGDEFINITION */

#define DB_RINGDEFCOPY    165     /* Copy a ring definition */
#define DB_RINGDEFCREATE  166     /* Create a ring definition */
#define DB_RINGDEFDESTROY 167     /* Destroy a ring definition */
#define DB_RINGDEFCOMFIND 168     /* Find common rings in ring definition */
#define DB_RINGDEFMINRINGFIND 169 /* Find minimum ring in ring def ??? */
#define DB_RINGDEFMIN4RINGFIND 170 /* Find minimum ring of size 4 ??? */
#define DB_RINGDEFPRMRNGNDSGET 171 /* Get mask for primary ring nodes */
#define DB_RINGDEFSTATIC  172     /* Static routine for ring definition */
#define DB_RINGDEFPRMCYCFIND 173  /* Find a prime cycle */
#define DB_RINGDEFCITERNG 174     /* Alternate entry for prime cycle finding */

/* RINGS */

#define DB_RINGCOPY       177     /* Copy a Ring data-struct */
#define DB_RINGCREATE     178     /* Create a Ring data-struct */
#define DB_RINGDESTROY    179     /* Destroy a Ring data-struct */
#define DB_RINGSYSFIND    180     /* Build the ring systems */
#define DB_RINGBONDSET    181     /* Bond mask for ring */
#define DB_RINGSTATIC     182     /* Static routine for ring manipulation */
#define DB_XTRETCMISC     183     /* For edge create/destroy routines */

/* RINGSYSTEMS */

#define DB_RINGSYSCOPY    185     /* Copy a Ring System */
#define DB_RINGSYSCREATE  186     /* Create a Ring System list */
#define DB_RINGSYSDESTROY 187     /* Destroy a Ring System list */
#define DB_RINGSYSSTATIC  188     /* XTR components static routine */

/* SLING */

#define DB_SLINGCOPY      190     /* Copy a Sling */
#define DB_SLINGCREATE    181     /* Create a Sling */
#define DB_SLINGDESTROY   192     /* Destroy a Sling */
#define DB_SLINGCANNAME2XTR 193   /* Convert a Canonical Name/? to XTR */
#define DB_SLING2TSD      194     /* Convert a Sling to a TSD */
#define DB_SLING2TSDPLHY  195     /* Convert Sling to TSD and add hydrogen */
#define DB_SLING2XTR      196     /* Convert Sling to XTR */
#define DB_SLING2XTRPLHY  197     /* Convert Sling to XTR and add hydrogen */
#define DB_SLINGSTATIC    198     /* Static routine */
#define DB_SLINGCANONGEN  199     /* Generate canonical sling */
#define DB_SLINGVALIDATE  200     /* Validate a Sling for syntax */

/* TSD */

#define DB_TSDATOMCONN    220     /* Connect two atoms in a TSD */
#define DB_TSDATOMCONCHNG 221     /* Change the connection of two atoms */
#define DB_TSDATOMDISC    222     /* Disconnect two atoms in a TSD */
#define DB_TSDATOMNEIGHGET 223    /* Get array of neighbor ids */
#define DB_TSDBONDCHANGE  224     /* Set the bond between two atoms */
#define DB_TSDATOMVALGET  225     /* Get the valence of an atom */
#define DB_TSDCOPY        226     /* Get a copy of a TSD */
#define DB_TSDCREATE      227     /* Create a new TSD */
#define DB_TSDDESTROY     228     /* Destroy a TSD */
#define DB_TSDEXPAND      229     /* Expand the # atoms in a TSD */
#define DB_TSDMATCOMP     230     /* Compress two matching TSDs */
#define DB_TSDVERIFY      231     /* Verify a TSD is consistent */
#define DB_TSD2XTR        232     /* Convert a TSD to an XTR */
#define DB_TSD2SLING      233     /* Convert a TSD to a Sling */
#define DB_TSD2SLINGX     234     /* Convert a TSD to a Sling and more */
#define DB_TSDSTATIC      235     /* TSD module static routine */

/* XTR */

#define DB_XTRCOMPRESS    240     /* Compress an XTR */
#define DB_XTRCOPY        241     /* Copy an XTR */
#define DB_XTRCREATE      242     /* Create an XTR */
#define DB_XTRDESTROY     243     /* Destroy an XTR */
#define DB_XTRCOPYSUBSET  244     /* Copy a subset of an XTR */
#define DB_XTRCOPYEXPAND  245     /* Copy and expand an XTR */
#define DB_XTRRESBONDSET  246     /* Find the bonds, then set them in XTR */
#define DB_XTRRINGSET     247     /* Setup ring structures */
#define DB_XTRRINGDEFSET  248     /* Set the Ring definition */
#define DB_XTR2SLING      249     /* Convert an XTR to a Sling */
#define DB_XTR2TSD        250     /* Convert XTR to TSD */
#define DB_XTRSTATIC      251     /* Static routines used by XTR module */

/* ISAM */

#define DB_ISAMCLOSE      260     /* Close an ISAM file */
#define DB_ISAMOPEN       261     /* Open an ISAM file */
#define DB_ISAMREAD       262     /* Read (buffered) an ISAM record */
#define DB_ISAMREADNEXT   263     /* Read next part of buffered record */
#define DB_ISAMREADNOBUF  264     /* Read (unbuffered) an ISAM record */
#define DB_ISAMRECINBUF   265     /* Test if end of record in current buffer */
#define DB_ISAMINBUFSIZE  266     /* How many bytes (record) in cur. buffer */
#define DB_ISAMWRITE      267     /* Write an ISAM record */

/* FUNCGROUPS FILES */

#define DB_FNGRPRECREAD   270     /* Read a Func. Group info record */
#define DB_FNGRPRECWRITE  271     /* Write a Func. Group info record */
#define DB_FNGRPRECDESTROY 272    /* Destroy a Func. Group info record */

/* SUBGOAL GENERATION */

#define DB_SUBGENRCREATE  275     /* Create a Match CB, Leaf or Node  */
#define DB_SUBGENRDESTROY 276     /* Destroy a MatchCB */
#define DB_SUBGENRFRAGMTCH 277    /* Match fragment */
#define DB_SUBGENRACTIVE  278     /* Match with subgoal */
#define DB_SUBGENRSTATIC  279     /* Static routines, many */
#define DB_SUBGENR        280     /* SubGenr_main itself */

/* Max literals */

#define MX_ROUTINES       512     /* # routines for tracing */

/* Tracing / debugging levels.  Higher number is more detail
   A major event is  - successful pathway found
   A minor event is  - related to a single expansion cycle
   A loop event is   - something in the top level executive loops over
                       all the schemas
   A detail event is - something of interest below the level of a single
                       schema or a detail about a single schema
*/

#define TL_NONE           0       /* Don't trace anything */
#define TL_RUNSTATS       1       /* Save only run statistics */
#define TL_SELECT         2       /* Trace selected slings */
#define TL_TRACE          3       /* Trace specific item, but nothing else */
#define TL_MEMORY         4       /* List when memory de/allocation occurs */
#define TL_INIT           5       /* List when an initialization occurs */
#define TL_IO             6       /* List when file io occurs */
#define TL_MAJOR          7       /* List when a major event occurs */
#define TL_MINOR          8       /* List when a minor event occurs */
#define TL_LOOP           10      /* List stuff in useful loops */
#define TL_DETAIL         12      /* List when a detail event occurs */
#define TL_PARAMS         15      /* List inputs and outputs */
#define TL_ALWAYS         20      /* Force output */

/* Exit reasons */

#define X_RESERVED      0
#define X_SYSCALL       1
#define X_LIBCALL       2
#define X_USERREQ       3
#define X_SYNERR        4
#define X_MEMTRASHED    5

#define X_MAX           6

#define HALT            IO_Exit_Error (R_MAIN, X_SYNERR, "")
#define HALTP           ((Address) (U32_t) IO_Exit_Error (R_MAIN, X_SYNERR, ""))
#define HALTSL          IO_Exit_Error_Sling (R_MAIN, X_SYNERR, "")
#define HALTST          Sling2String (IO_Exit_Error_Sling (R_MAIN, X_SYNERR, ""))

/* Loop frequency constants for main Synchem loop */

#define FREQ_ALWAYS     0
#define FREQ_SUBGOALS   1
#define FREQ_LIBRARY    2
#define FREQ_CHAPTER    3
#define FREQ_SCHEMA     4

/* Routine name constants for tracing, debugging, and error messages */

#define R_RESERVED       0
#define M_RESERVED       "Reserved"

#define R_AVL            1
#define M_AVL            "Available Compounds Library"

#define R_REACTION       2
#define M_REACTION       "Reaction Library"

#define R_IO             3
#define M_IO             "IO Module"

#define R_MAIN           4
#define M_MAIN           "Synchem Executive"

#define R_SUBGENR        5
#define M_SUBGENR        "Subgoal Generation"

#define R_ARRAY          6
#define M_ARRAY          "Array Module"

#define R_UTL            7
#define M_UTL            "Utility Module"

#define R_SLING          8
#define M_SLING          "Sling Module"

#define R_XTR            9
#define M_XTR            "XTR Module"

#define R_PST            10
#define M_PST            "PST Module"

#define R_STRATEGY       11
#define M_STRATEGY       "Strategy Module"

#define R_TSD            12
#define M_TSD            "TSD Module"

#define R_ISAM           13
#define M_ISAM           "ISAM Module"

#define R_POSTTEST       14
#define M_POSTTEST       "Post-test Module"

#define R_NONE           15
#define M_NONE           ""

#define MX_MODULES       16

/*** Data Structures ***/

typedef struct s_trace
  {
  U8_t            options;                   /* For tracing  */
  U8_t            params;                    /* For debugging */
  } SynTrace_t;
#define TRACESIZE sizeof (SynTrace_t)

/*** Macros ***/

/* The condition must be able to be put into an if-statement, and the action
   must be enclosed in braces so that it won't affect surrounding if-statements
*/

#define ASSERT(condition, fail_action)\
  if (!(condition))\
    fail_action

/* Check the routine's debug level (params) and if greater than the level, then
   format the output and call IO_Put_Debug to print it.  Note, 'outbuf' must
   be the first thing in action so that the sprintf works.
*/

#define DEBUG(module, routine, level, action)\
  if (GTrace[routine].params >= (level) || (level) == TL_ALWAYS)\
    { char outbuf[MX_OUTPUT_SIZE]; (void) sprintf action;\
     IO_Put_Debug (module, outbuf); }

/* Check the routine's debug level (params) ..., and action should be a
   curly brace ({}) surrounded action.
*/

#define DEBUG_DO(routine, level, action)\
  if (GTrace[routine].params >= (level))\
    action

#define DEBUG_ADDR(module, routine, ptr)\
  if (GTrace[routine].params >= TL_MEMORY)\
    (void) fprintf (GStdErr.handle, "DEBUG: Handle = %p ", ptr)

/* Check the routine's trace level (options) and if greater than the level,
   then format the output and call IO_Put_Trace to print it.  Note, 'outbuf'
   must be the first thing in action so that the sprintf works.
*/

#define TRACE(module, routine, level, action)\
  if (GTrace[routine].options >= (level) || (level) == TL_ALWAYS)\
    { char outbuf[MX_OUTPUT_SIZE]; (void) sprintf action;\
     IO_Put_Trace (module, outbuf); }

#define CHEMTRACE(level, action)\
  if (GTrace[DB_CHEMISTRY].options >= (level) || (level) == TL_ALWAYS)\
    { char outbuf[MX_OUTPUT_SIZE]; (void) sprintf action;\
     fprintf (GTraceFile.handle, "Chemistry explanation: %s\n", outbuf); }

#define TIMETRACE(module, routine, level, action)\
  if (GTrace[routine].options >= (level) || (level) == TL_ALWAYS)\
    { char outbuf[MX_OUTPUT_SIZE]; (void) sprintf action;\
     IO_Put_Trace_Time (outbuf); }

/* Check the routine's trace level (options) ..., and action should be a
   curly brace ({}) surrounded action.
*/

#define TRACE_DO(routine, level, action)\
  if (GTrace[routine].options >= (level))\
    action

/*** Global Variables ***/

#ifdef IO_GLOBALS
       void       *GBAddr;
       const char *GModuleNames[MX_MODULES];
       const char *GExitReasons[X_MAX];
       SynTrace_t  GTrace[MX_ROUTINES];  /* Trace options for each routine */
#else
extern void       *GBAddr;
extern const char *GModuleNames[MX_MODULES];
extern const char *GExitReasons[X_MAX];
extern SynTrace_t  GTrace[MX_ROUTINES];  /* Trace options for each routine */
#endif

/* End of Debug.H */
#endif
