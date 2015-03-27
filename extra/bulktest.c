/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     POSTTEST.C
*
*    This module contains all of the functions used in evaluating
*    the Post-transform tests.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    27-Feb-1995
*
*  Authors:
*
*    Lichan Hong      (translated from PL1 source code)
*    Daren Krebsbach  (extensive debugging--yet more to do)
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred	xxx
* 27-Nov-00  Miller     Changed sign of SCarbstb contributions from carbonyl,
*                       nitrile, etc. to reflect their destabilizing influences
* 29-Nov-00  Miller     Modified SPursue to accommodate intent of Boivie thesis
*                       (apparently missing in original PL/I version) that only
*                       non-H nodes be included in BULKFRGN
* 29-Jun-01  Miller     Backed out change to BULKFRGN to restore historic (and
*                       more correct) behavior - this change is tentative,
*                       pending a better understanding of the heuristics thereof
* 16-Oct-01  Miller     Converted several condition types to fuzzy logic, so that
*                       a relative continuum of eyc adjustments could be used in
*                       cases of borderline results.
* 19-Dec-01  Miller     Removed incomplete and erroneous fuzziness from AROM2;
*                       revised standings to better reflect similarities and
*                       differences relative to rating approximations.
*
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_NAME_
#include "name.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_POSTTEST_
#include "posttest.h"
#endif

#ifndef _H_SUBGOALGENERATION_
#include "subgoalgeneration.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

#define ARSTD_MULT 2
#define ARRAT_MULT 6
#define CARSB_MULT 10
#define ELECW_MULT 10
#define LVGRP_MULT 4
#define MGAPT_MULT 2

#define Zadehan_Byteval(val) (val).fuzzy_value

typedef union
{
  U8_t fuzzy_value;
  struct
  {
    unsigned TF   : 1;
    unsigned conf : 7;
  } fuzzy_components;
} Zadehan_t;

/* Routine Prototypes */

static void	     SAdd_Name (String_t *, STree_node_t *, Boolean_t, Xtr_t *);
static U16_t	     SAdjmult (Xtr_t *, U16_t, U8_t);
static Boolean_t     SAlkynsr (U8_t, U8_t, Xtr_t *, Array_t *);
static Boolean_t     SAllensr (U8_t, U8_t, Xtr_t *, Array_t *);
static U8_t         *SArom_Fusion (char *, U16_t *);
static Boolean_t     SAsearch (Xtr_t *, U16_t, U16_t, Boolean_t, U16_t, U8_t, 
                       Array_t *, Array_t *);
static Xtr_t        *SAtable (U16_t, U16_t *);
static Boolean_t     SAtmdist (U8_t, U8_t, U16_t, Array_t *, Array_t *,
                       Xtr_t *, Array_t *);
static U16_t         SAtom (U8_t, Xtr_t *, Array_t *);
static String_t      SAtom_Sym (U16_t);
static Boolean_t     SBinary_Search (U8_t *, S16_t, S16_t, S16_t *);
static U8_t         *SBlkfrgn (Xtr_t *, U16_t, U8_t);
static Boolean_t     SBredts_Rule_Violation (Xtr_t *);
static Boolean_t     SBrghead (Xtr_t *, U16_t);
static Boolean_t     SBridges_Ok (Tsd_t *, U16_t *, U16_t *, Path_List_t *);
static void          SBuild_Constant (Xtr_t *, Xtr_t *, Array_t *,
                       Array_t *, U8_t);
static SDistance_t  *SBuild_Distance_Structure (U16_t, U16_t);
static U8_t         *SBulk (U8_t, Xtr_t *, Xtr_t *, Array_t *);
static U8_t         *SBulk2 (U8_t, U8_t, Xtr_t *, Array_t *);
static U8_t         *SBulksln (const char *);
static Boolean_t     SCalc_Efx_Etc (Xtr_t *, U16_t *, float *, float *, 
                       Boolean_t *, Boolean_t *);
static float         SCarbstb (U8_t, U8_t, Xtr_t *, Xtr_t *, Array_t *);
static Boolean_t     SCediscn (Xtr_t *, U8_t, Array_t *, Xtr_t *, Array_t *);
static Boolean_t     SCediscn_Or_SSediscn (Xtr_t *, U8_t, Array_t *, 
                       Boolean_t, Xtr_t *, Array_t *);
/*
static Boolean_t     SCompare_Two_Strings (U8_t *, U8_t *, U8_t);
*/
static Zadehan_t     SCompare_Two_Strings (U8_t *, U8_t *, U8_t);
/*
static Boolean_t     SCompare_Two_Values (S16_t, S16_t, U8_t);
*/
static Zadehan_t     SCompare_Two_Values (S16_t, S16_t, U8_t, U8_t);
static Boolean_t     SCompok (Xtr_t *);
/*
static Boolean_t     SCondition (Condition_t *, Xtr_t *, Array_t *, U16_t, 
*/
static U8_t          SCondition (Condition_t *, Xtr_t *, Array_t *, U16_t, 
                       Array_t *, U16_t, Xtr_t *, Xtr_t *, Array_t *,
                       Array_t *, Array_t *, Array_t *, U8_t);
static Boolean_t     SCond_Alkyne_Test (Condition_t *, Xtr_t *, Array_t *);
static Boolean_t     SCond_Allene_Test (Condition_t *, Xtr_t *, Array_t *);
/*
static Boolean_t     SCond_Aromat_Sub_Test (Condition_t *, Array_t *, 
*/
static Zadehan_t     SCond_Aromat_Sub_Test (Condition_t *, Array_t *, 
                       Array_t *, Array_t *);
static Boolean_t     SCond_At_Coneq_Or_Streq_Test (Condition_t *, Boolean_t, 
                       Xtr_t *, Array_t *, Array_t *, Array_t *, Array_t *);
static Boolean_t     SCond_Atom_Test (Condition_t *, Array_t *, Array_t *, 
                       Xtr_t *, Array_t *);
static Boolean_t     SCond_Bridge_Head_Test (Condition_t *, Xtr_t *, 
                       Array_t *);
/*
static Boolean_t     SCond_Bulky_Test (Condition_t *, Xtr_t *, Xtr_t *, 
*/
static Zadehan_t     SCond_Bulky_Test (Condition_t *, Xtr_t *, Xtr_t *, 
                       Array_t *);
/*
static Boolean_t     SCond_Carbonium_Test (Condition_t *, Xtr_t *, Xtr_t *, 
*/
static Zadehan_t     SCond_Carbonium_Test (Condition_t *, Xtr_t *, Xtr_t *, 
                       Array_t *);
static Boolean_t     SCond_Disc_Coneq_Test (Condition_t *, Xtr_t *, Xtr_t *, 
                       Array_t *);
static Boolean_t     SCond_Disc_Streq_Test (Condition_t *, Xtr_t *, Xtr_t *,
                       Array_t *);
static Boolean_t     SCond_Dist_Test (Condition_t *, Array_t *, Array_t *, 
                       Xtr_t *, Array_t *);
/*
static Boolean_t     SCond_Elecwd_Test (Condition_t *, Xtr_t *, Xtr_t *, 
*/
static Zadehan_t     SCond_Elecwd_Test (Condition_t *, Xtr_t *, Xtr_t *, 
                       Array_t *);
static Boolean_t     SCond_Fg_Cnt_Test (Condition_t *, Xtr_t *, Array_t *, 
                       U16_t);
static Boolean_t     SCond_Fg_Coneq_Or_Streq_Test (Condition_t *, Boolean_t, 
                       Xtr_t *, Array_t *, U16_t);
static Boolean_t     SCond_Fg_Xcess_Test (Condition_t *, Xtr_t *, Array_t *, 
                       U16_t);
/*
static Boolean_t     SCond_Lvngroup_Test (Condition_t *, Array_t *, Array_t *,
*/
static Zadehan_t     SCond_Lvngroup_Test (Condition_t *, Array_t *, Array_t *,
                       Xtr_t *, Array_t *);
/*
static Boolean_t     SCond_Migratap_Test (Condition_t *, Array_t *, Array_t *, 
*/
static Zadehan_t     SCond_Migratap_Test (Condition_t *, Array_t *, Array_t *, 
                       Xtr_t *, Array_t *);
static Boolean_t     SCond_Molec_Test (Condition_t *, U16_t);
static Boolean_t     SCond_Pathlen_Test (Condition_t *, Array_t *, Array_t *, 
                       Xtr_t *, Array_t *);
static Boolean_t     SConn (U8_t, U8_t, Xtr_t *, Array_t *, Array_t *);
static Boolean_t     SConnect2 (U8_t, U8_t, Xtr_t *, Array_t *, Array_t *);
static Boolean_t     SConnect_Nodes (U16_t, U16_t, Tsd_t *, Boolean_t *);
static Boolean_t     SDist (U8_t, U8_t, U16_t, Array_t *, Array_t *,
                       Xtr_t *, Array_t *);
static Boolean_t     SDist_Or_SAtmdist (U8_t, U8_t, U16_t, Boolean_t,
                       Array_t *, Array_t *, Xtr_t *, Array_t *);
static Boolean_t     SElarsub (Xtr_t *, U16_t *, float *, S16_t *, S16_t *, 
                       S16_t *, Boolean_t);
static void          SElec (U8_t, float *, float *, Xtr_t *, Xtr_t *, 
                       Array_t *);
static float         SElecwd (U8_t, Xtr_t *, Xtr_t *, Array_t *);
static S8_t          SElecwd_Convert (float);
static Boolean_t     SExcess (Xtr_t *, Array_t *, U16_t, U8_t, U8_t, U16_t);
static S8_t          SFdist (U8_t, U8_t, Array_t *, Array_t *, Xtr_t *, 
                       Array_t *);
static void 	     SFind_Best_Lower (U8_t *, U16_t, Boolean_t, S16_t, 
                       S16_t *);
static void          SFind_Best_Upper (U8_t *, U16_t, Boolean_t, S16_t, 
                       Boolean_t *, S16_t *);
static void          SFind_Difference (U8_t *, S16_t, U16_t *, Boolean_t *);
static void          SFind_Vvatom (U8_t *, U8_t *);
static U8_t         *SFragnam (Xtr_t *, U16_t, U8_t);
static U8_t         *SFragnam_Or_SBlkfrgn (Xtr_t *, U16_t, U8_t, Boolean_t);
static void          SFree_Candidate_List (SCandidate_t *);
static void          SFree_Distance_Matrix (Array_t *, U16_t);
static void          SFree_Tree (STree_node_t *);
static SCandidate_t *SGet_Candidate_List (U8_t, U8_t, Array_t *, Array_t *, 
                       Xtr_t *, Array_t *);
static char         *SGet_Name (STree_node_t *, Boolean_t, Xtr_t *);
static void	     SGetout (U16_t, Tsd_t *, Array_t *);
static Path_List_t  *SGet_Path_List (Tsd_t *, U16_t *, Path_List_t *);
static Boolean_t     SIllegal_Combination (Xtr_t *);
static Boolean_t     SIllegal_Small_Rings (Xtr_t *);
static Boolean_t     SIllegal_Substructure (Xtr_t *);
static void          SIndefct (char *, float *, float *);
/*
static void          SLookahead (Boolean_t, Posttest_t *, U8_t *);
*/
static void          SLookahead (Zadehan_t, Posttest_t *, U8_t *);
static S8_t          SLvgroup (U8_t, U8_t, U8_t, Array_t *, Array_t *,
                       Xtr_t *, Array_t *);
static S8_t          SMigapt (U8_t, Array_t *, Array_t *, Xtr_t *, Array_t *);
static S16_t	     SN_Cetie (Xtr_t *, U16_t, Boolean_t);
static U16_t	     SNpaths (Tsd_t *, U16_t *, Path_List_t *);
static S16_t	     SN_Tie (Xtr_t *, U16_t, Boolean_t);
static Boolean_t     SOkring (Xtr_t *, U16_t,  Boolean_t);
static U16_t        *SPath (Tsd_t *, U16_t *, U16_t, U16_t *, Path_List_t *);
static STree_node_t *SPursue (U16_t, U16_t, U16_t, U16_t, U8_t *, Xtr_t *, Boolean_t);
static float	     SRatng (Xtr_t *, U16_t, Boolean_t);
static void	     SReposition (U16_t, U16_t, Tsd_t *, Array_t *);
static U16_t         SRing_System_Number (Xtr_t *, U16_t);
static Boolean_t     SRing_Tests_Passed (Xtr_t *, U16_t);
static Boolean_t     SSediscn (Xtr_t *, U8_t, Array_t *, Xtr_t *, Array_t *);
static Boolean_t     SSetup_Ar (Xtr_t *, U16_t, float *, S16_t *, S16_t *, 
                       S16_t *, Boolean_t);
static U16_t	     SSmallest_Common_Ring_Size (Xtr_t *, U16_t, U16_t);
static Boolean_t     SSmall_Ring (Xtr_t *, U8_t, U8_t);
static void          SSort_Tree (STree_node_t *, Boolean_t, Xtr_t *);
static S16_t	     SStdng (Xtr_t *, U16_t, Boolean_t);
static void	     SStore_Paths (Tsd_t *, Path_List_t *);
static void          STrace_Paths (U16_t, U16_t, Tsd_t *, Boolean_t *,
                       U16_t *, U16_t *, Path_List_t *);
static char         *SSub_String (const char *, U16_t, U16_t);
static void          SSwap_Tree_Nodes (STree_node_t *, ListElement_t *);
static Boolean_t     SThree_Unique (Tsd_t *, U16_t *, U16_t *, U16_t,
                       Path_List_t *);
static String_t      SVan_Der_Waal (U16_t);
static void          SVisit (U16_t, U8_t *, Array_t *, Array_t *, Xtr_t *);
static int           fuzzy_strcmp (U8_t *, U8_t *);
static Zadehan_t     Zadehan_AND (Zadehan_t, Zadehan_t);
static Zadehan_t     Zadehan_OR (Zadehan_t, Zadehan_t);
static Zadehan_t     Zadehan_EQ (Zadehan_t, Zadehan_t);
static Zadehan_t     Zadehan_XOR (Zadehan_t, Zadehan_t);
static char         *fuzzy_dump (Zadehan_t);
static char         *fuzzy_dump2 (U8_t);
static Zadehan_t     Zadehan_Normalized (Zadehan_t);

/* End of Routines Prototypes */

static U32_t global_schema; /* needed to make sense out of error messages! */
static const char *SResult[2] = { "F", "T" };
static const char *SOp[MX_OP] = { "Illegal", ">", ">=", "<", "<=", "=", "!=" };
static const char *SOutput[PT_MAXIDENT] = { "???", "Acidic", "???", "???", 
 "???", "???", "???", "Neutral", "???", "???", "???", "???", "???", "???", 
 "Basic","???", "???", "Methyl", "Ethyl", "Isopropyl", "t-Butyl", "???", 
 "???", "???", "???", "Arom1", "Arom2", "Arom3", "Arom4", "Arom5", "???", 
  "???", "???", "Non-migratory", "Alkyl", "Aryl", "Proton" };


static SAttribute_t   Attr_Info[CURRENT_NUM_OF_ATTRIBUTES] = 
{
  {"C=O",                    0, NULL},  /* CARBONYL */
  {"CH-1C=O",                0, NULL},  /* ENOLIZABLE CARBONYL */
  {"CH-1C*N",                0, NULL},  /* ACIDIC NITRITE */
  {"C<C",                    0, NULL},  /* AROMATIC RING */
  {"C=C",                    0, NULL},  /* ALKENE */ 
  {"C*C",                    0, NULL},  /* ALKYNE */ 
  {"C*N",                    0, NULL},  /* NITRILE */
  {"N=O-1=O",                0, NULL},  /* NITRO */
  {"S=O-1=O-1($1)",          0, NULL},  /* SULFONE */
  {"H",                      0, NULL},  /* HYDROGEN */
  {"C=O-1OC",                0, NULL},  /* ESTER (ACYL) */ 
  {"C=O-1NC-1C",             0, NULL},  /* TERTIARY AMIDE */
  {"C=CC=O",                 0, NULL},  /* ALPHA,BETA UNSATURATED CARBONYL */ 
  {"COC=O",                  0, NULL},  /* ESTER (ALKYL) */ 
  {"OC=O-1C",                0, NULL},  /* O ESTER */
  {"F",                      0, NULL},  /* FLUORINE */
  {"(CL)",                   0, NULL},  /* CHLORINE */ 
  {"(BR)",                   0, NULL},  /* BROMINE */
  {"I",                      0, NULL},  /* IODINE */ 
  {"S",                      0, NULL},  /* SULFUR */
  {"OH",                     0, NULL},  /* HYDROXYL */
  {"OC($2)-1($4)-1($6)",     0, NULL},  /* ETHER */
  {"NH-1H",                  0, NULL},  /* PRIMARY AMINE */ 
  {"NH-1C",                  0, NULL},  /* SECONDARY AMINE */ 
  {"NC-1C",                  0, NULL},  /* TERTIARY AMINE */ 
  {"C=O-1OH",                0, NULL},  /* CARBOXYLIC ACID */ 
  {"CH-1H-1H",               0, NULL},  /* METHYL */ 
  {"CH-1H-1C",               0, NULL},  /* METHYLENE */ 
  {"CC-1H-1C",               0, NULL},  /* METHINE */
  {"NC-1C-1C",               0, NULL},  /* QUATERNARY AMINE */
  {"SC-1C",                  0, NULL},  /* SULFONIUM */ 
  {"OS=O-1=O-1($1)",         0, NULL},  /* SULFONATE ESTER */ 
  {"OC<C",                   0, NULL},  /* PHENOXY */
  {"OC($2)-1($4)-1($6)",     0, NULL},  /* ALKOXY */  
  {"ON=O-1=O",               0, NULL},  /* NITRATE */
  {"OP=O-1($1)-1($2)",       0, NULL},  /* PHOSPHATE */ 
  {"OB($1)-1($2)",           0, NULL},  /* BORATE */
  {"C=CCH-1($2)-1($4)",      0, NULL},  /* VINYL WITH GAMMA HYDROGEN */ 
  {"S=O",                    0, NULL},  /* SULFOXIDE */
  {"P=O",                    0, NULL},  /* PHOSPHOROUS OXIDES */
  {"SC<C",                   0, NULL},  /* THIOPHENOL */ 
  {"SH",                     0, NULL},  /* MERCAPTAN */ 
  {"SC",                     0, NULL},  /* SULFIDE */ 
  {"P($2)-1($4)-1($6)",      0, NULL},  /* PHOSPHINE */
  {"P=O-1OC-2OC",            0, NULL},  /* PHOSPHONATE ESTER AT P */
  {"(SI)C-1C-1C",            0, NULL},  /* TRIALKYLSILYL */
  {"C<N",                    0, NULL},  /* ALPHA-PYRIDYL */
  {"C<C<N",                  0, NULL},  /* BETA-PYRIDYL */ 
  {"C<C<C<N",                0, NULL},  /* GAMMA-PYRIDYL */ 
  {"C=CC>CO/0",              0, NULL},  /* ALPHA-FURYL */ 
  {"C=CC>CS/0",              0, NULL},  /* ALPHA-THIENYL */
  {"C=CC>CN/0",              0, NULL},  /* ALPHA-PYRRYL */  
  {"CC>C>($3)-1OC($1)-1=/0", 0, NULL},  
                                 /* BETA-FURYL, ALPHA,ALPHA-DISUBSTITUTED */
  {"CC>C>($3)-1SC($1)-1=/0", 0, NULL},  
                                 /* BETA-THIENYL, ALPHA,ALPHA-DISUBSTITUTED */
  {"CC>C>($3)-1NC($1)-1=/0", 0, NULL},  
                                 /* BETA-PYRRYL, ALPHA,ALPHA-DISUBSTITUTED */
  {"C<C<C<COH-2<C<C</0",     0, NULL},  /* PARA-HYDROXYPHENYL */
  {"C<C<C<CN-1<C<C</0",      0, NULL},  /* PARA-AMINOPHENYL */
  {"C<COH-2<C<C<C<C</0",     0, NULL},  /* ORTHO-HYDROXYPHENYL */  
  {"C<CN-1<C<C<C<C</0",      0, NULL},  /* ORTHO-AMINOPHENYL */ 
  {"C(#J)-1(#J)-1(#J)",      0, NULL},  /* TRIHALOMETHYL */
  {"CC-1C-1C-1",             0, NULL},  /* QUATERNARY CARBON */ 
  {"COC/0",                  0, NULL},  /* OXIRANE */
  {"NC>CC>C/0",              0, NULL},  /* 1-PYRRYL */
  {"NC>NC>C/0",              0, NULL},  /* 1-IMIDAZOLYL */
  {"NN>CC>C/0",              0, NULL},  /* 1-PYRAZOLYL */
  {"CH-1H",                  0, NULL}   /* METHYL OR METHYLENE */
};


/* Array for illegal structure numbers, used in SIllegal_Substructure */


static const U16_t      illegal_nums[NUM_OF_ILLEGAL_STRUCTURES] = 
                          {
                            48,    
                            246,
                            247,
                            248,
                            249,
                            250,
                            252,
                            253,
                            254,
                            255,
                            257,
                            258,
                            259,
                            260,
                            262,
                            263,
                            264,
                            265,
                            266,
                            267,
                            268,
                            269,
                            271,
                            272,
                            273,
                            274,
                            275,
                            270,
                            549,
                            880,
                            881,
                            882
                           }; /* ??? */


/* Array for pairs of illegal structure numbers, used in SIllegal_Combination */

static const U16_t      illegal_pairs[NUM_OF_ILLEGAL_PAIRS][2] = 
           {
                 {20,    2},
                 {21,    2},
                 {20,    9},
                 {21,    9},
                 {20,   12},
                 {21,   12},
                 {20,    1},
                 {21,    1},
                 {20,   65},
                 {21,   65},
                 {20,   96},
                 {21,   96},
                 {20,   85},
                 {21,   85},
                 {20,  114},
                 {21,  114},
                 {20,  118},
                 {21,  118},
                 {20,  121},
                 {21,  121},
                 {20,  119},
                 {21,  119},
                 {20,  123},
                 {21,  123},
                 {20,  124},
                 {21,  124},
                 {20,  126},
                 {21,  126},
                 {20,  136},
                 {21,  136},
                 {20,  137},
                 {21,  137},
                 {58,    1},
                 {58,  123},
                 {58,  124},
                 {58,  126},
                 {58,   85},
                 {58,   96},
                 {58,  119},
                 {58,   65},
                 {58,  136},
                 {65,   85},
                 {65,  136},
                 {65,   96},
                 {65,  114},
                 {65,  118},
                 {65,  119},
                 {65,   97},
                 {65,   95},
                 {79,    1},
                 {79,   85},
                 {79,   65},
                 {79,   95},
                 {79,   96},
                 {79,  123},
                 {79,  124},
                 {79,  126},
                 {79,  136},
                 {85,    1},
                 {85,   96},
                 {85,  118},
                 {85,  119},
                 {85,  123},
                 {85,  124},
                 {85,  126},
                 {85,  136},
                {118,  119},
                {118,  121},
                {118,  123}
           };

static const char *bond_precedence[] = {
             "",
             "1",
             "3",
             "4",
             "",
             "",
             "2"
};

static const char *importance[] = {
             "   ",
             "  1",
             "103",
             " 83",
             " 15",
             " 27",
             "  2",
             " 29",
             " 21",
             " 97",
             "102",
             " 86",
             " 36",
             " 71",
             " 28",
             "  5",
             "  9",
             " 12",
             " 58",
             " 98",
             " 65",
             " 60",
             " 54",
             " 59",
             " 57", 
             " 40",
             " 42",
             " 34",
             " 37",
             " 35",
             " 14",
             " 70",
             " 32",
             " 10",
             " 11",
             "  4",
             " 23",
             " 99",
             " 75",
             " 61",
             " 53",
             " 52",
             " 47",
             " 46",
             " 43",
             " 39",
             " 26",
             " 38",
             " 20",
             " 74",
             " 44",
             " 22",
             " 17",
             "  6",
             "  8",
             "101",
             " 85",
             " 77",
             " 79",
             " 82",
             " 81",
             " 80",
             " 78",
             " 76",
             " 64",
             " 72",
             " 56",
             " 55",
             " 67",
             " 73",
             " 63",
             " 62",
             " 48",
             " 31",
             " 30",
             " 33",
             " 24",
             " 19",
             " 18",
             " 16",
             "  7",
             " 66",
             " 41",
             " 45",
             " 25",
             " 13",
             "  3",
             "100",
             " 84",
             " 51",
             " 50",
             " 49",
             " 69", 
             " 68",
             " 96",
             " 95",
             " 94",
             " 93",
             " 92",
             " 91",
             " 90",
             " 89", 
             " 88",
             " 87"
};

static const char  *squeeze[] = {
              "",
              "1",
              "2",
              "3",
              "4",
              "5",
              "6",
              "7",
              "8"
};

static const char *vdw[] = {
             "",
             "120",
             "150",
             "150",
             "150",
             "150",
             "150",
             "150",
             "140",
             "135",
             "130",
             "190",
             "190",
             "190",
             "190",
             "190",
             "185",
             "180",
             "175",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "200",
             "195",
             "190",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "220",
             "215",
             "210"
};

/* ??? needed to be changed later */

static SFragment_t	frag_info[TABLE_LENGTH] =   
{

        /* #  95 */
	{ 
	  "1 97..",
	  0.43,
	  -0.34
	},

        /* #  98 */
        {
	  "1 32.1 971 971 97....",
    	  0.79,
	  0.24	  
	},

	/* #  97 */
	{
	  "1 32.1 121 121 12....",
	  0.67,
	  0.17	 
	},

	/* #  96 */
	{

	  "1 32.1  41  41  4....",
	  0.62,
	  0.16	
	},

	/* # 106 */
	{
	  "1 29.4 29..",
	  1.69,
	  0.36
	},

	/* # 107 */
	{
	  "1 29.3 29.3 29..",
	  0.30,
	  -0.13	
	},

	/* # 151 */
	{
	  "1 29.3 29.1  2.2  22  2.2  21 21.2  21  1.....",
	  0.24,
	  0.08	
	},

	/* # 146 */
	{
	  "1 29.3 29.1  2.2  22  2.2  21  1.2  21  1.....",
	  0.28,
	  0.13  
	},

	/* # 105 */
	{
	  "1 29.3 213 21...",
	  0.67,
   	  0.16
	},

	/* #     */
	{
	  "1 29.3 21..",
	  0.40,
	  0.16	
	},

	/* # 115 */
	{
	  "1 29.3  2.3 21..",
	  0.29,
	  -0.08 	
 	},

	/* # 116 */
	{
	  "1 29.3  2.3  9..",
	  0.51,
	  -0.09
	},

	/* # 114 */
	{
	  "1 29.3  2.1 121 12...",
	  0.23,
	  -0.08
	},
 
	/* # 149 */
	{
	  "1 29.3  2.1  21  1.2  22  2..2  21  1.2  21  1.....",
	  0.10,
	  -0.63
	},

	/* # 117 */
	{
	  "1 29.1 291  2.3 29.3 291 29.1 29.1 29.3 29.3  2.3 29.3 29....",
	  0.53,
	  0.05
	},

	/* # 120 */
	{
	  "1 29.1 291  2.3 29.3 291 21.1 29.1 29.1  1.3  2.3 29....",
	  0.40,
	  -0.04	
	},

	/* # 113 */
	{
	  "1 29.1 291  2.3 29.3 291 12.1 29.1 29..3  2.3 29...",
	  0.58,
	  0.07	
	},

	/* # 130 */
	{
	  "1 29.1 291  2.3 29.3 291  9.1 29.1 29.1  9.3  2.3 29.1  2....",
	  0.61,
	  0.07	
	},

	/* # 121 */
	{
	  "1 29.1 291  2.3 29.3 291  9.1 29.1 29.1  1.3  2.3 29....",
	  0.44,
	  0.05	
	},

	/* # 119 */
	{
	  "1 29.1 291  2.3 29.3 291  1.1 29.1 29..3  2.3 29...",
	  0.52,
	  0.02
	},

	/* #     */
	{
	  "1 29.1 291  2.1  21  2.1  11  11  1.1  11  11  1.1  11  11  1.",
	  0.16,
	  -0.88
	},

	/* #     */
        {
	  "1 29.1 291  2.1  21  1.1  11  11  1.1  11  11  1........",
	  0.16,
	  -0.88
	},

	/* #	 */
	{
	  "1 29.1 291  2.1  11  1.1  11  11  1......",
	  0.16,
	  -0.88	
	},

	/* # 	 */
 	{
	  "1 29.1 291  1.1  21  2..1  11  11  1.1  11  11  1.......",
	  0.17,
	  -0.71
	},

	/* #	 */
	{
	  "1 29.1 291  1.1  21  1..1  11  11  1.....",
	  0.17,
	  -0.71	
	},

	/* # 111 */
	{
	  "1 29.1 291  1.1  11  1....",
	  0.17,
	  -0.71
	},

	/* #	 */
	{
	  "1 29.1 211  2.1  1.1  11  11  1.....",
	  0.05,
	  -0.56	
	},

	/* # 109 */
	{
	  "1 29.1 211  1.1  1...",
	  0.06,
	  -0.40
	},

	/* # 	 */
	{
	  "1 29.1  91  2.3 213 211  2.1  11  11  1...1  11  11  1.......",
	  0.26,
	  -0.38
	},

	/* # 148 */
	{
	  "1 29.1  91  1.3 213 211  2....2  22  2.2  21  1.2  21  1.....",
	  0.21,
	  -0.18	
	},

	/* # 127 */
	{
	  "1 29.1  91  1.3 213 211  2....1  11  11  1....",
	  0.25,
	  -0.20	
	},

	/* # 	 */
	{
	  "1 29.1  21  21  2.1  11  11  1.1  11  11  1.1  11  11  1......",
	  0.89,
	  0.00
	},

	/* #	 */
	{
	  "1 29.1  21  2.4 29.1  11  11  1.....",
	  0.25,
	  -0.35
	},

	/* #	 */
	{
	  "1 29.1  21  2.3 211 29.1  11  11  1..1  11  1......",
	  0.03,
	  -0.45	
	},

	/* # 	 */
	{
	  "1 29.1  21  2.3 211  2.1  11  11  1..1  11  11  1.......",
	  0.29,
	  -0.44	
	},

	/* #	 */
	{
	  "1 29.1  21  2.3 211  1.1  11  11  1......",
	  0.24,
	  -0.40
	},
	
	/* # 154 */
	{
	  "1 29.1  21  2.2  22  2.2  22  2.2  21  1.2  21  1.2  21  1.2  ",
	  0.07,
	  -0.29
	},

	/* # 128 */
	{
	  "1 29.1  21  2.1 971 971 97.1 971 971 97.......",
	  0.34,
	  0.22
	},

	/* # 135 */
	{
	  "1 29.1  21  2.1  11  11  1.1  11  11  1.......",
	  0.10,
	  -0.92
	},

	/* # 118 */
	{
	  "1 29.1  21  1.4 29...",
	  0.26,
	  -0.18
	},

	/* # 122 */
	{
	  "1 29.1  21  1.3 291  9..1 29.1 29.3 29.3 29...",
	  0.33,
	  -0.11
	},

	/* # 140 */
	{
	  "1 29.1  21  1.3 211 29...1  21  1.1  21  11  1.....",
	  0.14,
	  -0.39
	},

	/* # 124 */
	{
	  "1 29.1  21  1.3 211 29...1  11  1...",
	  0.04,
	  -0.28
	},

	/* # 139 */
	{
	  "1 29.1  21  1.3 211 21...1  2.1  21  11  1....",
	  0.14,
	  -0.28
	},

	/* # 150 */
	{
	  "1 29.1  21  1.3 211  2...2  22  2.2  21  1.2  21  1.....",
	  0.11,
	  -0.23
	},

	/* # 129 */
	{
	  "1 29.1  21  1.3 211  2...1 971 971 97....",
	  0.36,
	  -0.21
	},

	/* # 131 */
	{
	  "1 29.1  21  1.3 211  2...1 121  11  1....",
	  0.23,
	  -0.25
	},

	/* # 143 */
	{
	  "1 29.1  21  1.3 211  2...1  21  21  1.1  11  11  1.1  11  11  ",
	  0.18,
	  -0.26  
	},

	/* # 132 */
	{
	  "1 29.1  21  1.3 211  2...1  11  11  1....",
	  0.28,
	  -0.16
	},

	/* # 123 */
	{
	  "1 29.1  21  1.3 211  1....",
	  0.25,
	  -0.23  
	},

	/* # 141 */
	{ 
	  "1 29.1  21  1.3  91 29...1  21  1.1  21  11  1.....",
	  0.38,
	  -0.28
	},

	/* # 125 */
	{
	  "1 29.1  21  1.3  91 29...1  11  1...",
	  0.23,
	  -0.05
	},

	/* # 133 */
	{
	  "1 29.1  21  1.3  91  2...1  11  11  1....",
	  0.27,
	  -0.13
	},

	/* # 147 */
	{
	  "1 29.1  21  1.2  22  2..2  21  1.2  21  1.2  21  1..2  21  1..",
	  -0.02,
	  -0.38
	},

	/* # 	 */
	{
	  "1 29.1  21  1.1  21  11  1..1  21  11  1...1  21  11  1......",
	  0.28,
	  -0.25
	},

	/* # 	 */
	{
	  "1 29.1  21  1.1  21  11  1..1  11  11  1......",
	  0.11,
	  -0.51
	},

	/* # 	 */
	{
	  "1 29.1  21  1.1  11  11  1.....",
	  0.11,
	  -0.74
	},

	/* # 108 */
	{
	  "1 29.1  11  1...",
	  0.02,
	  -0.68
	},

	/* # 232 */
	{
	  "1 28.1 971 971 97....",
	  0.47,
	  0.25  
	},

	/* # 231 */
	{
	  "1 28.1 121 121 12....",
	  0.44,
	  0.16
	},

	/* # 230 */
	{
	  "1 28.1  41  41  4....",
	  0.44,
	  0.17  
	},

	/* # 233 */
	{
	  "1 28.1  21  21  2.1  11  11  1.1  11  11  1.1  11  11  1......",
	  -0.04,
	  -0.04
	},

	/* #   1 */
	{
	  "1 27.1 211 21.1  1.1  1...",
	  -0.07,
	  0.18
	},

	/* # 174 */
	{
	  "1 21.1  9.3 213 211  2...2  22  2.2  21  1.2  21  1.....",
	  0.36,
	  0.00
	},

	/* # 163 */
	{
	  "1 21.1  9.3 213 211  2...1  11  11  1....",
	  0.39,
	  0.00
	},

	/* # 176 */
	{
	  "1 21.1  2.3 211  2..2  22  2.2  21  1.2  21  1.....",
	  0.23,
	  -0.08
	},

	/* # 165 */
	{
	  "1 21.1  2.3 211  2..1  11  11  1....",
	  0.41,
	  -0.07
	},
	
	/* # 173 */
	{
	  "1 21.1  2.2  22  2.2  21  1.2  21  1.2  21  1..2  21  1......",
	  0.34,
	  -0.35
	},

	/* # 158 */
	{
	  "1 21.1  2.1 971 971 97....",
	  0.38,
	  0.00
	},

	/* # 164 */
	{
	  "1 21.1  2.1 971 971  2...1 971 121  1....",
	  0.37,
 	  -0.06
	},

	/* # 159 */
	{
	  "1 21.1  2.1 971 971  1....",
	  0.35,
	  -0.14
	},

	/* # 169 */
	{
	  "1 21.1  2.1  21  21  1.1  11  11  1.1  11  11  1........",
	  0.30,
	  -0.72
	},

	/* # 171 */
	{
	  "1 21.1  2.1  21  11  1.1  21  11  1...1  21  11  1......",
	  0.25,
	  -0.56
	},
 
	/* # 170 */
	{
	  "1 21.1  2.1  21  11  1.1  21  11  1...1  11  11  1......",
	  0.22,
	  -0.45
	},

	/* # 167 */
	{
	  "1 21.1  2.1  21  11  1.1  11  11  1......",
	  0.22,
	  -0.44
	},	

	/* # 162 */
	{
	  "1 21.1  2.1  11  11  1....",
	  0.26,
	  -0.51
	},	
	
	/* # 156 */
	{
	  "1 21.1  1..",
	  0.29,
	  -0.64
	},

	/* #  94 */
	{
	  "1 12..",
	  0.41,
	  -0.15
	},

	/* # 228 */
	{
	  "1 11.1  2.4 29..",
	  0.58,
	  0.13
	},
	
	/* # 227 */
	{
	  "1 11.1  2.1 971 971 97....",
	  0.29,
	  0.12
	},

	/* # 229 */
	{
	  "1 11.1  2.1  11  11  1....",
	  0.13,
	  -0.12
	},

	/* # 226 */
	{
	  "1  9.3 291  2.1  9.1  11  11  1.3 213 211  2......2  22  2...",
	  0.62,
	  0.13
	},

	/* # 203 */
	{
	  "1  9.3 213 211 97....",
	  0.75,
	  0.22  
	},

 	/* # 208 */
	{
	  "1  9.3 213 211 29...1  11  1...",
	  0.41,
	  0.19
	},

	/* # 224 */
	{
	  "1  9.3 213 211  2...2  22  2.2  21  1.2  21  1.2  21  1..2  21",
	  0.56, 
	  0.18  
 	},

	/* # 211 */
	{
	  "1  9.3 213 211  2...1 971 971 97....",
	  0.73,
	  0.26  
 	},

	/* # 216 */
	{
	  "1  9.3 213 211  2...1 971 971  1....",
	  0.70, 
	  0.22  
 	},

	/* # 218 */
	{
	  "1  9.3 213 211  2...1  11  11  1....",
	  0.54, 
	  0.22  
 	},

	/* # 210 */
	{
	  "1  9.3 211  2..1 971 971 97....",
	  0.60, 
	  0.14  
 	},

	/* # 215 */
	{
	  "1  9.3 211  2..1 971 971  1....",
	  0.51, 
	  0.11  
 	},

	/* # 204 */
	{
	  "1  9.1 971 971 971 971 97......",
	  0.57, 
	  0.15  
 	},

	/* # 217 */
	{
	  "1  9.1 21.1  2.1  11  11  1....",
	  0.52, 
	  0.01  
 	},

	/* # 213 */
	{
	  "1  9.1  2.4 29..",
	  0.36, 
	  0.19	  
 	},

	/* # 221 */
	{
	  "1  9.1  2.3 211  2..1  11  11  1....",
	  0.36, 
	  0.11  
 	},

	/* # 212 */
	{
	  "1  9.1  2.1 971 971 97....",
	  0.35, 
	  0.18  
 	},

	/* # 220 */
	{
	  "1  9.1  2.1 971 971  2...1 971 971  1....",
	  0.34, 
	  0.16  
 	},

	/* # 214 */
	{
	  "1  9.1  2.1 971 971  1....",
	  0.30, 
	  0.09  
 	},

	/* # 222 */
	{
	  "1  9.1  2.1  21  11  1.1  11  11  1......",
	  0.23,
	  -0.50  
 	},

	/* # 219 */
	{
	  "1  9.1  2.1  11  11  1....",
	  0.20,
	  -0.50  
 	},

	/* # 207 */
	{
	  "1  9.1  1..",
	  0.28,
	  -0.50  
 	},

	/* # 100 */
	{
	  "1  7.1  2.1  11  11  1....",
	  0.54,
	  -0.40  
 	},

	/* # 103 */
	{
	  "1  6.3 213 21...",
	  0.63,
	  0.20  
 	},

	/* # 101 */
	{
	  "1  6..",
	  0.40,
	  -0.19  
 	},

	/* # 179 */
	{
	  "1  5.3 211 971 97....",
	  0.77,
	  0.18  
 	},

	/* # 195 */
	{
	  "1  5.3 211 211 21..1  2.1  2.1  21  11  1.1  21  11  1.1  21  ",
	  0.32,
	  0.20  
 	},

	/* # 189 */
	{
	  "1  5.3 211 211 21..1  2.1  2.1  21  11  1.1  21  11  1.1  11  ",
	  0.52,
	  0.12  
 	},

	/* # 186 */
	{
	  "1  5.3 211 211 21..1  2.1  2.1  11  11  1.1  11  11  1.......",
	  0.37,
	  0.19 
 	},

	/* # 177 */
	{
	  "1  5.3 211 121 12....",
	  0.93,
	  -0.42  
 	},

	/* # 200 */
	{
	  "1  5.3 211  21  2..2  22  2.2  22  2.2  21  1.2  21  1.2  21  ",
	  0.31,
	  0.24  
 	},

	/* # 199 */
	{
	  "1  5.3 211  21  2..1  21  11  1.1  21  11  1.1  21  11  1...1 ",
	  0.29,
	  0.23  
 	},

	/* # 181 */
	{
	  "1  5.3  91 121 12....",
	  0.84,
	  -0.39  
 	},

	/* # 202 */
	{
	  "1  5.3  91  21  2..2  22  2.2  22  2.2  21  1.2  21  1.2  21  ",
	  0.21,
	  0.27  
 	},

	/* # 180 */
	{
	  "1  5.1 971 97...",
	  0.12,
	  0.50  
 	},

	/* # 184 */
	{
	  "1  5.1 291 12.1  21  2..1  11  11  1.1  11  11  1.......",
	  0.30,
	  0.28  
 	},

	/* # 178 */
	{
	  "1  5.1 121 12...",
	  0.49,
	  0.16  
 	},

	/* # 201 */
	{
	  "1  5.1  21  2.2  22  2.2  22  2.2  21  1.2  21  1.2  21  1.2  ",
	  0.07,
	  0.12  
 	},

	/* # 187 */
	{
	  "1  5.1  21  2.1  11  11  1.1  11  11  1.......",
	  -0.08,
	  0.39  
 	},

	/* #   2 */
	{
	  "1  4..",
	  0.44,
	  -0.17  
 	},

	/* #   6 */
	{
	  "1  2.4 29..",
	  0.51,
	  0.19  
 	},

	/* #  85 */
	{
	  "1  2.4  2.1  2.2  22  2.2  21  1.2  21  1.....",
	  0.12,
	  0.05  
 	},

	/* #  37 */
	{
	  "1  2.4  2.1  2.1 971 971 97....",
	  0.36,
	  0.18  
 	},

	/* #     */
	{
	  "1  2.4  2.1  2.1  11  11  1....",
	  0.19,
	  0.05  
 	},

	/* #  21 */
	{
	  "1  2.4  2.1  1..",
	  0.19,
	  0.05  
 	},

	/* #  92 */
	{
	  "1  2.3 291 29.1  2.1  21  2.2  22  2.2  22  2.2  22  2.2  21 2",
	  0.15,
	  0.08  
 	},

	/* #  78 */
	{
	  "1  2.3 291 21.1  2.1  2.2  22  2.2  22  2.2  21 21.2  21  1.2 ",
	  0.28,
	  0.07  
 	},

	/* #  79 */
	{
	  "1  2.3 291  9.1  2.1  2.2  22  2.2  22  2.2  21  9.2  21  1.2 ",
	  0.25,
	  0.06  
 	},

	/* #     */
	{
	  "1  2.3 291  2.1 21.1  11  11  1.1  1.....",
	  0.24,
	  -0.14  
 	},

	/* # 	 */
	{
	  "1  2.3 291  2.1  2.1  11  11  1.2  22  2....2  21  1.2  21  1.",
	  0.31,
	  0.13  
 	},

	/* #  14 */
	{
	  "1  2.3 291  1.1 21..1  1..",
	  0.25,
	  -0.13  
 	},

	/* #  81 */
	{
	  "1  2.3 291  1.1  2..2  22  2.2  21  1.2  21  1.....",
	  0.31,
	  0.13  
 	},

	/* #  30 */
	{
	  "1  2.3 211 29..1  21  1.1  11  11  1.....",
	  0.34,
	  0.05  
 	},

	/* #  13 */
	{
	  "1  2.3 211 29..1  11  1...",
	  0.24,
	  0.14  
 	},

	/* #  93 */
	{
	  "1  2.3 211 21..1  2.1  21  21  1.2  22  2.2  22  2......",
	  0.27,
	  0.31  
 	},

	/* #  49 */
	{
	  "1  2.3 211 21..1  2.1  21  11  1.1  11  11  1......",
	  0.33,
	  0.15  
 	},

	/* #  28 */
	{
	  "1  2.3 211 21..1  2.1  11  11  1....",
	  0.33,
	  0.15  
 	},

	/* #   9 */
	{
	  "1  2.3 211 21..1  1..",
	  0.33,
	  0.15  
 	},

	/* #  80 */
	{
	  "1  2.3 211  2..2  22  2.2  21  1.2  21  1.2  21  1..2  21  1..",
	  0.30,
	  0.16  
 	},

	/* #  27 */
	{
	  "1  2.3 211  2..1  11  11  1....",
	  0.32,
	  0.20  
 	},

	/* #   8 */
	{
	  "1  2.3 211  1...",
	  0.31,
	  0.13  
 	},

	/* #  32 */
	{
	  "1  2.3  91 29..1  21  1.1  11  11  1.....",
	  0.27,
	  0.09  
 	},

	/* #  58 */
	{
	  "1  2.3  21  9.1  21  1.1  2.3  21  1..3  21  1.1  91  1..1  21",
	  0.10,
	  0.04  
 	},

	/* #     */
	{
	  "1  2.3  21  2.1 291  2.1  11  11  1.3 213 21.1  11  11  1.....",
	  0.33, 
	  -0.05  
 	},

	/* # 	 */
	{
	  "1  2.3  21  2.1 291  1.1  11  11  1.3 213 21.......",
	  0.33, 
	  -0.05	  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  2.1  11  11  1.4 29.1  11  11  1........",
	  0.26,
	  -0.07	  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  2.1  11  11  1.3 211 21.1  11  11  1.....",
	  0.24,
	  -0.19  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  2.1  11  11  1.3 211  2.1  11  11  1.....",
	  0.28,
	  -0.27  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  2.1  11  11  1.3 211  1.1  11  11  1.....",
	  0.27,
	  -0.12  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  2.1  11  11  1.2  22  2.1  11  11  1....2",
	  0.13,
	  -0.13  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  1.1  11  11  1.4 29......",
	  0.26,
	  -0.07  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  1.1  11  11  1.3 211 21......1  2..",
	  0.24, 
	  -0.19  
 	},

	/* #	  */
	{
	  "1  2.3  21  2.1  21  1.1  11  11  1.3 211  2......1  11  11  1",
	  0.28,
	  -0.27  
 	},

	/* # 	 */
	{
	  "1  2.3  21  2.1  21  1.1  11  11  1.3 211  1.......",
	  0.27,
	  -0.12  
 	},

	/* # 	 */
	{
	  "1  2.3  21  2.1  21  1.1  11  11  1.3  21  1.....1  21  2....",
	  0.13,
	  -0.13  
 	},

	/* # 	 */
	{
	  "1  2.3  21  2.1  21  1.1  11  11  1.1 971 971 97........",
	  0.19,
	  0.05  
 	},

	/* # 	 */
	{
	  "1  2.3  21  2.1  11  1.1  11  11  1......",
	  0.07,
	  -0.08  
 	},

	/* # 	 */
	{
	  "1  2.3  21  1.1 291  2..3 213 21.1  11  11  1......",
	  0.33,
	  -0.05  
 	},

	/* #  25 */
	{
	  "1  2.3  21  1.1 291  1..3 213 21....",
	  0.33,
	  -0.05  
 	},

	/* #  	 */
	{
	  "1  2.3  21  1.1  21  2..4 29.1  11  11  1.....",
	  0.26,
	  -0.07  
 	},

	/* # 	 */
	{
	  "1  2.3  21  1.1  21  2..3 211 21.1  11  11  1..1  2.....",
	  0.24,
	  -0.19  
 	},

	/* #     */
	{
	  "1  2.3  21  1.1  21  2..3 211  2.1  11  11  1..1  11  11  1...",
	  0.28,
	  -0.27  
 	},

	/* #	 */
	{
	  "1  2.3  21  1.1  21  2..3 211  1.1  11  11  1......",
	  0.27,
	  -0.12
 	},

	/* #	 */
	{
	  "1  2.3  21  1.1  21  2..3  21  1.1  11  11  1.1  21  2.......",
	  0.13,
	  -0.13  
 	},

	/* #  42 */
	{
	  "1  2.3  21  1.1  21  1..4 29...",
	  0.26,
	  -0.07  
 	},

	/* #  67 */
	{
	  "1  2.3  21  1.1  21  1..3 211 21...1  2..",
	  0.24,
	  -0.19  
 	},

	/* #	 */
	{
	  "1  2.3  21  1.1  21  1..3 211 21...1  1..",
	  0.26
	  -0.10  
 	},

	/* #  89 */
	{
	  "1  2.3  21  1.1  21  1..3 211  2...2  22  2...",
	  0.20,
	  -0.13  
 	},

	/* #  60 */
	{
	  "1  2.3  21  1.1  21  1..3 211  2...1  11  11  1....",
	  0.28,
	  -0.27  
 	},

	/* #  44 */
	{
	  "1  2.3  21  1.1  21  1..3 211  1....",
	  0.27,
	  -0.12  
 	},

	/* #  86 */
	{
	  "1  2.3  21  1.1  21  1..2  22  2..2  21  1.2  21  1.....",
	  0.06,
	  -0.12
 	},

	/* #  26 */
	{
	  "1  2.3  21  1.1  11  1....",
	  0.07,
	  -0.08
 	},

	/* #  72 */
	{
	  "1  2.2  22  2.2  21 97.2  21 97.2  21 97..2  21 97..2  21 97..",
	  0.30,
	  0.13  
 	},

	/* #  73 */
	{
	  "1  2.2  22  2.2  21 29.2  21 29.2  21  1.3 213 21.2  21  1.3 2",
	  0.24,
	  0.08  
 	},

	/* #  71 */
	{
	  "1  2.2  22  2.2  21 12.2  21 12.2  21 12..2  21 12..2  21 12..",
	  0.24,
	  0.02
 	},

	/* #  74 */
	{
	  "1  2.2  22  2.2  21  1.2  21  1.2  21  1..2  21  1..2  21  1..",
	  0.16,
	  -0.08
 	},

	/* #   5 */
	{
	  "1  2.1 971 971 97....",
	  0.38,
	  0.19
 	},

	/* #  57 */
	{
	  "1  2.1 971 971  2...1 971 971  2...1 971 971  2...1 971 971 97",
	  0.44,
	  0.11
 	},

	/* #  38 */
	{
	  "1  2.1 971  21  2..1 971 971 97.1 971 971 97.......",
	  0.30,
	  0.25
 	},

	/* # 	 */
	{
	  "1  2.1 291  21  2.1  21  1.1  11  11  1.1  11  11  1.1  11  11",
	  -0.01, 
	  0.03
 	},

	/* # 	 */
	{
	  "1  2.1 291  21  1.1  21  1.1  11  11  1..1  11  11  1........",
	  -0.01, 
	  0.01
 	},

	/* #	 */
	{
	  "1  2.1 291  21  1.1  11  1.1  11  11  1.......",
	  -0.01,
	  0.01
 	},

	/* #	 */
	{
	  "1  2.1 291  11  1.1  21  2...1  11  11  1.1  11  11  1.......",
	  0.00,
	  0.00
 	},

	/* #	 */
	{
	  "1  2.1 291  11  1.1  21  1...1  11  11  1.....",
	  0.00,
	  0.00
 	},

	/* #	 */
	{
	  "1  2.1 291  11  1.1  11  1.....",
	  0.00,
	  0.00
 	},

	/* # 	 */
	{
	  "1  2.1 281  21  2.1  21  21  2.1  11  11  1.1  11  11  1.1  11",
	  -0.16,
	  -0.04
 	},

	/* #	 */
	{
	  "1  2.1 281  21  1.1  21  21  2.1  11  11  1..1  11  11  1.1  1",
	  -0.16,
	  -0.06
 	},

	/* #  65 */
	{
	  "1  2.1 281  11  1.1  21  21  2...1  11  11  1.1  11  11  1.1  ",
	  -0.15,
	  -0.07
 	},

	/* #	 */
	{
	  "1  2.1 211  21  2.1  2.1  11  11  1.1  11  11  1.1  11  11  1.",
	  -0.01,
	  0.03
 	},

	/* #  39 */
	{
	  "1  2.1 211  21  2.1  1.1 971 971 97.1 971 971 97........",
	  0.28,
	  0.05
 	},

	/* #	 */
	{
	  "1  2.1 211  21  2.1  1.1  11  11  1.1  11  11  1........",
	  -0.01,
	  0.03
 	},

	/* #	 */
	{
	  "1  2.1 211  21  1.1  2.1  11  11  1..1  11  11  1.......",
	  -0.01,
	  0.01
 	},

	/* #	 */
	{
	  "1  2.1 211  21  1.1  1.1  11  11  1......",
	  -0.01,
	  0.01
 	},

	/* #	 */
	{
	  "1  2.1 211  11  1.1  2...3 211  2..1  11  11  1....",
	  0.00,
	  0.00
 	},

	/* #	 */
	{
	  "1  2.1 211  11  1.1  2...1  11  11  1....",
	  0.00,
	  0.00
 	},

	/* #  17 */
	{
	  "1  2.1 211  11  1.1  1....",
	  0.00,
	  0.00
 	},

	/* #   4 */
	{
	  "1  2.1 121 121 12....",
	  0.31,
	  0.05
 	},

	/* # 	 */
	{
	  "1  2.1 121  21  1..1  11  11  1.....",
	  0.09,
	  0.04
 	},

	/* #  11 */
	{
	  "1  2.1 121  11  1....",
	  0.10,
	  0.03
 	},

	/* #	 */
	{
	  "1  2.1  91  21  1.3 213 211  2.1  11  11  1....1 971 971 97...",
	  0.26,
	  0.07
 	},

	/* #	 */
	{
	  "1  2.1  91  21  1.1  2.1  11  11  1..1 971 971 97.......",
	  0.09,
	  0.07
 	},

	/* #  23 */
	{
	  "1  2.1  91  11  1.3 213 211  2.....1 971 971 97....",
	  0.27,
	  0.06
 	},

	/* #  22 */
	{
	  "1  2.1  91  11  1.1  2...1 971 971 97....",
	  0.10,
	  0.06
 	},

	/* #	 */
	{
	  "1  2.1  61  21  1..1  11  11  1.....",
	  0.09,
	  0.04
 	},

	/* #  12 */
	{
	  "1  2.1  61  11  1....",
	  0.09,
	  0.03
 	},

	/* #   3 */
	{
	  "1  2.1  41  41  4....",
	  0.27,
	  0.04
 	},

	/* #	 */
	{
	  "1  2.1  41  21  1..1  11  11  1.....",
	  0.09,
	  0.06
 	},

	/* #  10 */
	{
	  "1  2.1  41  11  1....",
	  0.10,
	  0.05
 	},

	/* #	 */
	{
	  "1  2.1  21  21  2.4 29.1  11  11  1.1  11  11  1........",
	  0.20,
	  -0.21
 	},

	/* #	 */
	{
	  "1  2.1  21  21  2.3 211 29.1  11  11  1.1  11  11  1..1  11  1",
	  0.18,
	  -0.19
 	},

	/* #	 */
	{
	  "1  2.1  21  21  2.3 211 21.1  11  11  1.1  11  11  1..1  1....",
	  0.17,
	  -0.18
 	},

	/* #	 */
	{
	  "1  2.1  21  21  2.3 211  2.1  11  11  1.1  11  11  1..1  11  1",
	  0.18,
	  -0.13
 	},

	/* #	 */
	{
	  "1  2.1  21  21  2.3 211  1.1  11  11  1.1  11  11  1.........",
	  0.18,
	  -0.13
 	},

	/* #	 */
	{
	  "1  2.1  21  21  2.2  22  2.1  11  11  1.1  11  11  1.2  21  1.",
	  -0.09, 
	  0.02
 	},

	/* #  64 */
	{
	  "1  2.1  21  21  2.1  11  11  1.1  11  11  1.1  11  11  1......",
	  -0.07,
	  -0.13
 	},

	/* #	 */
	{
	  "1  2.1  21  21  1.4 29.1  11  11  1......",
	  0.20,
	  -0.19
 	},

	/* #	 */
	{
	  "1  2.1  21  21  1.3 211 29.1  11  11  1...1  11  1......",
	  0.18,
	  -0.17
 	},

	/* #	 */
	{
	  "1  2.1  21  21  1.3 211 21.1  11  11  1...1  1.....",
	  0.17,
	  -0.16
 	},

	/* #	 */
	{
	  "1  2.1  21  21  1.3 211  2.1  11  11  1...1  11  11  1.......",
	  0.18,
	  -0.15
 	},

	/* #	 */
	{
	  "1  2.1  21  21  1.3 211  1.1  11  11  1.......",
	  0.18,
	  -0.15
 	},

	/* #	 */
	{
	  "1  2.1  21  21  1.2  22  2.1  11  11  1..2  21  1.2  21  1....",
	  -0.08,
	  -0.01
 	},

	/* #  91 */
	{
	  "1  2.1  21  21  1.1  21  21  1.1  21  21  1..1  21  11  1.1  2",
	  -0.12,
	  -0.02
 	},

	/* #	 */
	{
	  "1  2.1  21  21  1.1  21  11  1.1  21  11  1..1  21  11  1...1 ",
	  -0.05,
	  -0.14
 	},

	/* #  47 */
	{
	  "1  2.1  21  21  1.1  21  11  1.1  21  11  1..1  11  1...1  11 ",
	  -0.03, 
	  -0.19
 	},

	/* #  55 */
	{
	  "1  2.1  21  21  1.1  11  11  1.1  11  11  1........",
	  -0.05,
	  -0.10
 	},

	/* #  24 */
	{
	  "1  2.1  21  11  1.4 29....",
	  0.21,
	  -0.18
 	},

	/* #	 */
	{
	  "1  2.1  21  11  1.3 211 29....1  11  1...",
	  0.19,
	  -0.16
 	},

	/* #	 */
	{
	  "1  2.1  21  11  1.3 211 21....1  1..",
	  0.18,
	  -0.15
 	},

	/* #	 */
	{
	  "1  2.1  21  11  1.3 211  2....1  11  11  1....",
	  0.19,
	  -0.16
 	},

	/* #	 */
	{
	  "1  2.1  21  11  1.3 211  1.....",
	  0.19,
	  -0.16
 	},

	/* #  82 */
	{
	  "1  2.1  21  11  1.2  22  2...2  21  1.2  21  1.2  21  1..2  21",
	  -0.08, 
	  -0.01
 	},

	/* #  51 */
	{
	  "1  2.1  21  11  1.1  21  11  1...3 211 21....1  1..",
	  -0.02,
	  -0.05
 	},

	/* #  69 */
	{
	  "1  2.1  21  11  1.1  21  11  1...1  21  11  1...1  21  11  1..",
	  -0.06,
	  -0.09
 	},

	/* #  63 */
	{
	  "1  2.1  21  11  1.1  21  11  1...1  21  11  1...1  11  11  1..",
	  -0.06,
	  -0.11
 	},

	/* #  54 */
	{
	  "1  2.1  21  11  1.1  21  11  1...1  11  11  1......",
	  -0.06,
	  -0.08
 	},

	/* #  33 */
	{
	  "1  2.1  21  11  1.1  11  11  1......",
	  -0.05,
	  -0.10
 	},

	/* #  16 */
	{
	  "1  2.1  11  11  1....",
	  -0.04,
	  -0.13
 	},

	/* #  99 */
	{
	  "1  1..",
	  0.00,
	  0.00
	}	
};


static const Boolean_t type[] = {
		FALSE,			/* value never used */
		TRUE,			/* 1 */
		TRUE,			/* 2 */	      
		FALSE,			/* 3 */
		FALSE,			/* 4 */
		FALSE,			/* 5 */
		TRUE,			/* 6 */
		TRUE, 			/* 7 */
		TRUE,			/* 8 */
		TRUE,			/* 9 */
		TRUE, 			/* 10 */
		FALSE,			/* 11 */
		FALSE,			/* 12 */
		FALSE,			/* 13 */
		FALSE,			/* 14 */
		FALSE,			/* 15 */
		FALSE,			/* 16 */
		TRUE,			/* 17 */
		TRUE,			/* 18 */
		FALSE,			/* 19 */
		FALSE,			/* 20 */
		FALSE,			/* 21 */
		FALSE,			/* 22 */
		FALSE,			/* 23 */
		FALSE,			/* 24 */
		FALSE,			/* 25 */
		FALSE,			/* 26 */
		FALSE,			/* 27 */
		FALSE, 			/* 28 */
		FALSE,			/* 29 */
		FALSE,			/* 30 */
		FALSE,			/* 31 */
		FALSE,			/* 32 */
		FALSE,			/* 33 */
		FALSE,			/* 34 */
		TRUE,			/* 35 */
		TRUE,			/* 36 */
		FALSE,			/* 37 */
  		FALSE,			/* 38 */
		FALSE,			/* 39 */
		FALSE,			/* 40 */
		FALSE,			/* 41 */
		FALSE,			/* 42 */
		FALSE,			/* 43 */
		FALSE,			/* 44 */
		FALSE,			/* 45 */
		FALSE, 			/* 46 */
		FALSE,			/* 47 */
		FALSE,			/* 48 */
		FALSE,			/* 49 */
		FALSE,			/* 50 */
		FALSE, 			/* 51 */
		FALSE,			/* 52 */
		FALSE,			/* 53 */
		TRUE,			/* 54 */
		FALSE,			/* 55 */
		FALSE,			/* 56 */
		FALSE,			/* 57 */
		FALSE,			/* 58 */
		FALSE,			/* 59 */
		FALSE,			/* 60 */
		FALSE,			/* 61 */
		FALSE,			/* 62 */
		FALSE,			/* 63 */
		FALSE,			/* 64 */
		FALSE,			/* 65 */
		FALSE,			/* 66 */
		FALSE,			/* 67 */
		FALSE,			/* 68 */
		FALSE,			/* 69 */
		FALSE,			/* 70 */
		FALSE,			/* 71 */
		FALSE,			/* 72 */
		FALSE,			/* 73 */
		FALSE,			/* 74 */
		FALSE,			/* 75 */
		FALSE,			/* 76 */
		FALSE,			/* 77 */
		FALSE,			/* 78 */
		FALSE,			/* 79 */
		FALSE,			/* 80 */
		FALSE,			/* 81 */
		FALSE,			/* 82 */
		FALSE,			/* 83 */
		FALSE,			/* 84 */
		FALSE,			/* 85 */
		FALSE,			/* 86 */
		FALSE,			/* 87 */
		FALSE,			/* 88 */
		FALSE,			/* 89 */
		FALSE,			/* 90 */
		FALSE,			/* 91 */
		FALSE,			/* 92 */
		FALSE,			/* 93 */
		FALSE,			/* 94 */
		FALSE,			/* 95 */
		FALSE,			/* 96 */
		FALSE,			/* 97 */
		FALSE,			/* 98 */
		FALSE,			/* 99 */
		FALSE,			/* 100 */
		FALSE,			/* 101 */
		FALSE,			/* 102 */
		FALSE			/* 103 */
};


static const U8_t  inverse_importance[] = {
		0,			/* value never used */
		1,			/* 1 */
		6,			/* 2 */
		86,			/* 3 */
		35,			/* 4 */
		15,			/* 5 */
		53,			/* 6 */
		80,			/* 7 */
		54,			/* 8 */
		16,			/* 9 */
		33,			/* 10 */
		34,			/* 11 */
		17,			/* 12 */
		85,			/* 13 */
		30,			/* 14 */
		4,			/* 15 */
		79,			/* 16 */
		52,			/* 17 */
		78,			/* 18 */
		77,			/* 19 */
		48,			/* 20 */
		8,			/* 21 */
		51,			/* 22 */
		36,			/* 23 */
		76,			/* 24 */
		84,			/* 25 */
		46,			/* 26 */
		5,			/* 27 */
		14,			/* 28 */
		7,			/* 29 */
		74,			/* 30 */
		73,			/* 31 */
		32,			/* 32 */
		75,			/* 33 */
		27,			/* 34 */
		29,			/* 35 */
		12,			/* 36 */
		28,			/* 37 */
		47,			/* 38 */
		45,			/* 39 */
		25,			/* 40 */
		82,			/* 41 */
		26,			/* 42 */
		44,			/* 43 */
		50,			/* 44 */
		83,			/* 45 */
		43,			/* 46 */
		42,			/* 47 */
		72,			/* 48 */
		91,			/* 49 */
		90,			/* 50 */
		89,			/* 51 */
		41,			/* 52 */
		40,			/* 53 */
		22,			/* 54 */
		67,			/* 55 */
		66,			/* 56 */
 		24,			/* 57 */
		18,			/* 58 */
		23,			/* 59 */
		21,			/* 60 */
		39,			/* 61 */
		71,			/* 62 */
		70,			/* 63 */
		64,			/* 64 */
		20,			/* 65 */
		81,			/* 66 */
		68,			/* 67 */
		93,			/* 68 */
		92,			/* 69 */
		31,			/* 70 */
		13,			/* 71 */
		65,			/* 72 */
		69,			/* 73 */
		49,			/* 74 */
		38,			/* 75 */
		63,			/* 76 */
		57,			/* 77 */
		62,			/* 78 */
		58,			/* 79 */
		61,			/* 80 */
		60,			/* 81 */
		59,			/* 82 */
		3,			/* 83 */
		88,			/* 84 */
		56,			/* 85 */
		11,			/* 86 */
		103,			/* 87 */
		102,			/* 88 */
		101,			/* 89 */
		100,			/* 90 */
		99,			/* 91 */
		98,			/* 92 */
		97,			/* 93 */
		96,			/* 94 */
		95,			/* 95 */
		94,			/* 96 */
		9,			/* 97 */
		19,			/* 98 */
		37,			/* 99 */
		87,			/* 100 */
		55,			/* 101 */
		10,			/* 102 */
		2			/* 103 */
};

main ()
{
  FILE *f;
  char line[100];

  f=fopen("/cygdrive/d/BULKSLN.TXT","r");
  while(fgets(line,99,f))
  {
    while(line[strlen(line)-1]<=' ') line[strlen(line)-1]=0;
    printf("%s: %s\n",line,SBulksln(line));
  }
  fclose(f);
}

/******************************************************************************
*
*  Function Name :       Destroy_SAtable_Attr_Xtr 
*
*    This routine  deallocates the memory allocated for the attribute xtr_t 
*    molecules in xtr_p_array. It should be called right before exiting
*    from the system.
*
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

void  Destroy_SAtable_Attr_Xtr 
  (
  void
  )
{
  U8_t           i;

  for (i = 0; i < CURRENT_NUM_OF_ATTRIBUTES; i++)
    Xtr_Destroy (Attr_Info[i].xtr_p);

  return;
}

/******************************************************************************
*
*  Function Name :       Destroy_Substituent_Table 
*
*    This routine  deallocates the memory allocated for frag_names in frag_info
*    structure. It should be called right before exiting from the system.
*
*    This table currently uses constant strings!
*
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

void  Destroy_Substituent_Table 
  (
  void
  )
{
/*
  U16_t				i;

  for (i = 0; i < TABLE_LENGTH; i++)
    Mem_Dealloc (frag_info[i].frag_name, strlen (frag_info[i].frag_name) + 1, 
      GLOBAL);  
*/

  return;
}

/******************************************************************************
*
*  Function Name :               Posttest_Check 
*
*    This routine performs the after-the-match reaction library tests. It
*    tells whether or not the transform passes these tests and adjusts the
*    ease, yield, and confidence values for the reaction as determined by 
*    the results of these tests.
*
*  Used to be :
*    
*    postran
*
*  Implicit Inputs :
*
*    Assume the subgxtr_p xtr's have the canonical sling stored in name.
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

Boolean_t  Posttest_Check
  (
  Xtr_t                        *target_p,
  Xtr_t                        *xtr_p,
  Xtr_t                        *goal_xtr_p,
  Array_t                      *subgxtr_p,
  Array_t                      *canonxtr_p,
  Array_t		       *tsdmaps_p,
  Array_t                      *match_p,
  Array_t                      *subgmap_p,
  U16_t                         num_xtrs,
  U16_t                         num_conjuncts,
  U32_t                         schema,
  S16_t                        *ease_adj_p,
  S16_t                        *yield_adj_p,
  S16_t                        *conf_adj_p,
  SShotInfo_t                  *sshotinfo_p
  )
{
  Posttest_t                   *post_p;
  React_Record_t               *rxn_p;
  Xtr_t                        *cxtr_p;           /* Canonical xtr from name */
  Array_t			condition_values;
  Array_t			constant;
  Array_t                  	dist_matrix;
  Array_t			test_values;
  U8_t                          i;
  U8_t                          insym;
  U8_t				matchsize;
  U8_t                          j;
  U8_t                          num_of_posttests;
  U8_t                          num_of_conditions;
  U8_t                          stack_top;
/*
  Boolean_t                     stack[MAX_POST_LENGTH];
  Boolean_t                     any_test_passed;
*/
  Zadehan_t                     stack[MAX_POST_LENGTH];
  Zadehan_t                     any_test_passed, true, false, fuzzy_temp;
  Boolean_t			compok;
/*
  Boolean_t                     finish_test;
*/
  Zadehan_t                     finish_test;
  Boolean_t			next_test;
  Boolean_t                	pass;
  React_Record_t               *react_p;
  React_Head_t                 *head_p;
  S16_t                         schema_ease, schema_yld, schema_conf;
  int                           fintest_numerator, fintest_numerator_delta;
  char                          fdval[2][100];

  react_p = React_Schema_Handle_Get (schema);
  head_p = React_Head_Get (react_p);
  schema_ease = React_Head_Ease_Get (head_p);
  schema_yld = React_Head_Yield_Get (head_p);
  schema_conf = React_Head_Confidence_Get (head_p);
printf("Posttest_Check\n");

  true.fuzzy_components.TF = 1;
  true.fuzzy_components.conf = 100;
  false.fuzzy_components.TF = 0;
  false.fuzzy_components.conf = 100;

  pass = TRUE;

  global_schema = schema;

  rxn_p = React_Schema_Handle_Get (schema); 

  matchsize = React_Head_GoalSize_Get (React_Head_Get (rxn_p));

  num_of_posttests = React_Head_NumTests_Get (React_Head_Get (rxn_p));
  num_of_conditions = React_Head_NumConditions_Get (React_Head_Get (rxn_p));

  *ease_adj_p = 0;
  *yield_adj_p = 0;
  *conf_adj_p = 0;

/* if (num_of_conditions == 0) no posttest exists for schema - WRONG!! Conditions can exist w/o tests!!! */
  if (num_of_posttests == 0) /* no posttest exists for schema */
    {
      for (i = 0; pass == TRUE && (i < num_xtrs); i++)
        {
/*
        cxtr_p = (Xtr_t *) Array_1d32_Get (canonxtr_p, i);
*/
        cxtr_p = (Xtr_t *) Array_1d32_Get (subgxtr_p, i);
/*
        if (cxtr_p != NULL)
*/
        if (Xtr_NumAtoms_Get (cxtr_p) > 1)
          pass = SCompok (cxtr_p);
        }

    if (sshotinfo_p != NULL)
      {
      SShotInfo_AllPostT_Put (sshotinfo_p, TRUE);
      SShotInfo_CompOk_Put (sshotinfo_p, pass);
      }

    return (pass);          
    }

  SBuild_Constant (xtr_p, goal_xtr_p, &constant, match_p, matchsize);
#ifdef _MIND_MEM_
in_subgenr(-811211);
  mind_Array_1d_Create ("&dist_matrix", "posttest{1}", &dist_matrix, Xtr_NumAtoms_Get (goal_xtr_p), ADDRSIZE);
  Array_Set (&dist_matrix, (U32_t) NULL);

/*
  mind_Array_1d_Create ("&test_values", "posttest{1}", &test_values, num_of_posttests, BITSIZE);
*/
  mind_Array_1d_Create ("&test_values", "posttest{1}", &test_values, num_of_posttests, BYTESIZE);
  mind_Array_1d_Create ("&condition_values", "posttest{1}", &condition_values, num_of_conditions, BYTESIZE);
#else
  Array_1d_Create (&dist_matrix, Xtr_NumAtoms_Get (goal_xtr_p), ADDRSIZE);
  Array_Set (&dist_matrix, (U32_t) NULL);

/*
  Array_1d_Create (&test_values, num_of_posttests, BITSIZE);
*/
  Array_1d_Create (&test_values, num_of_posttests, BYTESIZE);
  Array_1d_Create (&condition_values, num_of_conditions, BYTESIZE);
#endif
  Array_Set (&condition_values, UNDEFINED);

/*
  any_test_passed = FALSE;
  finish_test = FALSE;
*/
  any_test_passed = false;
  finish_test = false;
  fintest_numerator = 100;
 
  i = 0;
#ifdef _MIND_MEM_
in_subgenr(-811212);
#endif
  while ( (i < num_of_posttests) && pass && !(finish_test.fuzzy_components.TF && finish_test.fuzzy_components.conf == 100))
    {
#ifdef _FDEBUG_
printf("test %d:\n\tfinish_test=%s",i+1,fuzzy_dump(finish_test));
#endif
    if (finish_test.fuzzy_components.conf < 100)
    {
      if (finish_test.fuzzy_components.TF) fintest_numerator_delta = finish_test.fuzzy_components.conf - 100;
      else fintest_numerator_delta = -finish_test.fuzzy_components.conf;
      fintest_numerator += fintest_numerator_delta;
      if (fintest_numerator < 0) fintest_numerator = 0;
    }
    post_p =  React_Test_Get (rxn_p, i);
    stack_top = 0;
    
    j = 0;
    next_test = FALSE;
#ifdef _MIND_MEM_
in_subgenr(-811213);
#endif
    while (j < Post_Length_Get (post_p) && next_test == FALSE)
      {
#ifdef _FDEBUG_
printf("\n1) stack[1]=%s\n\n",fuzzy_dump(stack[1]));
#endif
      insym = Post_Op_Get (post_p, j);

      if (insym < PT_TEST_ADD)                 /* C condition */
        {
        if (insym >= num_of_conditions)
          fprintf (stderr, "Posttest Error: Condition number is too large!\n");

        if (Array_1d8_Get (&condition_values, insym) == UNDEFINED)
        {
#ifdef _FDEBUG_
printf("UNDEFINED\n");
#endif
          Array_1d8_Put (&condition_values, insym,  
               SCondition (React_Condition_Get (rxn_p, insym), target_p,
                           subgxtr_p, num_xtrs, subgmap_p, num_conjuncts,
                           xtr_p, goal_xtr_p, &dist_matrix, &constant, match_p,
                           tsdmaps_p, matchsize));           
        }
#ifdef _FDEBUG_
printf("\tC%02d: %s->", insym + 1, fuzzy_dump2 (Array_1d8_Get (&condition_values, insym)));
#endif
      
        Zadehan_Byteval (stack[stack_top]) = Array_1d8_Get (&condition_values, insym); 
#ifdef _FDEBUG_
printf("stack[%d]: %s\n", stack_top, fuzzy_dump (stack[stack_top]));
#endif
        stack_top++;
      }
      else if (insym == OP_NOT)
/*
        stack[stack_top - 1] = !(stack[stack_top - 1]);
*/
{
#ifdef _FDEBUG_
printf("\tNOT [s%d=%s]->", stack_top-1, fuzzy_dump (stack[stack_top - 1]));
#endif
        stack[stack_top - 1].fuzzy_components.TF = !(stack[stack_top - 1].fuzzy_components.TF);
#ifdef _FDEBUG_
printf("stack[%d]->%s\n", stack_top - 1, fuzzy_dump (stack[stack_top - 1]));
#endif
}
      else if (insym == OP_AND)
      {
/*
        stack[stack_top - 2] = stack[stack_top - 2] && stack[stack_top - 1];
*/
#ifdef _FDEBUG_
printf("\n1.3) stack[1]=%s\n\n",fuzzy_dump(stack[1]));
#endif
strcpy(fdval[0], fuzzy_dump (stack[stack_top - 2]));
strcpy(fdval[1], fuzzy_dump (stack[stack_top - 1]));
#ifdef _FDEBUG_
printf("\t[s%d=%s] AND [s%d=%s]->", stack_top-2, fdval[0], stack_top-1, fdval[1]);
#endif
        stack[stack_top - 2] = Zadehan_AND (stack[stack_top - 2], stack[stack_top - 1]);
        stack_top--;
#ifdef _FDEBUG_
printf("stack[%d]->%s\n", stack_top - 1, fuzzy_dump (stack[stack_top - 1]));
printf("\n1.6) stack[1]=%s\n\n",fuzzy_dump(stack[1]));
#endif
      }
      else if (insym == OP_OR)
      {
/*
        stack[stack_top - 2] = stack[stack_top - 2] || stack[stack_top - 1];
*/
strcpy(fdval[0], fuzzy_dump (stack[stack_top - 2]));
strcpy(fdval[1], fuzzy_dump (stack[stack_top - 1]));
#ifdef _FDEBUG_
printf("\t[s%d=%s] OR [s%d=%s]->", stack_top-2, fdval[0], stack_top-1, fdval[1]);
#endif
        stack[stack_top - 2] = Zadehan_OR (stack[stack_top - 2], stack[stack_top - 1]);
        stack_top--;
#ifdef _FDEBUG_
printf("stack[%d]->%s\n", stack_top - 1, fuzzy_dump (stack[stack_top - 1]));
#endif
      }
      else if (insym == OP_NOPASS)
      {
/*
        if (any_test_passed == TRUE) 
          stack[stack_top] = FALSE;
        else
          stack[stack_top] = TRUE;
*/
        stack[stack_top] = any_test_passed;
        stack[stack_top].fuzzy_components.TF = !stack[stack_top].fuzzy_components.TF;

        stack_top++;
#ifdef _FDEBUG_
printf("\tNOP: stack[%d]->%s\n", stack_top - 1, fuzzy_dump (stack[stack_top - 1]));
#endif

        if (Post_Length_Get (post_p) == 1) /* only OP_NOPASS */
/*
          any_test_passed = FALSE;
*/
          any_test_passed = false;
      }
      else if (insym == BOOLOP_EQ)
      {
/*
        stack[stack_top - 2] = (stack[stack_top - 2] == stack[stack_top - 1]);
*/
strcpy(fdval[0], fuzzy_dump (stack[stack_top - 2]));
strcpy(fdval[1], fuzzy_dump (stack[stack_top - 1]));
#ifdef _FDEBUG_
printf("\ts%d=%s] EQ s%d=%s]->", stack_top-2, fdval[0], stack_top-1, fdval[1]);
#endif
        stack[stack_top - 2] = Zadehan_EQ (stack[stack_top - 2], stack[stack_top - 1]);
        stack_top--;
#ifdef _FDEBUG_
printf("stack[%d]->%s\n", stack_top - 1, fuzzy_dump (stack[stack_top - 1]));
#endif
      }
      else if (insym == BOOLOP_XOR)
      {
/*
        stack[stack_top - 2] = (stack[stack_top - 2] != stack[stack_top - 1]);
*/
strcpy(fdval[0], fuzzy_dump (stack[stack_top - 2]));
strcpy(fdval[1], fuzzy_dump (stack[stack_top - 1]));
#ifdef _FDEBUG_
printf("\t[s%d=%s] XOR [s%d=%s]->", stack_top-2, fdval[0], stack_top-1, fdval[1]);
#endif
        stack[stack_top - 2] = Zadehan_XOR (stack[stack_top - 2], stack[stack_top - 1]);
        stack_top--;
#ifdef _FDEBUG_
printf("stack[%d]->%s\n", stack_top - 1, fuzzy_dump (stack[stack_top - 1]));
#endif
      }
      else 
        {
        insym -= PT_TEST_ADD;

         if (insym >= i)
           {
/*
           Array_1d1_Put (&test_values, i, FALSE);
*/
           Array_1d8_Put (&test_values, i, Zadehan_Byteval (false));
#ifdef _FDEBUG_
printf("\tT%02d: %s\n", i + 1, fuzzy_dump2 (Array_1d8_Get (&test_values, i)));
#endif
           next_test = TRUE;
           }
         else
           {
           Zadehan_Byteval (stack[stack_top]) = Array_1d8_Get (&test_values, insym); 
#ifdef _FDEBUG_
printf("\tT%02d: %s\n", insym + 1, fuzzy_dump2 (Array_1d8_Get (&test_values, insym)));
#endif
           stack_top++;
           }
        }
#ifdef _FDEBUG_
printf("\n2) stack[1]=%s\n\n",fuzzy_dump(stack[1]));
#endif
      
      if (next_test == FALSE)
        {
        j++;
        SLookahead (stack[stack_top - 1], post_p, &j);
        }
#ifdef _FDEBUG_
printf("\n3) stack[1]=%s\n\n",fuzzy_dump(stack[1]));
#endif
      }  /* end of while (j) */
#ifdef _MIND_MEM_
in_subgenr(-811214);
#endif
    
    if (next_test != TRUE && stack_top == 1)
      Array_1d8_Put (&test_values, i, Zadehan_Byteval (stack[0]));
    else
/*
      Array_1d1_Put (&test_values, i, FALSE);
*/
      Array_1d8_Put (&test_values, i, Zadehan_Byteval (false));
#ifdef _FDEBUG_
printf("\t->T%02d: %s\n", i + 1, fuzzy_dump2 (Array_1d8_Get (&test_values, i)));
#endif

/*
    if (Array_1d1_Get (&test_values, i) == TRUE)
      {
      any_test_passed = TRUE;
*/
printf("posttest2: T%02d=%x\n",i+1,Array_1d8_Get(&test_values,i));
    Zadehan_Byteval (fuzzy_temp) = Array_1d8_Get (&test_values, i);
    if (fintest_numerator < 100) fuzzy_temp.fuzzy_components.conf = fintest_numerator * fuzzy_temp.fuzzy_components.conf / 100;
    if (Zadehan_Byteval (fuzzy_temp) == Zadehan_Byteval (true) || fuzzy_temp.fuzzy_components.conf < 100)
      {
      any_test_passed = Zadehan_OR (any_test_passed, fuzzy_temp);
#ifdef _FDEBUG_
printf("\tany_test_passed: %s\n", fuzzy_dump (any_test_passed));
#endif
       
      if (Post_Result_Get (post_p) == FALSE)
        {
/*
        pass = FALSE;
*/
        fuzzy_temp.fuzzy_components.TF = !fuzzy_temp.fuzzy_components.TF;
        if (fuzzy_temp.fuzzy_components.TF == 1)
        {
          fuzzy_temp.fuzzy_components.TF = 0;
          fuzzy_temp.fuzzy_components.conf = 100 - fuzzy_temp.fuzzy_components.conf;
        }
        pass = fuzzy_temp.fuzzy_components.conf < 100;
/* Since a crisp failure stops further testing, a fuzzy failure must be treated as a fuzzy "PASS AND STOP" */
        finish_test = true;
        finish_test.fuzzy_components.conf = fuzzy_temp.fuzzy_components.conf;
        if (sshotinfo_p != NULL)
          {
          if (pass == FALSE) SShotInfo_IthTRslt_Put (sshotinfo_p, i, SSV_PTEST_FAIL);
          else if (fuzzy_temp.fuzzy_components.conf == 0) SShotInfo_IthTRslt_Put (sshotinfo_p, i, SSV_PTEST_PASS);
          else SShotInfo_IthTRslt_Put (sshotinfo_p, i, Zadehan_Byteval (Zadehan_Normalized (fuzzy_temp)));
          }
#ifdef _FDEBUG_
printf("\teyc_adj: %d %d %d; sch_eyc: %d %d %d->\n",*ease_adj_p,*yield_adj_p,*conf_adj_p,schema_ease,schema_yld,schema_conf);
#endif
printf("\teyc_adj: %d %d %d; sch_eyc: %d %d %d->\n",*ease_adj_p,*yield_adj_p,*conf_adj_p,schema_ease,schema_yld,schema_conf);
        if (pass)
          {
          *ease_adj_p -= fuzzy_temp.fuzzy_components.conf * (schema_ease + *ease_adj_p) / 100;
          *yield_adj_p -= fuzzy_temp.fuzzy_components.conf * (schema_yld + *yield_adj_p) / 100;
          *conf_adj_p -= fuzzy_temp.fuzzy_components.conf * (schema_conf + *conf_adj_p) / 100;
          }
#ifdef _FDEBUG_
printf("\t\tfintest_numerator=%d; fuzzy_conf=%d; eyc_adj: %d %d %d\n",fintest_numerator,fuzzy_temp.fuzzy_components.conf,
*ease_adj_p,*yield_adj_p,*conf_adj_p);
#endif
printf("\t\tfintest_numerator=%d; fuzzy_conf=%d; eyc_adj: %d %d %d\n",fintest_numerator,fuzzy_temp.fuzzy_components.conf,
*ease_adj_p,*yield_adj_p,*conf_adj_p);
        }
      else
        {
        if (fuzzy_temp.fuzzy_components.TF == 0)
        {
          fuzzy_temp.fuzzy_components.TF = 1;
          fuzzy_temp.fuzzy_components.conf = 100 - fuzzy_temp.fuzzy_components.conf;
        }
#ifdef _FDEBUG_
printf("\teyc_adj: %d %d %d->\n",*ease_adj_p,*yield_adj_p,*conf_adj_p);
#endif
printf("\teyc_adj: %d %d %d->\n",*ease_adj_p,*yield_adj_p,*conf_adj_p);
        *ease_adj_p += fuzzy_temp.fuzzy_components.conf * Post_EaseAdj_Get (post_p) / 100;
        *yield_adj_p += fuzzy_temp.fuzzy_components.conf * Post_YieldAdj_Get (post_p) / 100;
        *conf_adj_p += fuzzy_temp.fuzzy_components.conf * Post_ConfidenceAdj_Get (post_p) / 100;
#ifdef _FDEBUG_
printf("\t\tfintest_numerator=%d; fuzzy_conf=%d; eyc_adj: %d %d %d\n",fintest_numerator,fuzzy_temp.fuzzy_components.conf,
*ease_adj_p,*yield_adj_p,*conf_adj_p);
#endif
printf("\t\tfintest_numerator=%d; fuzzy_conf=%d; eyc_adj: %d %d %d\n",fintest_numerator,fuzzy_temp.fuzzy_components.conf,
*ease_adj_p,*yield_adj_p,*conf_adj_p);
        if (sshotinfo_p != NULL)
          {
          if (Zadehan_Byteval (fuzzy_temp) == Zadehan_Byteval (true)) SShotInfo_IthTRslt_Put (sshotinfo_p, i, SSV_PTEST_PASS);
          else SShotInfo_IthTRslt_Put (sshotinfo_p, i, Zadehan_Byteval (Zadehan_Normalized (fuzzy_temp)));
          }
        }

      if (Post_Stop_Get (post_p) == TRUE)
/*
        finish_test = TRUE;
*/
        finish_test = Zadehan_OR (finish_test, fuzzy_temp);
      }
    else 
      {
      if (sshotinfo_p != NULL)
         SShotInfo_IthTRslt_Put (sshotinfo_p, i, SSV_PTEST_FALSE);
      }

    i++;   
    }  /* end of while (i) */
#ifdef _MIND_MEM_
in_subgenr(-811215);
#endif
 
  if (sshotinfo_p == NULL) 
    {
    for (i = 0; pass == TRUE && (i < num_xtrs); i++)
      {
/*
      cxtr_p = (Xtr_t *) Array_1d32_Get (canonxtr_p, i);
*/
      cxtr_p = (Xtr_t *) Array_1d32_Get (subgxtr_p, i);
/*
      if (cxtr_p != NULL)
*/
      if (Xtr_NumAtoms_Get (cxtr_p) > 1)
        pass = SCompok (cxtr_p);
      }
    }
  else
    {
    for (i = 0, compok = TRUE; compok == TRUE && (i < num_xtrs); i++)
      {
/*
      cxtr_p = (Xtr_t *) Array_1d32_Get (canonxtr_p, i);
*/
      cxtr_p = (Xtr_t *) Array_1d32_Get (subgxtr_p, i);
/*
      if (cxtr_p != NULL)
*/
      if (Xtr_NumAtoms_Get (cxtr_p) > 1)
        compok = SCompok (cxtr_p);
      }

    SShotInfo_AllPostT_Put (sshotinfo_p, pass);
    SShotInfo_CompOk_Put (sshotinfo_p, compok);
    pass = pass && compok;
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&test_values", "posttest", &test_values);
  mind_Array_Destroy ("&condition_values", "posttest", &condition_values);

  mind_Array_Destroy ("&constant", "posttest", &constant);
#else
  Array_Destroy (&test_values);
  Array_Destroy (&condition_values);

  Array_Destroy (&constant);
#endif
  SFree_Distance_Matrix (&dist_matrix, 
                          Xtr_NumAtoms_Get (goal_xtr_p));

  return (pass);
}

/******************************************************************************
*
*  Function Name :               SAdd_Name 
*
*    This routine adds the contribution of "tree_node_p" to "name".
*
*  Used to be :
*    
*    add_name
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static void  SAdd_Name 
  (
  String_t				 *name_p,
  STree_node_t                           *tree_node_p, 
  Boolean_t                              which_case, 
  Xtr_t                                  *xtr_p
  )
{
  String_t	                   next_name;

  if (which_case == FALSE)
    {
    String_Concat_c (name_p, bond_precedence[tree_node_p->bm]);
    String_Concat_c (name_p, 
         importance[Xtr_Attr_Atomid_Get (xtr_p, tree_node_p->node)]);
    }
  else
    {
    String_Concat_c (name_p, squeeze[tree_node_p->num_of_neighbors]);
    if (tree_node_p->num_of_neighbors == 1)
      {
      next_name = SVan_Der_Waal (Xtr_Attr_Atomid_Get (xtr_p,
        tree_node_p->node));
      String_Concat (name_p, next_name);
      String_Destroy (next_name);
      }
    }

  return ;
}

/******************************************************************************
*
*  Function Name :               SAdjmult 
*
*    This routine returns ???
*
*  Used to be :
*    
*    adjmult
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U16_t  SAdjmult 
  (
  Xtr_t      *xtr_p, 
  U16_t       node, 
  U8_t        degree
  )
{
  U16_t       bond;
  U16_t       mult;
  U8_t        i;

  mult = 0;
  for (i = 0; i < degree; i++)
    {
    switch (Xtr_Attr_NeighborBond_Get (xtr_p, node, i))
      {
      case BOND_NONE :
        bond = 0;
        break;
      
      case BOND_SINGLE :
        bond = 2;
        break;

      case BOND_DOUBLE :
        bond = 4;
        break;

      case BOND_TRIPLE :
        bond = 6;
        break;

      case BOND_QUADRUPLE :
        bond = 8;
        break;
 
      case BOND_VARIABLE :
        bond = 16;
        break;

      case BOND_RESONANT :
        bond = 3;
      }
    
    if (bond > 2 && Xtr_Attr_Atomid_Get (xtr_p, 
        Xtr_Attr_NeighborId_Get (xtr_p, node, i)) != CARBON)
      bond = 25;

    mult += bond;
  }

  return (mult >> 1);
}

/******************************************************************************
*
*  Function Name :               SAlkynsr 
*
*    This routine tests whether the alkyne formed in the subgoal will be in
*    a small ring.
*
*  Used to be :
*    
*    alkynsr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SAlkynsr 
  (
  U8_t 				node1,
  U8_t 				node2,
  Xtr_t			       *xtr_p,
  Array_t		       *match_p
  )
{
  U16_t                         atom1;
  U16_t                         atom2;
  U16_t                         smallest_size;

  atom1 = Array_1d16_Get (match_p, node1);
  atom2 = Array_1d16_Get (match_p, node2);
  smallest_size = SSmallest_Common_Ring_Size (xtr_p, atom1, atom2);
  if (smallest_size == 0) 
    return (FALSE);

  if (smallest_size < 8)
    return (TRUE);

  return (FALSE);
}

/******************************************************************************
*
*  Function Name :               SAllensr 
*
*    This routine tests whether the allene formed in the subgoal will be
*    in a small ring.
*
*  Used to be :
*    
*    allensr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SAllensr 
  (
  U8_t 				node1, 
  U8_t 				node2,
  Xtr_t			       *xtr_p,
  Array_t 		       *match_p
  )
{
  U16_t                         atom1;
  U16_t                         atom2;
  U16_t                         smallest_size;

  atom1 = Array_1d16_Get (match_p, node1);
  atom2 = Array_1d16_Get (match_p, node2);
  smallest_size = SSmallest_Common_Ring_Size (xtr_p, atom1, atom2);
  if (smallest_size == 0) 
    return (FALSE);

  if (smallest_size < 9)
    return (TRUE);

  return (FALSE);

}

/******************************************************************************
*
*  Function Name :               SArom_Fusion
*
*    This routine attempts to sort through substituents and approximate
*    them by means of a composite fragment.
*
*    Added a hack to free any strings created by strdup until a more 
*    reasonable approach can be coded.  - Krebsbach
*
*    Had to make major changes in order to get substring to work like the
*    PLI equivalent.  - Krebsbach
*
*  Used to be :
*    
*    arom_fusion
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/
static U8_t *SArom_Fusion 
  (
  char 	*frag_string_p,
  U16_t *effect_adjust
  )
{
  char          *sub_string;
  char          *temp_frag;
  String_t       save_string;
  String_t       save_string2;
  U16_t          temp_effect_adjust;
  U16_t          save_effect_adjust;
 
  temp_effect_adjust = 0;
  save_effect_adjust = 10000;
  save_string = String_Create (NULL, 0);

  temp_frag = SSub_String (frag_string_p, 5, INFINITY);
  frag_string_p = temp_frag;

  while (strncmp (frag_string_p, "2  2", 4) == 0)
    {
    save_string2 = String_Create (NULL, 0);
    temp_effect_adjust++;

    temp_frag = SSub_String (frag_string_p, 4, INFINITY);
#ifdef _MIND_MEM_
    mind_free ("frag_string_p", "posttest", frag_string_p);
#else
    Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
    frag_string_p = temp_frag;

    if (strncmp (frag_string_p, "1  1.", 5) != 0)
      {
      sub_string = SSub_String (frag_string_p, 0, 5);
      String_Concat_c (&save_string2, sub_string);
      save_effect_adjust = MIN (save_effect_adjust, temp_effect_adjust);
#ifdef _MIND_MEM_
      mind_free ("sub_string", "posttest", sub_string);
#else
      Mem_Dealloc (sub_string, 6, GLOBAL);
#endif
      }
  
    temp_frag = SSub_String (frag_string_p, 5, INFINITY);
#ifdef _MIND_MEM_
    mind_free ("frag_string_p", "posttest", frag_string_p);
#else
    Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
    frag_string_p = temp_frag;

    if (strlen(frag_string_p) < 1)
        {
#ifdef _MIND_MEM_
        mind_free ("frag_string_p", "posttest", frag_string_p);
#else
        Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
        frag_string_p = SSub_String ("1  1..", 0, INFINITY);
        }

    while (*frag_string_p == '.')
      {
      if (strlen(frag_string_p) > 1)
        {
        temp_frag = SSub_String (frag_string_p, 1, INFINITY);
#ifdef _MIND_MEM_
        mind_free ("frag_string_p", "posttest", frag_string_p);
#else
        Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
        frag_string_p = temp_frag;
        }

      else
        {
#ifdef _MIND_MEM_
        mind_free ("frag_string_p", "posttest", frag_string_p);
#else
        Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
        frag_string_p = SSub_String ("1  1..", 0, INFINITY);
        }
      }  /*  End of while */
    
    if ((strncmp (frag_string_p, "2  2", 4) != 0) &&
         (strncmp (frag_string_p, "1  1..", 6) != 0))
      {
      while (*frag_string_p != '.')
        { 
        sub_string = SSub_String (frag_string_p, 0, 1);
        String_Concat_c (&save_string, sub_string);
#ifdef _MIND_MEM_
        mind_free ("sub_string", "posttest", sub_string);

        temp_frag = SSub_String (frag_string_p, 1, INFINITY);
        mind_free ("frag_string_p", "posttest", frag_string_p);
#else
        Mem_Dealloc (sub_string, 2, GLOBAL);

        temp_frag = SSub_String (frag_string_p, 1, INFINITY);
        Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
        frag_string_p = temp_frag;
        }  /* End of while */

      while (*frag_string_p == '.')
        {
        String_Concat_c (&save_string, ".");

        if (strlen(frag_string_p) > 1)
          {
          temp_frag = SSub_String (frag_string_p, 1, INFINITY);
#ifdef _MIND_MEM_
          mind_free ("frag_string_p", "posttest", frag_string_p);
#else
          Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
          frag_string_p = temp_frag;
          }

        else
          {
#ifdef _MIND_MEM_
          mind_free ("frag_string_p", "posttest", frag_string_p);
#else
          Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif
          frag_string_p = SSub_String ("1  1..", 0, INFINITY);
          }
        }  /* End of while */
      }  /*  End of if  */
    
    String_Concat (&save_string, save_string2);
    String_Destroy (save_string2);
    }  /* End of while */

  if ((String_Length_Get (save_string) == 0) || 
      (strncmp (frag_string_p, "1  1..", 6) != 0) )
    String_Concat_c (&save_string, frag_string_p);

  if (save_effect_adjust < 3)
    temp_effect_adjust = 1;
  else
    temp_effect_adjust = 4;

  *effect_adjust = temp_effect_adjust;

#ifdef _MIND_MEM_
  mind_free ("frag_string_p", "ppsttest", frag_string_p);
#else
  Mem_Dealloc (frag_string_p, INFINITY, GLOBAL);
#endif

  return (String_Value_Get (save_string));
}

/******************************************************************************
*
*  Function Name :               SAsearch
*
*    This routine searches the compound molecule at node "candidate_node" for 
*    attribute "attribute".
*
*  Used to be :
*    
*    asearch
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/
static Boolean_t  SAsearch 
  (
  Xtr_t                         *xtr_p,
  U16_t                         attribute,
  U16_t                         candidate_node,
  Boolean_t                     which_dist,
  U16_t                         root,
  U8_t                          distance,
  Array_t                       *constant_p,
  Array_t                     	*dist_matrix_p
  )
{
  SDistance_t                          *dist_p;
  MatchCB_t                            *tree_p;
  Xtr_t                                *attr_xtr_p;
  Array_t                               illegal_atoms;
  Array_t				stereoopt;   
  U16_t                                 attr_atom_id;
  U16_t                                 attr_root;
  U16_t                                 i;
  U16_t                                 node_atom_id;
  U16_t                                 num_of_atoms;
  Boolean_t                             found;
 
  if (which_dist == FALSE)
    attr_atom_id = attribute;
  else 
    {
    attr_xtr_p = SAtable (attribute, &attr_root);
    if (attr_xtr_p == NULL)
      return (FALSE);
    
    attr_atom_id = Xtr_Attr_Atomid_Get (attr_xtr_p, attr_root);
    }
  
  node_atom_id = Xtr_Attr_Atomid_Get (xtr_p, candidate_node);
  if (node_atom_id == attr_atom_id)
    {
    if (which_dist == FALSE)
      return (TRUE);

    num_of_atoms = Xtr_NumAtoms_Get (xtr_p);
#ifdef _MIND_MEM_
    mind_Array_1d_Create ("&stereoopt", "posttest{2}", &stereoopt, num_of_atoms, BITSIZE);
    mind_Array_1d_Create ("&illegal_atoms", "posttest{2}", &illegal_atoms, num_of_atoms, BITSIZE);
#else
    Array_1d_Create (&stereoopt, num_of_atoms, BITSIZE);
    Array_1d_Create (&illegal_atoms, num_of_atoms, BITSIZE);
#endif
    Array_Set (&stereoopt, FALSE);
    Array_Set (&illegal_atoms, FALSE);
     
    /* Now mark the illegal atoms.  Illegal atoms are those non-root atoms
       at less than or equal to the required distance, or atoms mapped to
       constant nodes in the goal pattern.  First mark constant atoms as 
       illegal.
    */
    for (i = 0; i < num_of_atoms; i++)
      if (Array_1d1_Get (constant_p, i) == TRUE)
        Array_1d1_Put (&illegal_atoms, i, TRUE);
     
    /* Now mark non-root atoms at less than or equal to the required 
       distance.
    */

    if (distance > 0)
      {
      dist_p = (SDistance_t *)Array_1d32_Get (dist_matrix_p, root);
      for (i = 0; i <num_of_atoms; i++)
        if (Array_1d16_Get (SDist_Distance_Get (dist_p), i) <= distance 
            && i != candidate_node)
          Array_1d1_Put (&illegal_atoms, i, TRUE);
      } 
     
    tree_p = SubGenr_Fragment_Match (xtr_p, attr_xtr_p, &stereoopt, 
      &illegal_atoms, candidate_node, attr_root, TRUE); 
    if (tree_p != NULL)
      {
      SubGenr_MatchCB_Destroy (tree_p);
      found = TRUE;
      }
    else
      found = FALSE;

#ifdef _MIND_MEM_
    mind_Array_Destroy ("&illegal_atoms", "posttest", &illegal_atoms);
    mind_Array_Destroy ("&stereoopt", "posttest", &stereoopt);
#else
    Array_Destroy (&illegal_atoms);
    Array_Destroy (&stereoopt);
#endif
    return (found);     
    }

  return (FALSE);
}


/******************************************************************************
*
*  Function Name :          SAtable
*
*    This routine returns the attribute xtr_p and root node for a given input
*    attribute number.
*
*  Used to be :
*    
*    atable
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Xtr_t  *SAtable 
  (
  U16_t 	attribute, 
  U16_t 	*attr_root
  )
{
  Xtr_t                            *attr_xtr_p;
  String_t                          string;
  Sling_t                           sling;

  if (attribute == 0)
    {
    fprintf (stderr, 
      "SAtable:  Case (attribute = 0) is not supported by the system.\n");
    }

  if ( (attribute < 1) || (attribute > CURRENT_NUM_OF_ATTRIBUTES) )
    {
    fprintf (stderr, "SAtable: Illegal attribute number in schema %d\n", global_schema);
    exit (-2);
    }

  attribute--;
  *attr_root = Attr_Info[attribute].root;
  attr_xtr_p = Attr_Info[attribute].xtr_p;
  if (attr_xtr_p == NULL)
    {
    string = String_Create (Attr_Info[attribute].sling_name, 
      strlen (Attr_Info[attribute].sling_name));
    sling = String2Sling (string);
    attr_xtr_p = Sling2Xtr (sling);
    Attr_Info[attribute].xtr_p = attr_xtr_p;
    Sling_Destroy (sling);
    String_Destroy (string);
    }

  return (attr_xtr_p);
}

/******************************************************************************
*
*  Function Name :          SAtmdist
*
*    This routine is quite similar to SDist, except that "attr = atom_id".
*
*  Used to be :
*    
*    atmdist
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SAtmdist 
  (
  U8_t                          distance, 
  U8_t                          node, 
  U16_t                         atom_id,
  Array_t                   	*dist_matrix_p,
  Array_t                       *constant_p,
  Xtr_t                         *xtr_p,
  Array_t                       *match_p
  )
{
  return (SDist_Or_SAtmdist (distance, node, atom_id, FALSE, 
    dist_matrix_p, constant_p, xtr_p, match_p));
}

/******************************************************************************
*
*  Function Name :          SAtom
*
*    This routine returns the atom_id of the node mapped to "node" in the
*    current compound molecule.
*
*  Used to be :
*    
*    atom
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U16_t SAtom 
  (
  U8_t       node, 
  Xtr_t     *xtr_p, 
  Array_t   *match_p
  )
{
  U8_t       atom;

  atom = Array_1d16_Get (match_p, node);

  return (Xtr_Attr_Atomid_Get (xtr_p, atom));
}

/******************************************************************************
*
*  Function Name :               SAtom_Sym 
*
*    This routine returns the symbol of an atom.
*
*  Used to be :
*    
*    atom_sym
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static String_t SAtom_Sym 
  (
  U16_t          atom_id
  )
{
  String_t       temp;

  temp = String_Create (Atomid2Symbol (atom_id),
    strlen (Atomid2Symbol (atom_id)));
  if (String_Length_Get (temp) == 1 ||
      String_Value_Get (temp)[0] == '(')
     return (temp);

  String_Destroy (temp);
  temp = String_Create ("(", 1);
  String_Concat_c (&temp, Atomid2Symbol (atom_id));
  String_Concat_c (&temp, ")");

  return (temp);
}

/******************************************************************************
*
*  Function Name :           SBinary_Search
*
*    This routine binary searches the table of known substituents to see
*    if the fragment name is in the table.
*
*  Used to be :
*    
*    binary_search
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SBinary_Search 
  (
  U8_t                          *frag_string,
  S16_t                         upper_index,
  S16_t                         lower_index,
  S16_t                         *entry_index_p
  )
{
  if (upper_index > lower_index)
    return (FALSE);

  (*entry_index_p) = (upper_index + lower_index) >> 1;
  if (strcmp ((char *)frag_string, frag_info[*entry_index_p].frag_name) == 0)
    return (TRUE);
  else
    if (strcmp ((char *)frag_string, frag_info[*entry_index_p].frag_name) > 0)
      return (SBinary_Search (frag_string, upper_index, (*entry_index_p) - 1,
        entry_index_p));
    else
      return (SBinary_Search (frag_string, (*entry_index_p) + 1, lower_index,
        entry_index_p));  
}

/******************************************************************************
*
*  Function Name :           SBlkfrgn
*
*    This routine computes the bulk fragment name of the substituent.
*
*  Used to be :
*    
*    blkfrgn
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U8_t  *SBlkfrgn 
  (
  Xtr_t   *xtr_p, 
  U16_t    compound_base, 
  U8_t     index
  )
{
  U8_t           *result;
  
  result = SFragnam_Or_SBlkfrgn (xtr_p, compound_base, index, TRUE);
  return (result);
}

/******************************************************************************
*
*  Function Name :           SBredts_Rule_Violation
*
*    This routine tests whether the molecule violates the Bredts rule.
*
*  Used to be :
*    
*    bredts_rule_violation 
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SBredts_Rule_Violation 
  (
  Xtr_t 	*sptr
  )
{
  Xtr_t                       *slx;
  Array_t		       constant;
  Array_t                      dist_matrix;
  Array_t                      matchmap;
  Sling_t                      sling;
  String_t                     half_string;
  String_t                     string;
  String_t                     temp_string;
  U16_t                        distce[3];
  U16_t                        num_of_atoms;
  U16_t                        i;
  U16_t                        atom_id;
  U16_t                        ring_sys;
  U16_t                        node1;
  U16_t                        node2;
  U16_t                        node3;
  U16_t                        node4;
  U16_t                        j;
  U16_t                        neighbor;
  U16_t                        mapsize;
  S8_t                         n;
  U8_t                         degree;
  U8_t                         k;
  U8_t                         m2;
  U8_t                         m3;
  U8_t                         m23;
  U8_t                         m32;
  U8_t                         m4;
  Boolean_t                    conn2;
  Boolean_t                    conn3;
  Boolean_t                    conn4;
  Boolean_t                    node2_found;
  Boolean_t                    ok;

  String_Alloc_Put (half_string, 0); 
  String_Alloc_Put (string, 0); 

  num_of_atoms = Xtr_NumAtoms_Get (sptr);
  for (i = 0; i < num_of_atoms; i++)
    {
    atom_id = Xtr_Attr_Atomid_Get (sptr, i);
    if (String_Alloc_Get (half_string) > 0)
      String_Destroy (half_string);
 
    half_string = SAtom_Sym (atom_id);
    String_Concat_c (&half_string, ">($1)-1>($3)-1>($5)_");
    if (Xtr_Attr_NumNeighbors_Get (sptr, i) == 3 && atom_id > 4 && atom_id < 8)
      {
      ring_sys = SRing_System_Number (sptr, i);
      if (ring_sys != INVALID_NUM)
        {
        node1 = i;
        node2 = TSD_INVALID;
        n = -1;
        node2_found = FALSE;
        for (j = 0; j < 3; j++)
          {
          if (Xtr_Attr_NeighborBond_Get (sptr, i, j) == BOND_DOUBLE ||
              (Xtr_Attr_NeighborBond_Get (sptr, i, j) == BOND_RESONANT &&
              !node2_found))
            {
            node2 = Xtr_Attr_NeighborId_Get (sptr, i, j);
            atom_id = Xtr_Attr_Atomid_Get (sptr, node2);
            if (atom_id < 5 || atom_id > 8 ||
                  Xtr_Attr_NumNeighbors_Get (sptr, node2) < 2)
              node2 = TSD_INVALID;
            else
              node2_found = TRUE;   
            }
          else
            {
            n++;
            neighbor = Xtr_Attr_NeighborId_Get (sptr, i, j);
            if (n == 0)
              node3 = neighbor;
            else
              node4 = neighbor;
            }
          }  /* End of for j */

        if (node2 != TSD_INVALID &&
            Xtr_Attr_NumNeighbors_Get (sptr, node3) > 1 &&
            Xtr_Attr_NumNeighbors_Get (sptr, node4) > 1)  
          {
          for (j = 0; j < num_of_atoms; j++)
            {
            degree = Xtr_Attr_NumNeighbors_Get (sptr, j);
            if (j != node1 && j != node2 && j != node3 && j != node4 &&
                degree > 2 && SRing_System_Number (sptr, j) == ring_sys)
              {
              n = -1;
              mapsize = 5 + degree;
#ifdef _MIND_MEM_
              mind_Array_1d_Create ("&matchmap", "posttest{3}", &matchmap, mapsize, WORDSIZE);
#else
              Array_1d_Create (&matchmap, mapsize, WORDSIZE);
#endif
              atom_id = Xtr_Attr_Atomid_Get (sptr, j);
              if (String_Alloc_Get (string) > 0)
                String_Destroy (string);

              string = String_Copy (half_string);
              temp_string = SAtom_Sym (atom_id);
              String_Concat (&string, temp_string);
              String_Destroy (temp_string);
              String_Concat_c (&string, ">($7)-1>($9)-1>($11)");
              if (mapsize == 9)                 
                String_Concat_c (&string, "-1>($2)");

              for (k = 0; k < degree; k++) 
                {
                neighbor = Xtr_Attr_NeighborId_Get (sptr, j, k);
                if (Xtr_Attr_NumNeighbors_Get (sptr, neighbor) > 1)
                  {
                  n++;
                  Array_1d16_Put (&matchmap, n+5, neighbor);
                  }
                else 
                  Array_1d16_Put (&matchmap, mapsize-1, neighbor);
                }  /* End of for k */

              if (n >= 2)
                {
                Array_1d16_Put (&matchmap, 0, node1);
                Array_1d16_Put (&matchmap, 1, node2);
                Array_1d16_Put (&matchmap, 2, node3);
                Array_1d16_Put (&matchmap, 3, node4);
                Array_1d16_Put (&matchmap, 4, j);
                sling = String2Sling (string);
                slx = Sling2Xtr (sling);
                Sling_Destroy (sling);
                SBuild_Constant (sptr, slx, &constant, &matchmap, mapsize);
#ifdef _MIND_MEM_
                mind_Array_1d_Create ("&dist_matrix", "posttest{3}", &dist_matrix, Xtr_NumAtoms_Get (slx),
                  ADDRSIZE);
#else
                Array_1d_Create (&dist_matrix, Xtr_NumAtoms_Get (slx),
                  ADDRSIZE);
#endif
                Array_Set (&dist_matrix, (U32_t) NULL);
                for (m2 = 5; m2 <= n + 5; m2++)
                  for (m3 = 5; m3 <= n + 5; m3++)
                    if (m3 != m2)
                      {
                      m23 = m2; m32 = m3;
                      if (m3 < m2) 
                        {
                        m23 = m3; m32 = m2;
                        }

                      for (m4 = 5; m4 <= n + 5; m4++)
                        if (m4 != m23 && m4 != m32)
                          {
                          conn2 = (Array_1d16_Get (&matchmap, 1) ==
                            Array_1d16_Get (&matchmap, m2)) ||
                            SConn (1, m2, sptr, &constant, &matchmap); 
                          conn3 = (Array_1d16_Get (&matchmap, 2) ==
                            Array_1d16_Get (&matchmap, m3)) ||
                            SConn (2, m3, sptr, &constant, &matchmap);
                          conn4 = (Array_1d16_Get (&matchmap, 3) ==
                            Array_1d16_Get (&matchmap, m4)) ||
                            SConn (3, m4, sptr, &constant, &matchmap);
                          if (conn2 && conn3 && conn4)
                            {
                            distce[0] = SFdist (1, m2, &dist_matrix,
                              &constant, sptr, &matchmap);
                            distce[1] = SFdist (2, m3, &dist_matrix,
                              &constant, sptr, &matchmap);
                            distce[2] = SFdist (3, m4, &dist_matrix,
                              &constant, sptr, &matchmap);
                            ok = distce[0] > 3 || distce[1] > 3 ||
                              distce[2] > 3;
                            if (!ok) 
                              {
                              if (String_Alloc_Get (half_string) > 0)
                                String_Destroy (half_string);
  		              if (String_Alloc_Get (string) > 0)
                                String_Destroy (string);

                              /*  Destroy matchmap, constant, xtr, 
                                  dist_matrix (DK).
                              */
#ifdef _MIND_MEM_
                              mind_Array_Destroy ("&matchmap", "posttest", &matchmap);
                              mind_Array_Destroy ("&constant", "posttest", &constant);
#else
                              Array_Destroy (&matchmap);
                              Array_Destroy (&constant);
#endif
                              SFree_Distance_Matrix (&dist_matrix,
                                Xtr_NumAtoms_Get (slx));
                              Xtr_Destroy (slx);
                              return (TRUE);
                              }  /* End of if not ok */
                            }
                          }  /* End of if m4 != m23 && m4 != m32 */
                      }  /* End of if m3 != m2 */

#ifdef _MIND_MEM_
                    mind_Array_Destroy ("&constant", "posttest", &constant);
                    SFree_Distance_Matrix (&dist_matrix, 
                      Xtr_NumAtoms_Get (slx));
                    Xtr_Destroy (slx);
                    }  /* End of if n >= 2 */
    
                mind_Array_Destroy ("&matchmap", "posttest", &matchmap);
#else
                    Array_Destroy (&constant);
                    SFree_Distance_Matrix (&dist_matrix, 
                      Xtr_NumAtoms_Get (slx));
                    Xtr_Destroy (slx);
                    }  /* End of if n >= 2 */
    
                Array_Destroy (&matchmap); 
#endif
                }  /* End of if j != node1 && ... */
              }  /* End of for j */
          }  /* End of if node2 != TSD_INVALID */
        }  /* End of if ring_sys != INVALID_NUM */
      }  
    }  /* End of for i */

  if (String_Alloc_Get (half_string) > 0)
    String_Destroy (half_string);

  if (String_Alloc_Get (string) > 0)
    String_Destroy (string);

  return (FALSE);  
}

/******************************************************************************
*  
*  Function Name :          SBrghead
*
*    This routine determines whether a node is a carbon-atom bridgehead in
*    a [9 .9 .9] or smaller bicyclic.
*
*  Used to be :
*    
*    brghead
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate Memory
*    Deallocate Memory
*
******************************************************************************/

static Boolean_t  SBrghead (Xtr_t *target_p, U16_t node)
{
  Pathnodes_t			*temp_pathnodes_p;
  Path_List_t			*path_list_head;
  Path_List_t			*temp_list_p;
  Tsd_t				*tsd_p;
  U16_t				*pathnodes;
  U16_t				 neighbor_node[4];
  U16_t				 nodes[2];
  U16_t				 count2;
  U16_t				 i;
  U16_t				 j;
  U16_t				 k;
  U16_t				 num_paths;
  U16_t				 size;
  Boolean_t			 found;
  Boolean_t			 neighbor_used;
  
  if (node > Xtr_NumAtoms_Get (target_p))
    {
    fprintf (stderr, "SBrghead Warning : Invalid Node Number\n");
    return (FALSE);
    }

  if (Xtr_Attr_Atomid_Get (target_p, node) != 6)
    return (FALSE);

  if (Xtr_Attr_NumNonHydrogen_Get (target_p, node) < 3)
    return (FALSE);

  if (Xtr_Attr_NumNeighbors_Get (target_p, node) != 4)
    return (FALSE);

  if (!SRing_Tests_Passed (target_p, node))
    return (FALSE);

  tsd_p = Xtr2Tsd (target_p);

  for (i = 0; i < 4; i++)
    neighbor_node[i] = INVALID_NUM; 

  nodes[0] = node;
  found = FALSE;
  path_list_head = NULL;

  for (i = 0; !found && (i < Tsd_NumAtoms_Get (tsd_p)); i++)
    {
    nodes[1] = i;
    num_paths = SNpaths (tsd_p, nodes, path_list_head); 
    if (num_paths >= 3) 
      {
      count2 = 0;
      found = TRUE;
      for (j = 0; found && (j < num_paths); j++)
        {
        pathnodes = SPath (tsd_p, nodes, j, &size, path_list_head); 
        if (size == 2)
          found = FALSE;
        else
          {
          neighbor_used = FALSE;
          for (k = 0; !neighbor_used && (k < count2); k++)
            neighbor_used = (pathnodes[1] == neighbor_node[k]);

          if (!neighbor_used)
            {
            neighbor_node[count2] = pathnodes[1];
            count2++;
            }       
          } 

#ifdef _MIND_MEM_
        mind_free ("pathnodes", "posttest", pathnodes);
#else
        Mem_Dealloc (pathnodes, size, GLOBAL);       
#endif
        }  /* End of for j */

      if (found == TRUE)
        found = SThree_Unique (tsd_p, nodes, neighbor_node, count2, 
          path_list_head);
      
      if (found == TRUE)
        {
        /*  Restructure so that data structures can be destroyed before
            the result of SBridges_Ok is returned (DK).
        */
        Boolean_t       bridges_okay;
      
        bridges_okay = SBridges_Ok (tsd_p, nodes, neighbor_node, 
          path_list_head);
        while (path_list_head != NULL)
          {
          temp_list_p = path_list_head;
          path_list_head = path_list_head->next;
#ifdef _MIND_MEM_
          while (temp_list_p->pathnodes_p != NULL)
            {
            temp_pathnodes_p = temp_list_p->pathnodes_p;
            temp_list_p->pathnodes_p = (temp_list_p->pathnodes_p)->next;
            mind_free ("temp_pathnodes_p->nodes", "posttest", temp_pathnodes_p->nodes);
            mind_free ("temp_pathnodes_p", "posttest", temp_pathnodes_p);
            }
      
          mind_free ("temp_list_p", "posttest", temp_list_p);
#else
          while (temp_list_p->pathnodes_p != NULL)
            {
            temp_pathnodes_p = temp_list_p->pathnodes_p;
            temp_list_p->pathnodes_p = (temp_list_p->pathnodes_p)->next;
            Mem_Dealloc (temp_pathnodes_p->nodes, temp_pathnodes_p->size,
              GLOBAL);
            Mem_Dealloc (temp_pathnodes_p, PATHNODESSIZE, GLOBAL);
            }
      
          Mem_Dealloc (temp_list_p, PATHLISTSIZE, GLOBAL);
#endif
          }
        Tsd_Destroy (tsd_p);
        return (bridges_okay);
        }
      }  /* End of if num_paths >= 3 */
    }  /* End of for i */

  while (path_list_head != NULL)
    {
    temp_list_p = path_list_head;
    path_list_head = path_list_head->next;

#ifdef _MIND_MEM_
    while (temp_list_p->pathnodes_p != NULL)
      {
      temp_pathnodes_p = temp_list_p->pathnodes_p;
      temp_list_p->pathnodes_p = (temp_list_p->pathnodes_p)->next;
    
      mind_free ("temp_pathnodes_p->nodes", "posttest", temp_pathnodes_p->nodes);
      mind_free ("temp_pathnodes_p", "posttest", temp_pathnodes_p);
      }
    
    mind_free ("temp_list_p", "posttest", temp_list_p);
#else
    while (temp_list_p->pathnodes_p != NULL)
      {
      temp_pathnodes_p = temp_list_p->pathnodes_p;
      temp_list_p->pathnodes_p = (temp_list_p->pathnodes_p)->next;
    
      Mem_Dealloc (temp_pathnodes_p->nodes, temp_pathnodes_p->size, GLOBAL);
      Mem_Dealloc (temp_pathnodes_p, PATHNODESSIZE, GLOBAL);
      }
    
    Mem_Dealloc (temp_list_p, PATHLISTSIZE, GLOBAL);
#endif
    }

  Tsd_Destroy (tsd_p);
  return (FALSE);   
}

/******************************************************************************
*  
*  Function Name :          SBridges_Ok
*
*    This routine ???.
*
*  Used to be :
*    
*    bridges_ok
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate Memory
*    Deallocate Memory
*
******************************************************************************/

static Boolean_t  SBridges_Ok 
  (
  Tsd_t				*tsd_p, 
  U16_t 			*nodes,
  U16_t 			*neighbor_node,
  Path_List_t			*path_list_head
  )
{
  U16_t				*pathnodes;
  U16_t				 i;
  U16_t				 j;
  U16_t				 min_distance;
  U16_t				 num_paths;
  U16_t				 size;
  Boolean_t			 ok;
 
  ok = TRUE;
  num_paths = SNpaths (tsd_p, nodes, path_list_head);
  for (i = 0; ok && i < 3; i++)
    {
    min_distance = INVALID_NUM;
    for (j = 0; j < num_paths; j++)
      {
      pathnodes = SPath (tsd_p, nodes, j, &size, path_list_head);      
      if ((pathnodes[1] == neighbor_node[i]) && (min_distance > size - 2))
        min_distance = size - 2;

#ifdef _MIND_MEM_
      mind_free ("pathnodes", "posttest", pathnodes);
#else
      Mem_Dealloc (pathnodes, size, GLOBAL);
#endif
      }

    if (min_distance > 9)
      ok = FALSE;
    }

  return (ok);
}

/******************************************************************************
*  
*  Function Name :          SBuild_Constant
*
*    This routine allocates the constant array.
*
*  Used to be :
*    
*    bldcnst 
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/
static void SBuild_Constant 
  (
  Xtr_t 	*xtr_p, 
  Xtr_t 	*goal_xtr_p, 
  Array_t	*constant_p,
  Array_t 	*match_p,
  U8_t	  	matchsize
  )
{
  U16_t	        atomid;
  U16_t         count;

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("constant_p", "posttest{4}", constant_p, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
#else
  Array_1d_Create (constant_p, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
#endif
  Array_Set (constant_p, FALSE);
  for (count = 0; count < matchsize; count++)
    {
    atomid = Xtr_Attr_Atomid_Get (goal_xtr_p, count);
    if (atomid < VARIABLE_START && atomid > 0 && 
        Array_1d16_Get (match_p, count) != XTR_INVALID)
      Array_1d1_Put (constant_p, Array_1d16_Get (match_p, count), TRUE);
    }

  return ;
}

/******************************************************************************
*  
*  Function Name :          SBuild_Distance_Structure
*
*    This routine allocates the distance structure.
*
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/

static SDistance_t  *SBuild_Distance_Structure 
  (
  U16_t		size,
  U16_t		atom 
  )
{
  SDistance_t  *temp;

#ifdef _MIND_MEM_
  mind_malloc ("temp", "posttest{5}", &temp, SDISTANCESIZE);
  SDist_Depth_Put (temp, 0);
  mind_Array_1d_Create ("SDist_Distance_Get(temp)", "posttest{5}", SDist_Distance_Get (temp), size, WORDSIZE);
#else
  Mem_Alloc (SDistance_t *, temp, SDISTANCESIZE, GLOBAL);
  SDist_Depth_Put (temp, 0);
  Array_1d_Create (SDist_Distance_Get (temp), size, WORDSIZE);
#endif
  Array_Set (SDist_Distance_Get (temp), INVALID_NUM);
  Array_1d16_Put (SDist_Distance_Get (temp), atom, 0);

  return (temp);
}


/******************************************************************************
*  
*  Function Name :          SBulk
*
*    This routine returns a value for the steric hindrance (or bulk) of a 
*    variable in a reaction pattern after the reaction pattern has been
*    matched to a goal compound. Returned value is in the form of character
*    string which can be compared as an ordinary number.
*
*  Used to be :
*    
*    bulk
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U8_t  *SBulk 
  (
  U8_t 				  pattern_node,
  Xtr_t				 *xtr_p,
  Xtr_t				 *goal_xtr_p,
  Array_t			 *match_p
  )
{
  U8_t                           *name;
  U16_t                           compound_base;
  U16_t                           pattern_base;

  if (Xtr_Attr_NumNeighbors_Get (goal_xtr_p, pattern_node) != 1)
    {
/*
    fprintf (stderr, 
      "SBulk : Error! Ambiguous input, pattern node not degree 1 in schema %d. \n", global_schema);
    exit (-3);
*/
    fprintf (stderr, 
      "SBulk : Warning! Ambiguous input, pattern node not degree 1 in schema %d. \n", global_schema);
    }

  pattern_base = Xtr_Attr_NeighborId_Get (goal_xtr_p, pattern_node, 0);
  compound_base = Array_1d16_Get (match_p, pattern_base);
  name = SBlkfrgn (xtr_p, compound_base, Xtr_Attr_NeighborIndex_Find (xtr_p, 
    compound_base, Array_1d16_Get (match_p, pattern_node)));
 
  return (name); 
}

/******************************************************************************
*
*  Function Name :           SBulk2
*
*    This routine returns a value for the steric hindrance of the substituent
*    defined by the edge from "node1" to "node2" in the reaction pattern
*    after matching.
*
*  Used to be :
*    
*    bulk2
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U8_t  *SBulk2 
  (
  U8_t                   node1, 
  U8_t                   node2,
  Xtr_t                 *xtr_p,
  Array_t		*match_p
  )
{
  U8_t			*name;
  U16_t 		 node1_prime;
  U16_t                  node2_prime;

  node1_prime = Array_1d16_Get (match_p, node1);
  node2_prime = Array_1d16_Get (match_p, node2);
  name = SBlkfrgn (xtr_p, node1_prime,
    Xtr_Attr_NeighborIndex_Find (xtr_p, node1_prime, node2_prime));
 
  return (name); 
}

/******************************************************************************
*
*  Function Name :             SBulksln
*
*    This routine returns a value for the steric hindrance (or bulk) of a 
*    substituent defined by a low-h sling. Returned value is in the form
*    of character strings which can be compared as an ordinary number.
*
*  Used to be :
*    
*    bulksln
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static U8_t  *SBulksln 
  (
  const char            *low_sling_name
  )
{
  Xtr_t                *temp_xtr_p;
  U8_t 	               *name;
  String_t              string;
  Sling_t               sling;

  string = String_Create ("H", 1);
  String_Concat_c (&string, low_sling_name);
  sling = String2Sling (string);
  temp_xtr_p = Sling2Xtr_PlusHydrogen (sling);
  name = SBlkfrgn (temp_xtr_p, 0, 0);
  Xtr_Destroy (temp_xtr_p);
  Sling_Destroy (sling);
  String_Destroy (string);

  return (name);
}

/******************************************************************************
*
*  Function Name :               SCalc_Efx_Etc 
*
*    This routine ???.
*
*  Used to be :
*    
*    calc_efx_etc
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCalc_Efx_Etc 
  (
  Xtr_t				*subgoal_xtr_p,
  U16_t				*ring_nodes,
  float				*ind,
  float				*res,
  Boolean_t			*pi,
  Boolean_t			*non_h
  )
{
  U8_t				*name;
  U16_t				 res_nb[3];
  U16_t				 atom_id;
  U8_t				 i;
  U8_t				 index;
  U8_t				 j;
  U8_t			         k;
  U8_t				 n_res_nb;
  Boolean_t			 ok;

 
  ok = TRUE;
  for (i = 0; (i < 6) && ok; i++)
    {
    ok = FALSE;
    n_res_nb = 0;
    for (j = 0; j < Xtr_Attr_NumNeighbors_Get (subgoal_xtr_p, ring_nodes[i]);
         j++)
      {
      if (Xtr_Attr_NeighborBond_Get (subgoal_xtr_p, ring_nodes[i], j) == 
          BOND_SINGLE)
        {
        index = j;
        ok = TRUE;
        }
      else 
        if (Xtr_Attr_NeighborBond_Get (subgoal_xtr_p, ring_nodes[i], j) == 
            BOND_RESONANT)
          {
          res_nb[n_res_nb] = Xtr_Attr_NeighborId_Get (subgoal_xtr_p,
            ring_nodes[i], j);
          n_res_nb++;
          }
      }  /* End of for j */

    ok =  ok && n_res_nb == 2 && 
           Xtr_Attr_Atomid_Get (subgoal_xtr_p, ring_nodes[i]) == 6;

    if (ok == TRUE)
      {
      j = (i + 1) % 6;
      k = (i + 5) % 6;
      ok = res_nb[0] == ring_nodes[j] && res_nb[1] == ring_nodes[k];
      ok = ok || (res_nb[0] == ring_nodes[k] && res_nb[1] == ring_nodes[j]);
      }

    if (ok == TRUE)
      {
      atom_id = Xtr_Attr_Atomid_Get (subgoal_xtr_p, 
        Xtr_Attr_NeighborId_Get (subgoal_xtr_p, ring_nodes[i], index));
      pi[i] = atom_id == 7 || atom_id == 8 || atom_id == 9 || atom_id == 16 
        || atom_id == 17 || atom_id == 34 || atom_id == 35 || atom_id == 53;
      non_h[i] = atom_id != 1;      
      if (non_h[i] == TRUE)
        {
        name = SFragnam (subgoal_xtr_p, ring_nodes[i], index);
        SIndefct ((char *)name, ind + i, res + i);
#ifdef _MIND_MEM_
        mind_free ("name", "posttest", name);
#else
        Mem_Dealloc (name, strlen ((char *)name) + 1, GLOBAL);
#endif
        }
      else
        {
        ind[i] = 0.0;
        res[i] = 0.0;
        }
      }
    }

  return (ok);
}

/******************************************************************************
*
*  Function Name :               SCarbstb 
*
*    This routine returns the relative stabilities of two potential 
*    carbonium ions.
*
*  Used to be :
*    
*    carbstb
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static float  SCarbstb 
  (
  U8_t              node1, 
  U8_t              node2,
  Xtr_t            *xtr_p,
  Xtr_t	           *goal_xtr_p,
  Array_t          *match_p
  )
{
  float             ix;
  float             iy;
  float             rx;
  float             ry;
  U16_t             func_group[10] = {3, 4, 6, 8, 28, 40, 106, 125, 129, 135};
  U16_t             adjust;
  U16_t             atom1;
  U16_t             atom2;
  U16_t	            atom_id;
  U16_t             func_group_num;
  U16_t             instance;
  U16_t             j;
  U16_t             neighbor;
  U8_t              degree;
  U8_t              i;

  ix = 0;
  iy = 0;

  if (Xtr_FuncGroups_Get (xtr_p) == NULL)
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));     

  atom1 = Array_1d16_Get (match_p, node1);
  atom2 = Array_1d16_Get (match_p, node2);
  for (i = 0; i < 10; i++)
    {
    func_group_num = func_group[i];
    for (j = 1; j <= Xtr_FuncGrp_NumInstances_Get (xtr_p, func_group_num); j++)
      {
      instance = Xtr_FuncGrp_SubstructureInstance_Get (xtr_p, 
        func_group_num, j);
      if (ix == 0 && instance == atom1)
        if (func_group_num == 106)
          ix = -0.15 + 0.05;
        else
          ix = -0.15;
      else if (iy == 0 && instance == atom2)
        if (func_group_num == 106)
          iy = -0.15 + 0.05;
        else
          iy = -0.15;
      }   
    }  /* End of for i */
  
  if (ix == 0)
    for (i = 0; ( i < Xtr_Attr_NumNeighbors_Get (xtr_p, atom1) ); i++)
      {
      neighbor = Xtr_Attr_NeighborId_Get (xtr_p, atom1, i);
      atom_id = Xtr_Attr_Atomid_Get (xtr_p, neighbor);
      degree = Xtr_Attr_NumNeighbors_Get (xtr_p, neighbor);
      adjust = SAdjmult (xtr_p, neighbor, degree);

      if ((atom_id == 7 && adjust == 3) || (atom_id == 8 && adjust == 2) ||
          (atom_id == 9 && adjust == 1) || (atom_id == 15 && adjust == 3) ||
          (atom_id == 16 && adjust == 2) || (atom_id == 17 && adjust == 1) ||
	  (atom_id == 34 && adjust == 2) || (atom_id == 35 && adjust == 1) ||
	  (atom_id == 53 && adjust == 1))
        {
/*
        ix = -0.1;
*/
        ix = 0.1; /* should be major DEcrease in stability! */
        break; 
        } 
      }

  if (iy == 0)
    for (i = 0; ( i < Xtr_Attr_NumNeighbors_Get (xtr_p, atom2) ); i++)
      {
      neighbor = Xtr_Attr_NeighborId_Get (xtr_p, atom2, i);
      atom_id = Xtr_Attr_Atomid_Get (xtr_p, neighbor);
      degree = Xtr_Attr_NumNeighbors_Get (xtr_p, neighbor);
      adjust = SAdjmult (xtr_p, neighbor, degree);

      if ((atom_id == 7 && adjust == 3) || (atom_id == 8 && adjust == 2) ||
          (atom_id == 9 && adjust == 1) || (atom_id == 15 && adjust == 3) ||
          (atom_id == 16 && adjust == 2) || (atom_id == 17 && adjust == 1) ||
	  (atom_id == 34 && adjust == 2) || (atom_id == 35 && adjust == 1) ||
	  (atom_id == 53 && adjust == 1))
        {
/*
        iy = -0.1;
*/
        iy = 0.1; /* should be major DEcrease in stability! */
        break; 
        } 
      }

  if (ix == 0)
    SElec (node1, &ix, &rx, xtr_p, goal_xtr_p, match_p);

  if (iy == 0)
    SElec (node2, &iy, &ry, xtr_p, goal_xtr_p, match_p);

  return (-100 * (ix + iy));
}

/******************************************************************************
*
*  Function Name :          SCediscn
*
*    This routine tests whether the nodes are all constitutionally equivalent
*    when disconnected from the goal pattern.
*
*  Used to be :
*    
*    cediscn
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t SCediscn 
  (
  Xtr_t 			*target_p, 
  U8_t 				 num_of_nodes, 
  Array_t 			*nodes_p,
  Xtr_t				*goal_xtr_p,
  Array_t			*match_p
  )
{
  Boolean_t			result;

  result = SCediscn_Or_SSediscn (target_p, num_of_nodes, nodes_p, TRUE,
    goal_xtr_p, match_p);

  return (result);
}

/******************************************************************************
*
*  Function Name :          SCediscn_Or_SSediscn
*
*    This routine tests "SCediscn" or "SSediscn" depending on the value of
*    "which_case".
*
*  Used to be :
*    
*    sediscn & cediscn
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCediscn_Or_SSediscn 
  (
  Xtr_t 			*target_p, 
  U8_t 				num_of_nodes,
  Array_t			*nodes_p,
  Boolean_t			which_case,
  Xtr_t				*goal_xtr_p,
  Array_t			*match_p
  )
{
  Stack_t                      *node1_p;
  Stack_t                      *node2_p;
  Stack_t                      *node_p;
  Tsd_t                        *final_tsd_p;
  Tsd_t                        *new_tsd_p;
  Tsd_t                        *tsd_p;
  Xtr_t                        *final_xtr_p;
  Array_t			news;
  Array_t			out;
  U16_t				atom;
  U16_t				atom1;
  U16_t				atom2;
  U16_t				begin_index;
  U16_t				end_index;
  U16_t				i;
  U16_t 			j;
  U16_t				neighbor;
  U16_t				node;
  U16_t				node1;
  U16_t				node2;
  U16_t				size;
  Boolean_t			deg1;
  Boolean_t			done;
  Boolean_t			ok;
  Boolean_t			result;


  for (i = 0; i < num_of_nodes; i++)
    {
    node = Array_1d16_Get (nodes_p, i);
    if (Xtr_Attr_NumNeighbors_Get (goal_xtr_p, node) != 1)
      {
      if (which_case == TRUE)
        {
        TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
          "SCediscn Error:  pattern node #%u is not a terminal node,"
          " returning FALSE.", node));
        }
      else
        {
        TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
          "SSediscn Error:  pattern node #%u is not a terminal node,"
          " returning FALSE.", node));
        }

      return (FALSE);
      }
    }
   
  if (num_of_nodes == 2) 
    if (Xtr_Attr_NeighborId_Get (goal_xtr_p, Array_1d16_Get (nodes_p, 0), 0) 
	== Array_1d16_Get (nodes_p, 1))
      {
      atom1 = Array_1d16_Get (match_p, Array_1d16_Get (nodes_p, 0));
      atom2 = Array_1d16_Get (match_p, Array_1d16_Get (nodes_p, 1));
   
      if (which_case == TRUE)
        result = Name_ConstitutionalEquivilance_Get (target_p, atom1, atom2);
      else
        result = Name_StereochemicalEquivilance_Get (target_p, atom1, atom2);

      return (result);
      }

  tsd_p = Xtr2Tsd (target_p);
  deg1 = FALSE;
  node1_p = Stack_Create (STACK_SCALAR);
  node2_p = Stack_Create (STACK_SCALAR);

  for (i = 0; i < num_of_nodes; i++)
    {
    /* 
    Obtain all nodes and their neighbors for use in the tsd-disconnection loop;
    Also, check for terminal nodes in the target compound - if any nodes is
    terminal, all the rest must be, and their atom id's may be compared and no
    disconnection is necessary; Comparison of atom id's also provides screening
    to prevent unnecessary disconnections and calls to 
    Name_ConstitutionalEquivilance_Get () or
    Name_StereochemicalEquivilance_Get ().
    */
    node = Array_1d16_Get (nodes_p, i);
    Stack_PushU16 (node1_p, Array_1d16_Get (match_p, node));
    Stack_PushU16 (node2_p, Array_1d16_Get (match_p, 
      Xtr_Attr_NeighborId_Get (goal_xtr_p, node, 0)));    
    node = Array_1d16_Get (match_p, node);
    if (Xtr_Attr_NumNeighbors_Get (target_p, node) == 1)
      {
      if ( !deg1 && (i > 0) ) 
        {
        Tsd_Destroy (tsd_p);
        Stack_Destroy (node1_p);
        Stack_Destroy (node2_p);
        return (FALSE);
        }

      deg1 = TRUE;
      }  
    else
      if (deg1 == TRUE)
        {
        Tsd_Destroy (tsd_p);
        Stack_Destroy (node1_p);
        Stack_Destroy (node2_p);
        return (FALSE);
        } 
    }

  for (i = 1; i < num_of_nodes; i++)
    if (Tsd_Atomid_Get (tsd_p, Array_1d16_Get (match_p, 
	Array_1d16_Get (nodes_p, i))) !=
        Tsd_Atomid_Get (tsd_p, Array_1d16_Get (match_p, 
	Array_1d16_Get (nodes_p, 0))))
      {
      Tsd_Destroy (tsd_p);
      Stack_Destroy (node1_p);
      Stack_Destroy (node2_p);
      return (FALSE);
      }

  if (deg1 == TRUE)
    {
    Tsd_Destroy (tsd_p);
    Stack_Destroy (node1_p);
    Stack_Destroy (node2_p);
    return (TRUE);
    }

  for (i = 1, ok = TRUE; ok && (i < num_of_nodes); i++)
    {
    atom1 = Array_1d16_Get (match_p, Array_1d16_Get (nodes_p, i - 1));
    atom2 = Array_1d16_Get (match_p, Array_1d16_Get (nodes_p, i));

    if (which_case == TRUE)
      ok = Name_ConstitutionalEquivilance_Get (target_p, atom1, atom2);
    else
      ok = Name_StereochemicalEquivilance_Get (target_p, atom1, atom2);
    }

  if (ok == TRUE)
    {    
    Tsd_Destroy (tsd_p);
    Stack_Destroy (node1_p);
    Stack_Destroy (node2_p);
    return (TRUE);
    }
  
  new_tsd_p = Tsd_Create (num_of_nodes + Tsd_NumAtoms_Get (tsd_p));
  for (i = 0; i < Tsd_NumAtoms_Get (tsd_p); i++)
    Tsd_RowCopy (new_tsd_p, i, tsd_p, i);
  atom = Stack_TopU16 (node2_p);
  node_p = Stack_Create (STACK_SCALAR);
  for (i = Tsd_NumAtoms_Get (tsd_p); i < Tsd_NumAtoms_Get (new_tsd_p); i++)
    {
    /*  
    Add one nitrogen-atom for each node and form a ring (or, in the case of
    only two nodes, an N=N double-bond); disconnect each node from its neighbor
    (making sure to change the tsd-array members of both) and connect each node
    to one of the nitrogens; The resulting two-piece tsd cannot be used as is, 
    because an error will result during the call to 
    Name_ConstitutionalEquivilance_Get ().

    The disconnection must preserve the UP, DOWN, LEFT, RIGHT, IN, and OUT
    positionsing found in the original molecule, and the nitrogens should be
    connected to each other in a consistent fashion
    */
    Tsd_Atomid_Put (new_tsd_p, i, NITROGEN);
    node1 = Stack_TopU16 (node1_p);
    node2 = Stack_TopU16 (node2_p);
    Stack_Pop_Save (node1_p);
    Stack_Pop_Save (node2_p);
    if (Tsd_NumAtoms_Get (new_tsd_p) > Tsd_NumAtoms_Get (tsd_p) + 2)
      {
      neighbor = i - 1;
      if (neighbor == Tsd_NumAtoms_Get (tsd_p) - 1)
        neighbor = Tsd_NumAtoms_Get (new_tsd_p) - 1;

      Tsd_Atom_NeighborId_Put (new_tsd_p, i, 0, neighbor); 
      neighbor = i + 1;
      if (neighbor == Tsd_NumAtoms_Get (new_tsd_p))
        neighbor = Tsd_NumAtoms_Get (tsd_p);

      Tsd_Atom_NeighborId_Put (new_tsd_p, i, 1, neighbor); 
      Tsd_Atom_NeighborId_Put (new_tsd_p, i, 2, node1);
      Tsd_Atom_NeighborBond_Put (new_tsd_p, i, 0, BOND_SINGLE);
      Tsd_Atom_NeighborBond_Put (new_tsd_p, i, 1, BOND_SINGLE);
      Tsd_Atom_NeighborBond_Put (new_tsd_p, i, 2, BOND_SINGLE);
      }
    else
      {
      Tsd_Atom_NeighborBond_Put (new_tsd_p, i, 0, BOND_SINGLE);
      if (i == Tsd_NumAtoms_Get (tsd_p))
        Tsd_Atom_NeighborBond_Put (new_tsd_p, i, 5, BOND_DOUBLE);
      else
        Tsd_Atom_NeighborBond_Put (new_tsd_p, i, 4, BOND_DOUBLE);

      if ( i < (Tsd_NumAtoms_Get (new_tsd_p) - 1))
         Tsd_Atom_NeighborId_Put (new_tsd_p, i, 5, 
	   Tsd_NumAtoms_Get (new_tsd_p) - 1);
      else
        Tsd_Atom_NeighborId_Put (new_tsd_p, i, 4, 
         Tsd_NumAtoms_Get (new_tsd_p) - 2);
      Tsd_Atom_NeighborId_Put (new_tsd_p, i, 0, node1);
      }

    for (j = 0; j < MX_NEIGHBORS; j++)
      {
      if (Tsd_Atom_NeighborId_Get (new_tsd_p, node1, j) == node2)
        {
        Tsd_Atom_NeighborId_Put (new_tsd_p, node1, j, i);
        Tsd_Atom_NeighborBond_Put (new_tsd_p, node1, j , BOND_SINGLE);
        }
      
      if (Tsd_Atom_NeighborId_Get (new_tsd_p, node2, j) == node1)
	Tsd_Atom_NeighborId_Put (new_tsd_p, node2, j, TSD_INVALID);
      } 

    Stack_PushU16 (node_p, node1);
  }
    
  Tsd_Destroy (tsd_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&out", "posttest{6}", &out, Tsd_NumAtoms_Get (new_tsd_p), BITSIZE);
  Array_Set (&out, FALSE);
  mind_Array_1d_Create ("&news", "posttest{6}", &news, Tsd_NumAtoms_Get (new_tsd_p), WORDSIZE);
#else
  Array_1d_Create (&out, Tsd_NumAtoms_Get (new_tsd_p), BITSIZE);
  Array_Set (&out, FALSE);
  Array_1d_Create (&news, Tsd_NumAtoms_Get (new_tsd_p), WORDSIZE);
#endif
  for (i = 0; i < Tsd_NumAtoms_Get (new_tsd_p); i++)
    Array_1d16_Put (&news, i, i);

  SGetout (atom, new_tsd_p, &out);  
  for (i = 0; i < Tsd_NumAtoms_Get (new_tsd_p); i++)
    if (Array_1d1_Get (&out, i) == FALSE)
      break;

  if (i >= Tsd_NumAtoms_Get (new_tsd_p) - 1)
    {
#ifdef _MIND_MEM_
    mind_Array_Destroy ("&out", "posttest", &out);
    mind_Array_Destroy ("&news", "posttest", &news);
#else
    Array_Destroy (&out);
    Array_Destroy (&news);
#endif
    Stack_Destroy (node1_p);
    Stack_Destroy (node2_p);
    Stack_Destroy (node_p);
    Tsd_Destroy (new_tsd_p);
    return (FALSE);
    }


  done = FALSE;

  /*
  Condense the tsd by removing the unwanted atoms defined by out; Close
  up the openings by moving the remaining atoms down.
  */

  while (done == FALSE)
    {
    begin_index = 0;
    while (Array_1d1_Get (&out, begin_index) == FALSE)
      begin_index++;

    Array_1d1_Put (&out, begin_index, FALSE);
    end_index = begin_index + 1;
    while (Array_1d1_Get (&out, end_index) == TRUE)
      end_index++;
    
    SReposition (begin_index, end_index, new_tsd_p, &news);
    Array_1d1_Put (&out, end_index, TRUE);
    done = (end_index == Tsd_NumAtoms_Get (new_tsd_p) - 1);
    }

  size = begin_index + 1;
  final_tsd_p = Tsd_Create (size);
  
  /* prepare a final version of the tsd */
  for (i = 0; i < size; i++)
    Tsd_RowCopy (final_tsd_p, i, new_tsd_p, i);
  
  final_xtr_p = Tsd2Xtr (final_tsd_p);
  node1 = Array_1d16_Get (&news, Stack_TopU16 (node_p));
  Stack_Pop_Save (node_p);
  for (i = 1, result = TRUE; result && (i < num_of_nodes); i++)
    {
    node2 = Array_1d16_Get (&news, Stack_TopU16 (node_p));
    Stack_Pop_Save (node_p);
    if (which_case == TRUE)
      result = Name_ConstitutionalEquivilance_Get (final_xtr_p, 
			          node1, node2); 
    else
      result = Name_StereochemicalEquivilance_Get (final_xtr_p,
    				  node1, node2);
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&out", "posttest", &out);
  mind_Array_Destroy ("&news", "posttest", &news);
#else
  Array_Destroy (&out);
  Array_Destroy (&news);
#endif
  Stack_Destroy (node_p);
  Stack_Destroy (node1_p);
  Stack_Destroy (node2_p);
  Tsd_Destroy (new_tsd_p);
  Tsd_Destroy (final_tsd_p);
  Xtr_Destroy (final_xtr_p);

  return (result);
} 

/******************************************************************************
*
*  Function Name :          SCompare_Two_Strings
*
*    This routine compares two strings and returns the result.
*
*  Used to be :
*    
*    cmprstr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t  SCompare_Two_Strings 
*/
static Zadehan_t  SCompare_Two_Strings 
  (
  U8_t      *str1, 
  U8_t      *str2, 
  U8_t       comp_op
  )
{
  Zadehan_t true, false, ambiguous, fuzzy_result;
  int cmpval, signed_conf;

  true.fuzzy_components.TF = 1;
  true.fuzzy_components.conf = 100;
  false.fuzzy_components.TF = 0;
  false.fuzzy_components.conf = 100;
  ambiguous.fuzzy_components.TF = 0;
  ambiguous.fuzzy_components.conf = 50;

  if (comp_op == OP_LT)
    {
/*
    return (strcmp ((char *)str1, (char *)str2) < 0);
*/
    if ((cmpval = fuzzy_strcmp (str1, str2)) < 4)
      {
/*
      if (cmpval < 0) return (true);
*/
      if (cmpval < -5) return (true);
      signed_conf = 20 * (cmpval + 1);
      if (signed_conf < 0)
        {
        fuzzy_result = true;
        fuzzy_result.fuzzy_components.conf = -signed_conf;
        }
      else
        {
        fuzzy_result = false;
        fuzzy_result.fuzzy_components.conf = signed_conf;
        }
      }
    else return (false);
    }
  else if (comp_op == OP_GT)
    {
/*
    return (strcmp ((char *)str1, (char *)str2) > 0);
*/
    if ((cmpval = fuzzy_strcmp (str1, str2)) > -4)
      {
      if (cmpval > 5) return (true);
      signed_conf = 20 * (cmpval - 1);
      if (signed_conf < 0)
        {
        fuzzy_result = false;
        fuzzy_result.fuzzy_components.conf = -signed_conf;
        }
      else
        {
        fuzzy_result = true;
        fuzzy_result.fuzzy_components.conf = signed_conf;
        }
      }
    else return (false);
    }
  else
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
      "SCompare_Two_Strings Error:  invalid operator %hu.", comp_op));
/*
    return (FALSE);
*/
    return (ambiguous);
    }
}
  
/******************************************************************************
*
*  Function Name :          SCompare_Two_Values
*
*    This routine compares two values and returns the result.
*
*  Used to be :
*    
*    cmprval
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t SCompare_Two_Values 
*/
static Zadehan_t SCompare_Two_Values 
  (
  S16_t      val1, 
  S16_t      val2, 
  U8_t       comp_op,
  U8_t       multiplier
  )
{
  Zadehan_t  true, false, ambiguous, fuzzy_result;
  Boolean_t  exact;

  exact = multiplier == 1;
  true.fuzzy_components.TF = 1;
  true.fuzzy_components.conf = 100;
  false.fuzzy_components.TF = 0;
  false.fuzzy_components.conf = 100;
  ambiguous.fuzzy_components.TF = 0;
  ambiguous.fuzzy_components.conf = 50;
  
  switch(comp_op)
    {
    case OP_GT : 
printf("OP_GT: val1=%d val2=%d exact=%d\n",val1,val2,exact);
      if (exact) return (val1 > val2 ? true : false);
      if (val1 + multiplier / 2 > val2)
      {
printf("val1+%d/2>val2\n",multiplier);
#ifdef _FDEBUG_
printf("fuzzy true\n");
#endif
        if (val1 <= val2)
        {
printf("val1<=val2\n");
#ifdef _FDEBUG_
printf("fuzzy false: ");
#endif
          fuzzy_result = false;
          fuzzy_result.fuzzy_components.conf = MIN (100, 400 * (val2 - val1 + 1) * (val2 - val1 + 1) / multiplier / multiplier);
#ifdef _FDEBUG_
printf("MIN (100, 200 * (%d - %d + 1 [= %d]) / %d = %d\n",val2,val1,val2-val1+1,multiplier,fuzzy_result.fuzzy_components.conf);
#endif
        }
        else return (true);
      }
      else return (false);
      break; /* !!! Bad form to use (formerly) unconditional return in lieu of break !!! */
    case OP_GE :
printf("OP_GE: val1=%d val2=%d exact=%d\n",val1,val2,exact);
      if (exact) return (val1 >= val2 ? true : false);
      if (val1 + multiplier / 2 >= val2)
      {
printf("val1+%d/2>=val2\n",multiplier);
        if (val1 < val2)
        {
printf("val1<val2\n");
          fuzzy_result = false;
          fuzzy_result.fuzzy_components.conf = MIN (100, 400 * (val2 - val1) * (val2 - val1) / multiplier / multiplier);
        }
        else return (true);
      }
      else return (false);
      break; /* !!! Bad form to use (formerly) unconditional return in lieu of break !!! */
    case OP_LT :
      if (exact) return (val1 < val2 ? true : false);
      if (val1 - multiplier / 2 < val2)
      {
        if (val1 >= val2)
        {
          fuzzy_result = false;
          fuzzy_result.fuzzy_components.conf = MIN (100, 400 * (val1 - val2 + 1) * (val1 - val2 + 1) / multiplier / multiplier);
        }
        else return (true);
      }
      else return (false);
      break; /* !!! Bad form to use (formerly) unconditional return in lieu of break !!! */
    case OP_LE :
      if (exact) return (val1 <= val2 ? true : false);
      if (val1 - multiplier / 2 <= val2)
      {
        if (val1 > val2)
        {
          fuzzy_result = false;
          fuzzy_result.fuzzy_components.conf = MIN (100, 400 * (val1 - val2) * (val1 - val2) / multiplier / multiplier);
        }
        else return (true);
      }
      else return (false);
      break; /* !!! Bad form to use (formerly) unconditional return in lieu of break !!! */
    case OP_EQ : 
      if (exact) return (val1 == val2 ? true : false);
      if (abs (val1 - val2) < multiplier / 2)
      {
        if (val1 != val2)
        {
          fuzzy_result = false;
          fuzzy_result.fuzzy_components.conf = MIN (100, 400 * abs (val1 - val2) * abs (val1 - val2) / multiplier / multiplier);
        }
        else return (true);
      }
      else return (false);
      break; /* !!! Bad form to use (formerly) unconditional return in lieu of break !!! */
    case OP_NE :
      if (exact) return (val1 != val2 ? true : false);
      if (abs (val1 - val2) < multiplier / 2)
      {
        if (val1 == val2) return (false);
        else
        {
          fuzzy_result = true;
          fuzzy_result.fuzzy_components.conf = MIN (100, 400 * abs (val1 - val2) * abs (val1 - val2) / multiplier / multiplier);
        }
      }
      else return (true);
      break; /* !!! Bad form to use (formerly) unconditional return in lieu of break !!! */
    default :
      TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
        "SCompare_Two_Values Error:  invalid operator %hu.", comp_op));
      return (exact ? false : ambiguous);
    }
  return (fuzzy_result);
}



/******************************************************************************
*
*  Function Name :               SCompok
*
*    This routine checks the goal compound for the presence of various
*    illegal structures and combinations of structures that would invalidate
*    the compounds selection by synchem for further expansion.
*
*  Used to be :
*    
*    compok
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCompok 
  (
  Xtr_t 	*sptr
  )
{
  if (Xtr_NumAtoms_Get (sptr) == 1)
    return (TRUE);

  if (SIllegal_Substructure (sptr) == TRUE)
    return (FALSE);

  if (SIllegal_Combination (sptr) == TRUE)
    return (FALSE);

  if (SIllegal_Small_Rings (sptr) == TRUE)
    return (FALSE);

  return (TRUE);
}

/******************************************************************************
*
*  Function Name :         SCond_Alkyne_Test
*   
*    This routine tests the alkyne ring condition.
*
*  Used to be :
*    
*    alkpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Alkyne_Test 
  (
  Condition_t        *cond_p,
  Xtr_t              *xtr_p,
  Array_t            *match_p
  )
{
  Boolean_t            result;

  result = SAlkynsr (Cond_Alkyne_Prime_Get (cond_p),
    Cond_Alkyne_Second_Get (cond_p), xtr_p, match_p);
   
  return (result);
}

/******************************************************************************
*
*  Function Name :               SCond_Allene_Test 
*
*    This routine tests the allene ring condition.
*
*  Used to be :
*    
*    allpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Allene_Test 
  (
  Condition_t                       *cond_p,
  Xtr_t                             *xtr_p,
  Array_t                           *match_p
  )
{
  Boolean_t                result;

  result = SAllensr (Cond_Allene_Prime_Get (cond_p),
    Cond_Allene_Second_Get (cond_p), xtr_p, match_p);

  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Aromat_Sub_Test
*
*    This routine tests the aromatic substitution condition.
*
*  Used to be :
*    
*    arsub_pr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t  SCond_Aromat_Sub_Test 
*/
static Zadehan_t  SCond_Aromat_Sub_Test 
  (
  Condition_t 			*cond_p,
  Array_t			*subgxtr_p,
  Array_t			*subgmap_p,
  Array_t			*match_p
  )
{
  Xtr_t			        *subgoal_xtr_p;
  float				 nod_rtng;
  S16_t				 value1;
  S16_t				 value2;
  U16_t				 atom_index;
  U16_t				 subgoal_index;
  U8_t				 node;
  Boolean_t			 honly;
  Boolean_t			 result;
  Zadehan_t                      fuzzy_result, true, false, ambiguous;
  Boolean_t                      fuzzy;

  true.fuzzy_components.TF = 1;
  true.fuzzy_components.conf = 100;
  false.fuzzy_components.TF = 0;
  false.fuzzy_components.conf = 100;
  ambiguous.fuzzy_components.TF = 0;
  ambiguous.fuzzy_components.conf = 50;
  fuzzy = FALSE; /* unless modified in switch */

  honly = Cond_AromatSub_Hydrogen_Get (cond_p);
  node = Cond_AromatSub_Node_Get (cond_p);
  node = Array_1d16_Get (match_p, node);
  subgoal_index = Array_2d16_Get (subgmap_p, node, SUBG_COL);
  atom_index = Array_2d16_Get (subgmap_p, node, ATOM_COL);
  subgoal_xtr_p = (Xtr_t *) Array_1d32_Get (subgxtr_p, subgoal_index);
  switch (Cond_AromatSub_Type_Get (cond_p))
    {
    case PT_AROM1:
      result = SOkring (subgoal_xtr_p, atom_index, honly);
      break;

    case PT_AROM2:
      value1 = SStdng (subgoal_xtr_p, atom_index, honly);
      value2 = Cond_AromatSub_Metric_Get (cond_p);
#ifdef _DEBUG_
printf("PT_AROM2: value1=%d value2=%d\n",value1,value2);
#endif
/*
      result = SCompare_Two_Values (value1, value2, Cond_Op_Get (cond_p));
*/
      fuzzy = TRUE;
      fuzzy_result = SCompare_Two_Values (value1, value2, Cond_Op_Get (cond_p), ARSTD_MULT);
      break;

    case PT_AROM3:
      nod_rtng = 100.0 * SRatng (subgoal_xtr_p, atom_index, honly);
      if (nod_rtng > 32767 || nod_rtng < -32768)
        {
        TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
          "SCond_Aromat_Sub_Test Error:  Arrat value uncalculated or out of"
          " range %f.", nod_rtng));
/*
        return (FALSE);
*/
        return (ambiguous);
        }

      value1 = (S16_t) nod_rtng;     
      value2 = Cond_AromatSub_Metric_Get (cond_p);
/*
      result = SCompare_Two_Values (value1, value2, Cond_Op_Get (cond_p));
*/
      fuzzy = TRUE;
      fuzzy_result = SCompare_Two_Values (value1, value2, Cond_Op_Get (cond_p), ARRAT_MULT);
      break;

    case PT_AROM4:
      result = (SN_Tie (subgoal_xtr_p, atom_index, honly) ==
        SN_Cetie (subgoal_xtr_p, atom_index, honly));
      break;

    case PT_AROM5:
      value1 = SN_Tie (subgoal_xtr_p, atom_index, honly);
      value2 = Cond_AromatSub_Metric_Get (cond_p);
/*
      result = SCompare_Two_Values (value1, value2, Cond_Op_Get (cond_p));
*/
      fuzzy_result = SCompare_Two_Values (value1, value2, Cond_Op_Get (cond_p), 1);
      result = fuzzy_result.fuzzy_components.TF;
      break;

    default :
      TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
        "SCond_Aromat_Sub_Test Error:  invalid condition field %hu.",
        Cond_AromatSub_Type_Get (cond_p)));
/*
      result = FALSE;
*/
      return (ambiguous);
      break;    
    }

  if (!fuzzy)
  {
    fuzzy_result.fuzzy_components.TF = result;
    fuzzy_result.fuzzy_components.conf = 100;
  }
/*
  return (result);
*/
  return (fuzzy_result);
}

/******************************************************************************
*
*  Function Name :          SCond_At_Coneq_Or_Streq_Test
*
*    This routine tests the constitutionally equivalent atoms condition
*    or the stereochemically equivalent atoms condition.
*
*  Used to be :
*    
*    identpr 
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static Boolean_t  SCond_At_Coneq_Or_Streq_Test 
  (
  Condition_t                   *cond_p,
  Boolean_t    			constonly,
  Xtr_t                         *target_p,
  Array_t                       *subgxtr_p,
  Array_t                       *subgmap_p,
  Array_t                       *match_p,
  Array_t			*tsdmaps_p
  )
{
  Xtr_t                         *sptr;
  U8_t                          *name1;
  U8_t                          *name2;
  Array_t			nodes;
  U16_t                         atmid;
  U16_t                         cnode;
  U16_t				index;
  U16_t                         j;
  U16_t                         k;
  U16_t                         node1;
  U16_t                         sgnode1;
  U16_t                         sgnode2;
  U16_t                         sg_num1;
  U16_t                         sg_num2;
  U8_t                          i;
  Boolean_t                     found;
  Boolean_t                     ok;

  ok = TRUE;
  atmid = 0;
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&nodes", "posttest{7}", &nodes, Cond_Count_Get (cond_p), WORDSIZE);
#else
  Array_1d_Create (&nodes, Cond_Count_Get (cond_p), WORDSIZE);
#endif
  for (i = 0; i < Cond_Count_Get (cond_p); i++)
    Array_1d16_Put (&nodes, i, (constonly == TRUE) ? 
      Cond_AtomsCE_Get (cond_p, i) : Cond_AtomsSE_Get (cond_p, i));  
                                    
  for (i = 1; (i < Cond_Count_Get (cond_p)) && ok; i++)
    {
    node1 = Array_1d16_Get (&nodes, i-1);
    cnode = Array_1d16_Get (&nodes, i);
    node1 = Array_1d16_Get (match_p, node1);
    cnode = Array_1d16_Get (match_p, cnode);
    if (Cond_Goal_Get (cond_p) == TRUE)
      {
      if (Xtr_Attr_NumNeighbors_Get (target_p, node1) == 1 ||
          Xtr_Attr_NumNeighbors_Get (target_p, cnode) == 1)
        {
        ok = (Xtr_Attr_NumNeighbors_Get (target_p, node1) ==
              Xtr_Attr_NumNeighbors_Get (target_p, cnode)) &&
              (Xtr_Attr_Atomid_Get (target_p, node1) ==
               Xtr_Attr_Atomid_Get (target_p, cnode));
        if (ok)
          {
          node1 = Xtr_Attr_NeighborId_Get (target_p, node1, 0);
          cnode = Xtr_Attr_NeighborId_Get (target_p, cnode, 0);
          }
        }
      if (ok && (node1 != cnode))
         if (constonly == TRUE)
            ok = Name_ConstitutionalEquivilance_Get (target_p, node1, cnode);
          else
            ok = Name_StereochemicalEquivilance_Get (target_p, node1, cnode);
      }
    else
      { 
      sgnode1 = Array_2d16_Get (subgmap_p, node1, ATOM_COL);
      sgnode2 = Array_2d16_Get (subgmap_p, cnode, ATOM_COL);      
      sg_num1 = Array_2d16_Get (subgmap_p, node1, SUBG_COL);
      sg_num2 = Array_2d16_Get (subgmap_p, cnode, SUBG_COL);
      ok = (sg_num1 == sg_num2);

      if (ok == FALSE)
        {
        name1 = Sling_Name_Get (Name_Canonical_Get (
          Xtr_Name_Get ((Xtr_t *)Array_1d32_Get (subgxtr_p, sg_num1))));
        name2 = Sling_Name_Get (Name_Canonical_Get (
          Xtr_Name_Get ((Xtr_t *)Array_1d32_Get (subgxtr_p, sg_num2))));
        if (constonly == FALSE)
          {
          /* two names are compared completely */
          if (strcmp ( (char *) name1, (char *) name2) == 0)
            ok = TRUE;
          else
            ok = FALSE;
          }
        else   /* two names are compared partially */
          {
          for (j = 0, ok = FALSE; j < strlen((char *)name1) 
               && j < strlen((char *)name2); j++)
            if (name1[j] != name2[j])
              {
              ok = FALSE;
              break;
              }
            else if (name1[j] == '|')
              {
              ok = TRUE;
              break;
              }
          }
    
        if (ok == TRUE)
          {
          sptr = (Xtr_t *)Array_1d32_Get (subgxtr_p, sg_num2);
          found = TRUE;
          if (Xtr_Attr_NumNeighbors_Get (sptr, sgnode2) == 1)
            {
            atmid =  Xtr_Attr_Atomid_Get (sptr, sgnode2);
            found = FALSE;
            sgnode2 = Xtr_Attr_NeighborId_Get (sptr, sgnode2, 0);
            }

          index = Array_2d16_Get (&(tsdmaps_p[sg_num2]), sgnode2, 0);
	  sgnode2 = Array_2d16_Get (&(tsdmaps_p[sg_num1]), index, 1);
          sptr = (Xtr_t *)Array_1d32_Get (subgxtr_p, sg_num1);
          for (j = 0; j < Xtr_Attr_NumNeighbors_Get (sptr, sgnode2) 
               && !found; j++) 
            {
            k = Xtr_Attr_NeighborId_Get (sptr, sgnode2, j);
            found = Xtr_Attr_Atomid_Get (sptr, k) == atmid &&
              Xtr_Attr_NumNeighbors_Get (sptr, k) == 1;
            if (found == TRUE)
              sgnode2 = k;
            }
          }
        }  /* End of if okay is FALSE */

      if (ok == TRUE) 
        {
        sptr = (Xtr_t *)Array_1d32_Get (subgxtr_p, sg_num1);
        if (Xtr_Attr_NumNeighbors_Get (sptr, sgnode1) == 1 ||
            Xtr_Attr_NumNeighbors_Get (sptr, sgnode2) == 1)
          {
          ok = Xtr_Attr_NumNeighbors_Get (sptr, sgnode1) ==
            Xtr_Attr_NumNeighbors_Get (sptr, sgnode2) &&
            Xtr_Attr_Atomid_Get (sptr, sgnode1) == 
            Xtr_Attr_Atomid_Get (sptr, sgnode2);
          if (ok)
            {
            sgnode1 = Xtr_Attr_NeighborId_Get (sptr, sgnode1, 0);
            sgnode2 = Xtr_Attr_NeighborId_Get (sptr, sgnode2, 0);
            }          
          }

        if (ok && sgnode1 != sgnode2)
          if (constonly == TRUE)
            ok = Name_ConstitutionalEquivilance_Get (sptr, sgnode1, sgnode2);
          else
            ok = Name_StereochemicalEquivilance_Get (sptr, sgnode1, sgnode2);
        }  /* End of if ok is TRUE */
      }
    }  /* End of for i */

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&nodes", "posttest", &nodes);
#else
  Array_Destroy (&nodes);
#endif

  return (ok); 
}




/******************************************************************************
*
*  Function Name :          SCond_Atom_Test
*
*    This routine tests the atom (matching) condition.
*
*  Used to be :
*    
*    atompr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Atom_Test 
  (
  Condition_t                       *cond_p,
  Array_t                      	    *dist_matrix_p,
  Array_t                           *constant_p, 
  Xtr_t                             *xtr_p,
  Array_t                           *match_p
  )
{
  Boolean_t               result;

  if (Cond_Atom_Distance_Get (cond_p) != 0)
    result = SAtmdist (Cond_Atom_Distance_Get (cond_p), 
      Cond_Atom_Node_Get (cond_p), Cond_Atom_Id_Get (cond_p), dist_matrix_p,
      constant_p, xtr_p, match_p);
  else
    result = SAtom (Cond_Atom_Node_Get (cond_p), xtr_p, match_p) 
      == Cond_Atom_Id_Get (cond_p);

  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Bridge_Head_Test
*
*    This routine tests the bridge head condition.
*
*  Used to be :
*    
*    bheadpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Bridge_Head_Test
  (
  Condition_t				*cond_p,
  Xtr_t					*target_p,
  Array_t				*match_p
  )
{
  U16_t					 atom;
  U8_t					 node;
  Boolean_t				 result;


  fprintf (stderr, "BRGHD Not yet implemented (schema %d)\n", global_schema);
  Condition_Dump (cond_p, &GStdErr);
  exit (-1);

/*
  node = Cond_BridgeHead_Node_Get (cond_p);
*/

  node = 0;  /*  Dummy value to allow clean compile.  */
  atom = Array_1d16_Get (match_p, node);
  result = SBrghead (target_p, atom);

  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Bulky_Test
*
*    This routine tests the substituent bulk condition.
*
*  Used to be :
*    
*    bulkpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t  SCond_Bulky_Test 
*/
static Zadehan_t  SCond_Bulky_Test 
  (
  Condition_t          *cond_p,
  Xtr_t		       *xtr_p,
  Xtr_t		       *goal_xtr_p,
  Array_t	       *match_p
  )
{
  U8_t                *bulk_1;
  U8_t                *bulk_2;
/*
  Boolean_t            result;
*/
  Zadehan_t            result;
   
  if (Cond_Count_Get (cond_p) == 1)
    bulk_1 = SBulk (Cond_Bulk_Prime_Get (cond_p, 0), xtr_p, goal_xtr_p,
      match_p);
  else
    bulk_1 = SBulk2 (Cond_Bulk_Prime_Get (cond_p, 0), 
      Cond_Bulk_Prime_Get (cond_p, 1), xtr_p, match_p);

  if (Cond_Base_Exists (cond_p) == TRUE)
    switch (Cond_Base_Get (cond_p)) 
      {
      case 17 :                                  /* methyl group */
        bulk_2 = SBulksln ("CH-1H-1H");
        break;
      case 18 :                                  /* ethyl group */
        bulk_2 = SBulksln ("CH-1H-1CH-1H-1H");
        break;
      case 19 :                                  /* isopropyl group */
        bulk_2 = SBulksln ("CH-1CH-1H-1H-2CH-1H-1H");
        break;
      case 20 :
        bulk_2 = SBulksln ("CCH-1H-1H-2CH-1H-1H-2CH-1H-1H");
        break;
      default :
        TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
          "SCond_Bulky_Test:  invalid base bulky group %hu.",
          Cond_Base_Get (cond_p)));
        bulk_2 = NULL;
        break;
      }
  else 
    if (Cond_Count2_Get (cond_p) == 1)
      bulk_2 = SBulk (Cond_Bulk_Second_Get (cond_p, 0), xtr_p, goal_xtr_p,
        match_p);
    else
      bulk_2 = SBulk2 (Cond_Bulk_Second_Get (cond_p, 0),
        Cond_Bulk_Second_Get (cond_p, 1), xtr_p, match_p);

  result = SCompare_Two_Strings (bulk_1, bulk_2, Cond_Op_Get (cond_p));
#ifdef _MIND_MEM_
  if (bulk_1 != NULL)
    mind_free ("bulk_1", "posttest", bulk_1);
  if (bulk_2 != NULL)
    mind_free ("bulk_2", "posttest", bulk_2);
#else
  if (bulk_1 != NULL)
    Mem_Dealloc (bulk_1, strlen ((char *)bulk_1) + 1, GLOBAL);
  if (bulk_2 != NULL)
    Mem_Dealloc (bulk_2, strlen ((char *)bulk_2) + 1, GLOBAL);
#endif

  return (result); 
}

/******************************************************************************
*
*  Function Name :               SCond_Carbonium_Test
*
*    This routine tests the carbonium ion stability condition. 
*
*  Used to be :
*    
*    stabpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t  SCond_Carbonium_Test 
*/
static Zadehan_t  SCond_Carbonium_Test 
  (
  Condition_t 		 *cond_p,
  Xtr_t			 *xtr_p,
  Xtr_t			 *goal_xtr_p,
  Array_t		 *match_p
  )
{
  S8_t                  val1;
  S8_t                  val2;
/*
  Boolean_t             result;
*/
  Zadehan_t             result;

  val1 = (S8_t) SCarbstb (Cond_Stability_Prime_Get (cond_p, 0),
    Cond_Stability_Prime_Get (cond_p, 1), xtr_p, goal_xtr_p, match_p);
  val2 = (S8_t) SCarbstb (Cond_Stability_Second_Get (cond_p, 0),
    Cond_Stability_Second_Get (cond_p, 1), xtr_p, goal_xtr_p, match_p);
  result = SCompare_Two_Values (val1, val2, Cond_Op_Get (cond_p), CARSB_MULT);

  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Disc_Coneq_Test
*
*    This routine tests the constitutionally equivalent disconnected
*    atoms condition.
*
*  Used to be :
*    
*    discnpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate Memory
*    Deallocate Memory
*
******************************************************************************/


static Boolean_t  SCond_Disc_Coneq_Test 
  (
  Condition_t 			*cond_p, 
  Xtr_t 			*target_p,
  Xtr_t				*goal_xtr_p,
  Array_t			*match_p
  )
{
  Array_t		nodes;
  U8_t                	i;
  Boolean_t            result;

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&nodes", "posttest{8}", &nodes, Cond_Count_Get (cond_p), WORDSIZE);
  for (i = 0; i < Cond_Count_Get (cond_p); i++)
     Array_1d16_Put (&nodes, i, Cond_DiscCE_Get (cond_p, i));

  result = SCediscn (target_p, Cond_Count_Get (cond_p), &nodes,
    goal_xtr_p, match_p);
  mind_Array_Destroy ("&nodes", "posttest", &nodes);
#else
  Array_1d_Create (&nodes, Cond_Count_Get (cond_p), WORDSIZE);
  for (i = 0; i < Cond_Count_Get (cond_p); i++)
     Array_1d16_Put (&nodes, i, Cond_DiscCE_Get (cond_p, i));

  result = SCediscn (target_p, Cond_Count_Get (cond_p), &nodes,
    goal_xtr_p, match_p);
  Array_Destroy (&nodes); 
#endif

  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Disc_Streq_Test
*
*    This routine tests the distance to substituent condition.
*
*  Used to be :
*    
*    discnpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/


static Boolean_t  SCond_Disc_Streq_Test 
  (
  Condition_t 			*cond_p, 
  Xtr_t 			*target_p,
  Xtr_t				*goal_xtr_p,
  Array_t			*match_p
  )
{
  Array_t		nodes;
  U8_t                 i;
  Boolean_t            result;

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&nodes", "posttest{9}", &nodes, Cond_Count_Get (cond_p), WORDSIZE);
  for (i=0; i < Cond_Count_Get (cond_p); i++)
    Array_1d16_Put (&nodes, i, Cond_DiscSE_Get (cond_p, i));

  result = SSediscn (target_p, Cond_Count_Get (cond_p), &nodes, 
    goal_xtr_p, match_p);
  mind_Array_Destroy ("&nodes", "posttest", &nodes);
#else
  Array_1d_Create (&nodes, Cond_Count_Get (cond_p), WORDSIZE);
  for (i=0; i < Cond_Count_Get (cond_p); i++)
    Array_1d16_Put (&nodes, i, Cond_DiscSE_Get (cond_p, i));

  result = SSediscn (target_p, Cond_Count_Get (cond_p), &nodes, 
    goal_xtr_p, match_p);
  Array_Destroy (&nodes);
#endif

  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Dist_Test
*
*    This routine tests the distance to substituent condition.
*
*  Used to be :
*    
*    distpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Dist_Test 
  (
  Condition_t                         *cond_p,
  Array_t                             *dist_matrix_p,
  Array_t                             *constant_p,
  Xtr_t                               *xtr_p,
  Array_t                             *match_p
  )
{
  Boolean_t                  result;

  result = SDist (Cond_Dist_Value_Get (cond_p, 0), Cond_Dist_Base_Get (cond_p),
    Cond_Dist_FuncGrp_Get (cond_p), dist_matrix_p, constant_p, xtr_p, match_p);

  if (Cond_Dist_Sling_Get (cond_p) == TRUE)
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
      "SCond_Dist_Test Error:  this condition provides its own sling,"
      " but it is not supported by the system now."));
    }
    
  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Elecwd_Test
*
*    This routine tests the electron-withdrawing condition.
*
*  Used to be :
*    
*    elecpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t SCond_Elecwd_Test 
*/
static Zadehan_t SCond_Elecwd_Test 
  (
  Condition_t                          *cond_p,
  Xtr_t                                *xtr_p,
  Xtr_t                                *goal_xtr_p,
  Array_t                              *match_p
  )
{
  S8_t              val1;
  S8_t              val2;
  U8_t              i;
/*
  Boolean_t         result;
*/
  Zadehan_t         result;
  float             fval;

/*
  for (val1 = 0, i = 0; i < Cond_Count_Get (cond_p); i++)
    val1 += SElecwd (Cond_ElecWith_Prime_Get (cond_p, i), xtr_p, goal_xtr_p, 
      match_p);
*/
  for (fval = 0., i = 0; i < Cond_Count_Get (cond_p); i++)
    fval += SElecwd (Cond_ElecWith_Prime_Get (cond_p, i), xtr_p, goal_xtr_p, 
      match_p);
  val1 = SElecwd_Convert (fval);
   
  if (Cond_Base_Exists (cond_p) == TRUE)
/*
    val2 = (S8_t) Cond_Base_Get (cond_p);
*/
    val2 = (S8_t) Cond_Base_Get (cond_p) * ELECW_MULT;
  else
    {
/*
    for (val2 = 0, i = 0; i < Cond_Count2_Get (cond_p); i++)
      val2 += SElecwd (Cond_ElecWith_Second_Get (cond_p, i), xtr_p, 
        goal_xtr_p, match_p);
*/
    for (fval = 0., i = 0; i < Cond_Count2_Get (cond_p); i++)
      fval += SElecwd (Cond_ElecWith_Second_Get (cond_p, i), xtr_p, 
        goal_xtr_p, match_p);
    val2 = SElecwd_Convert (fval);
    }
#ifdef _FDEBUG_
printf("val1=%d val2=%d\n",(int)val1,(int)val2);
#endif
printf("val1=%d val2=%d\n",(int)val1,(int)val2);
   
  result = SCompare_Two_Values (val1, val2, Cond_Op_Get (cond_p), ELECW_MULT);
  
  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Fg_Cnt_Test
*
*    This routine tests the functional group comparison condition.
*
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Fg_Cnt_Test 
  (
  Condition_t               *cond_p, 
  Xtr_t                     *target_p,
  Array_t                   *subgxtr_p,
  U16_t                      num_xtrs
  )
{
  Xtr_t               *sx;
  U16_t                i;
  S8_t                 num_in_goal;
  S8_t                 num_in_subgoal;
/*
  Boolean_t            result;
*/
  Zadehan_t            result;
   

  if (Xtr_FuncGroups_Get (target_p) == NULL)
    Xtr_FuncGroups_Put (target_p, FuncGroups_Create (target_p));     

  num_in_goal = (S8_t) Xtr_FuncGrp_NumInstances_Get (target_p, 
    Cond_FuncGrpCnt_Get (cond_p));
  for (num_in_subgoal = 0, i = 0; i < num_xtrs; i++) 
    {
    sx = (Xtr_t *) Array_1d32_Get (subgxtr_p, i);
    if (Xtr_NumAtoms_Get (sx) > 1)
      {
      if (Xtr_FuncGroups_Get (sx) == NULL)
        Xtr_FuncGroups_Put (sx, FuncGroups_Create (sx));     

      num_in_subgoal += (S8_t) Xtr_FuncGrp_NumInstances_Get (sx, 
        Cond_FuncGrpCnt_Get (cond_p));
      }
    }

  result = SCompare_Two_Values (num_in_goal, num_in_subgoal, 
    Cond_Op_Get (cond_p), 1);
   
  return (result.fuzzy_components.TF);                                               
}

/******************************************************************************
*
*  Function Name :          SCond_Fg_Coneq_Or_Streq_Test
*
*    This routine tests the functional group constitutional equivalence
*    condition or the functional group stereochemical equivalence condition.
*
*  Used to be :
*    
*    instcepr 
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t SCond_Fg_Coneq_Or_Streq_Test 
  (
  Condition_t             *cond_p, 
  Boolean_t                constonly,
  Xtr_t                   *target_p,
  Array_t                 *subgxtr_p,
  U16_t                    num_xtrs
  )
{
  Xtr_t                       *sptr;
  U16_t                        cnode;
  U16_t                        fgroup;
  U16_t                        i;
  U16_t                        j;
  U16_t                        node1;
  U16_t                        num_inst;
  U16_t                        sgnode1;
  U16_t                        sgnode2;
  Boolean_t                    ok;

  ok = TRUE;
  if (constonly == TRUE)
    fgroup = Cond_FuncGrpCE_Get (cond_p);
  else 
    fgroup = Cond_FuncGrpSE_Get (cond_p);

  if (Cond_Goal_Get (cond_p) == TRUE)
    {
    if (Xtr_FuncGroups_Get (target_p) == NULL)
      Xtr_FuncGroups_Put (target_p, FuncGroups_Create (target_p));     

    num_inst = Xtr_FuncGrp_NumInstances_Get (target_p, fgroup);
    for (i = 2; i <= num_inst && ok; i++) 
      {
      node1 = Xtr_FuncGrp_SubstructureInstance_Get (target_p, fgroup, i-1);
      cnode = Xtr_FuncGrp_SubstructureInstance_Get (target_p, fgroup, i);
      if (Xtr_Attr_NumNeighbors_Get (target_p, node1) == 1 ||
          Xtr_Attr_NumNeighbors_Get (target_p, cnode) == 1)
        {
        ok = Xtr_Attr_NumNeighbors_Get (target_p, node1)
          == Xtr_Attr_NumNeighbors_Get (target_p, cnode)
          && Xtr_Attr_Atomid_Get (target_p, node1)
          == Xtr_Attr_Atomid_Get (target_p, cnode);

        if (ok)
          {
          /* not sure about the index ??? */
          node1 = Xtr_Attr_NeighborId_Get (target_p, node1, 0);
          cnode = Xtr_Attr_NeighborId_Get (target_p, cnode, 0);
          }
        }

      if (ok && node1 != cnode) 
        if (constonly == TRUE)
          ok = Name_ConstitutionalEquivilance_Get (target_p, node1, cnode);
        else
          ok = Name_StereochemicalEquivilance_Get (target_p, node1, cnode);
      }  /*  End of for i */ 
    }
  else
    {
    for (i = 0; i < num_xtrs && ok; i++)
      {
      sptr = (Xtr_t *) Array_1d32_Get (subgxtr_p, i);
      if (Xtr_FuncGroups_Get (sptr) == NULL)
        Xtr_FuncGroups_Put (sptr, FuncGroups_Create (sptr));     

      num_inst = Xtr_FuncGrp_NumInstances_Get (sptr, fgroup);
      for (j = 2; j <= num_inst && ok; j++) 
        /* not sure about the index ??? */
        {
        sgnode1 = Xtr_FuncGrp_SubstructureInstance_Get (sptr, fgroup, j-1);
        sgnode2 = Xtr_FuncGrp_SubstructureInstance_Get (sptr, fgroup, j);
 
        if (Xtr_Attr_NumNeighbors_Get (sptr, sgnode1) == 1
            || Xtr_Attr_NumNeighbors_Get (sptr, sgnode2) == 1)
          {
          ok = Xtr_Attr_NumNeighbors_Get (sptr, sgnode1) 
             == Xtr_Attr_NumNeighbors_Get (sptr, sgnode2)
             && Xtr_Attr_Atomid_Get (sptr, sgnode1)
             == Xtr_Attr_Atomid_Get (sptr, sgnode2);

          if (ok && sgnode1 != sgnode2)
/*
            if (constonly == TRUE)
              ok = Name_ConstitutionalEquivilance_Get (sptr, sgnode1, sgnode2);
            else
              ok = Name_StereochemicalEquivilance_Get (sptr, sgnode1, sgnode2);
*/
/* Corrects first of two problems:
   1) Testing must be unconditional - conditional block is supposed to find neighbor of terminal node, so that CE/SE strings
      contain a reference;
   2) The code should also determine equivalence among nodes in separate subgoal pieces.  (To be addressed later)
*/
            sgnode1 = Xtr_Attr_NeighborId_Get (sptr, sgnode1, 0);
            sgnode2 = Xtr_Attr_NeighborId_Get (sptr, sgnode2, 0);
          }
          if (constonly == TRUE)
            ok = Name_ConstitutionalEquivilance_Get (sptr, sgnode1, sgnode2);
          else
            ok = Name_StereochemicalEquivilance_Get (sptr, sgnode1, sgnode2);
        }  /* End of for j */
      }  /* End of for i */
    }

  return (ok);
}

/******************************************************************************
*
*  Function Name :          SCond_Fg_Xcess_Test
*
*    This routine tests the excess functional group condition.
*
*  Used to be :
*    
*    excesspr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Fg_Xcess_Test 
  (
  Condition_t               *cond_p, 
  Xtr_t                    *target_p, 
  Array_t                  *match_p,
  U16_t			    matchsize
  )
{
  Boolean_t                result;

  result = SExcess (target_p, match_p, Cond_Excess_FGNum_Get (cond_p),
    Cond_Excess_Node_Get (cond_p), Cond_Excess_Count_Get (cond_p), matchsize);

  return (result);
}

/******************************************************************************
*
*  Function Name :               SCondition 
*
*    This routine tests the specific condition.
*
*  Used to be :
*    
*    condition (somehow changed)
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t  SCondition 
*/
static U8_t  SCondition 
  (
  Condition_t                   *cond_p,
  Xtr_t                         *target_p,
  Array_t                       *subgxtr_p,
  U16_t                          num_xtrs,
  Array_t                       *subgmap_p,
  U16_t                          num_conjuncts,
  Xtr_t                         *xtr_p,
  Xtr_t                         *goal_xtr_p,
  Array_t                       *dist_matrix_p,
  Array_t                       *constant_p,
  Array_t                       *match_p,
  Array_t			*tsdmaps_p,
  U8_t			         matchsize
  )
{
  Boolean_t             result, fuzzy;
  int                   confidence_in_result;
  Zadehan_t             false, true, fuzzy_result, ambiguous;

  false.fuzzy_components.TF = 0;
  false.fuzzy_components.conf = 100;
  true.fuzzy_components.TF = 1;
  true.fuzzy_components.conf = 100;
  ambiguous.fuzzy_components.TF = 0;
  ambiguous.fuzzy_components.conf = 50;
  fuzzy = FALSE;

  if (cond_p == NULL)
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
      "SCondition Error:  attempted to test a NULL condition."));
/*
    return (FALSE);
*/
    return (Zadehan_Byteval (ambiguous));
    }

/* Fix annoying error where it doesn't matter! */
  switch (Cond_Type_Get (cond_p))
    {
    case PT_TYPE_ELECWD :
    case PT_TYPE_BULKY :
      DEBUG_ADDR (R_POSTTEST, DB_REACTREAD, cond_p);
      if (Cond_Count_Get (cond_p) + Cond_Count2_Get (cond_p)> MX_NODES)
        IO_Exit_Error (R_REACTION, X_SYNERR,
          "Too many items stuffed into a Condition_t");
      break;
    case PT_TYPE_AT_CONEQ :
    case PT_TYPE_AT_STREQ :
    case PT_TYPE_DISC_CONEQ :
    case PT_TYPE_DISC_STREQ :
      DEBUG_ADDR (R_POSTTEST, DB_REACTREAD, cond_p);
      if (Cond_Count_Get (cond_p)> MX_NODES)
        IO_Exit_Error (R_REACTION, X_SYNERR,
          "Too many items stuffed into a Condition_t");
    default :
      break;
    }


  switch (Cond_Type_Get (cond_p))
    {
    case PT_TYPE_ELECWD :
      fuzzy = TRUE;
      fuzzy_result = SCond_Elecwd_Test (cond_p, xtr_p, goal_xtr_p, match_p);
#ifdef _FDEBUG_
printf("SCondition: fuzzy=%d\n",(int)fuzzy);
printf("SCondition: fuzzy_result=%s\n",fuzzy_dump(fuzzy_result));
#endif
      break;
      
    case PT_TYPE_NUMMOLEC :
      result = SCond_Molec_Test (cond_p, num_conjuncts);
      break;

    case PT_TYPE_BULKY :
      fuzzy = TRUE;
      fuzzy_result = SCond_Bulky_Test (cond_p, xtr_p, goal_xtr_p, match_p);  
      break;

    case PT_TYPE_DIST :
      result = SCond_Dist_Test (cond_p, dist_matrix_p, constant_p, xtr_p, 
        match_p);
      break;

    case PT_TYPE_PATHLEN :
      result = SCond_Pathlen_Test (cond_p, dist_matrix_p, constant_p, 
        xtr_p, match_p);
      break;

    case PT_TYPE_ALKYNE :
      result = SCond_Alkyne_Test (cond_p, xtr_p, match_p);
      break;

    case PT_TYPE_ALLENE :
      result = SCond_Allene_Test (cond_p, xtr_p, match_p);
      break;

    case PT_TYPE_CARBONIUM :
      fuzzy = TRUE;
      fuzzy_result = SCond_Carbonium_Test (cond_p, xtr_p, goal_xtr_p, match_p);
      break;

    case PT_TYPE_LVNGROUP :
      fuzzy = TRUE;
      fuzzy_result = SCond_Lvngroup_Test (cond_p, dist_matrix_p, constant_p, 
        xtr_p, match_p);
      break;

    case PT_TYPE_MIGRATAP :
      fuzzy = TRUE;
      fuzzy_result = SCond_Migratap_Test (cond_p, dist_matrix_p, constant_p, 
        xtr_p, match_p);
      break;

    case PT_TYPE_ATOM :
      result = SCond_Atom_Test (cond_p, dist_matrix_p, constant_p, xtr_p, 
        match_p);
      break;

    case PT_TYPE_FG_XCESS :
      result = SCond_Fg_Xcess_Test (cond_p, target_p, match_p, matchsize);
      break;

    case PT_TYPE_AT_CONEQ :
      result = SCond_At_Coneq_Or_Streq_Test (cond_p, TRUE, target_p, 
        subgxtr_p, subgmap_p, match_p, tsdmaps_p);
      break;

    case PT_TYPE_AT_STREQ :
      result = SCond_At_Coneq_Or_Streq_Test (cond_p, FALSE, target_p, 
        subgxtr_p, subgmap_p, match_p, tsdmaps_p);
      break;

    case PT_TYPE_FG_CONEQ :
      result = SCond_Fg_Coneq_Or_Streq_Test (cond_p, TRUE, target_p, 
        subgxtr_p, num_xtrs);
      break;

    case PT_TYPE_FG_STREQ :
      result = SCond_Fg_Coneq_Or_Streq_Test (cond_p, FALSE, target_p, 
        subgxtr_p, num_xtrs);
      break;
  
    case PT_TYPE_DISC_CONEQ :
      result = SCond_Disc_Coneq_Test (cond_p, target_p, goal_xtr_p, match_p);
      break;

    case PT_TYPE_DISC_STREQ :
      result = SCond_Disc_Streq_Test (cond_p, target_p, goal_xtr_p, match_p);
      break;

    case PT_TYPE_FG_CNT :
      result = SCond_Fg_Cnt_Test (cond_p, target_p, subgxtr_p, num_xtrs);
      break;

    case PT_TYPE_AROMSUB :
      fuzzy = TRUE;
      fuzzy_result = SCond_Aromat_Sub_Test (cond_p, subgxtr_p, subgmap_p, match_p);
      break;

    case PT_TYPE_BRIDGEHEAD :
      result = SCond_Bridge_Head_Test (cond_p, target_p, match_p);
      break;

/*
    case PT_TYPE_UNKNOWN :
      fprintf (stderr, "NEW!!! and unknown condition type. \n");
      break;
*/
    default :
      TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
        "SCondition Error:  illegal condition type %lu.", 
        Cond_Type_Get (cond_p)));
      break;
    }

#ifdef _FDEBUG_
printf("SCondition: fuzzy=%d\n",(int)fuzzy);
#endif
  if (!fuzzy) fuzzy_result = result ? true : false;
/*
  if (Cond_Negate_Get (cond_p) == TRUE)
    result = !result;
   
  return (result);
*/
  if (Cond_Negate_Get (cond_p) == TRUE)
    fuzzy_result.fuzzy_components.TF = !fuzzy_result.fuzzy_components.TF;
#ifdef _FDEBUG_
printf("SCondition: returning %s\n",fuzzy_dump(fuzzy_result));
#endif
  return (Zadehan_Byteval (fuzzy_result));
}

/******************************************************************************
*
*  Function Name :               SCond_Lvngroup_Test 
*
*    This routine tests the leaving group ability condition.
*
*  Used to be :
*    
*    lvgrpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t  SCond_Lvngroup_Test 
*/
static Zadehan_t  SCond_Lvngroup_Test 
  (
  Condition_t 			*cond_p,
  Array_t		        *dist_matrix_p,
  Array_t			*constant_p,
  Xtr_t				*xtr_p,
  Array_t			*match_p
  )
{
  S8_t                val1;
  S8_t                val2;
/*
  Boolean_t           result;
*/
  Zadehan_t           result;

  val1 = SLvgroup (Cond_LeavingGrp_Prime_Get (cond_p),
    Cond_LeavingGrp_PrimeDist_Get (cond_p), 
    Cond_LeavingGrp_PrimePh_Get (cond_p), dist_matrix_p, constant_p, xtr_p,
    match_p);

  if (Cond_Base_Exists (cond_p) == TRUE)
    val2 = (S8_t) Cond_Base_Get (cond_p);
  else
    val2 = SLvgroup (Cond_LeavingGrp_Second_Get (cond_p),
      Cond_LeavingGrp_SecondDist_Get (cond_p),
      Cond_LeavingGrp_SecondPh_Get (cond_p), dist_matrix_p, constant_p,
      xtr_p, match_p);
    
  result = SCompare_Two_Values (val1, val2, Cond_Op_Get (cond_p), LVGRP_MULT);
 
  return (result);
}

/******************************************************************************
*
*  Function Name :          SCond_Migratap_Test
*
*    This routine tests the migratory ability condition.
*
*  Used to be :
*    
*    mgaptpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static Boolean_t  SCond_Migratap_Test 
*/
static Zadehan_t  SCond_Migratap_Test 
  (
  Condition_t 				*cond_p,
  Array_t			       *dist_matrix_p,
  Array_t				*constant_p,
  Xtr_t					*xtr_p,
  Array_t				*match_p
  )
{
  S8_t                  val1;
  S8_t                  val2;
/*
  Boolean_t             result;
*/
  Zadehan_t             result;

  val1 = SMigapt (Cond_MigratoryApt_Prime_Get (cond_p), dist_matrix_p,
    constant_p, xtr_p, match_p); 
   
  if (Cond_Base_Exists (cond_p) == TRUE)
    val2 = (S8_t) Cond_Base_Get (cond_p);
  else
    val2 = SMigapt (Cond_MigratoryApt_Second_Get (cond_p), dist_matrix_p, 
      constant_p, xtr_p, match_p);

  result = SCompare_Two_Values (val1, val2, Cond_Op_Get (cond_p), MGAPT_MULT);

  return (result);
}

/******************************************************************************
*
*  Function Name :         SCond_Molec_Test
*
*    This routine tests the number of reacting molecules condition.
*
*  Used to be :
*    
*    molecpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Molec_Test 
  (
  Condition_t *cond_p, 
  U16_t num_conjuncts
  )
{
  switch (Cond_NumMolecules_Get (cond_p))
    {
    case 1 :
      return (num_conjuncts == 1);
    case 2 :
      return (num_conjuncts == 2);
    case 3 :
      return (num_conjuncts == 3);
    case 4 :
      return (num_conjuncts == 4); 
    default :
      TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
        "SCond_Molec_Test Error:  invalid molecularity %hu.", 
        Cond_NumMolecules_Get (cond_p)));
      return (FALSE);
    }
}

/******************************************************************************
*
*  Function Name :          SCond_Pathlen_Test
*
*    This routine tests the path length condition.
*
*  Used to be :
*    
*    ringpr
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SCond_Pathlen_Test
  (
  Condition_t                   *cond_p, 
  Array_t                       *dist_matrix_p,
  Array_t                       *constant_p,
  Xtr_t                         *xtr_p,
  Array_t                       *match_p
  )
{
/*
  Boolean_t             result;
*/
  Zadehan_t             result;

  if (SConn (Cond_Path_Prime_Get (cond_p, 0), Cond_Path_Prime_Get (cond_p, 1),
      xtr_p, constant_p, match_p) == FALSE)
    return (FALSE);
   
  if (Cond_Path_Connected_Get (cond_p) == TRUE)
    return (TRUE);

  if (Cond_Base_Exists (cond_p) == TRUE)
    {
    result = SCompare_Two_Values (SFdist (Cond_Path_Prime_Get (cond_p, 0),
      Cond_Path_Prime_Get (cond_p, 1), dist_matrix_p, constant_p, xtr_p,
      match_p), (S8_t) Cond_Base_Get (cond_p), Cond_Op_Get (cond_p), 1);
    return (result.fuzzy_components.TF);
    }

  /*  Does not call SConn on the Second path (DK).  */
  result = SCompare_Two_Values (SFdist (Cond_Path_Prime_Get (cond_p, 0),
    Cond_Path_Prime_Get (cond_p, 1), dist_matrix_p, constant_p, xtr_p,
    match_p), SFdist (Cond_Path_Second_Get (cond_p, 0), 
    Cond_Path_Second_Get (cond_p, 1), dist_matrix_p, constant_p, xtr_p,
    match_p), Cond_Op_Get (cond_p), 1);

  return (result.fuzzy_components.TF);
}

/******************************************************************************
*
*  Function Name :          SConn
*
*    Node1 and node2 must be variable node numbers in the goal pattern.
*    Suppose that node1 and node2 is matched to atom1 and atom2 in the 
*    compound molecule, then SConn(node1, node2) returns TRUE if there
*    is a path in the compound molecule connecting atom1 and atom2
*    which traverses only fragment atoms.
*
*  Used to be :
*    
*    conn
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate Memory
*    Deallocate Memory
*
******************************************************************************/

static Boolean_t  SConn
  (
  U8_t                                   node1, 
  U8_t                                   node2, 
  Xtr_t                                 *xtr_p, 
  Array_t				*constant_p,
  Array_t                               *match_p
  )
{
  Array_t                       marked;
  U8_t                          atom1;
  U8_t                          atom2;
  Boolean_t                     result;

  atom1 = Array_1d16_Get (match_p, node1);
  atom2 = Array_1d16_Get (match_p, node2);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&marked", "posttest{10}", &marked, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
  Array_Set (&marked, FALSE);
  result = SConnect2 (atom1, atom2, xtr_p, constant_p, &marked);
  mind_Array_Destroy ("&marked", "posttest", &marked);
#else
  Array_1d_Create (&marked, Xtr_NumAtoms_Get (xtr_p), BITSIZE);
  Array_Set (&marked, FALSE);
  result = SConnect2 (atom1, atom2, xtr_p, constant_p, &marked);
  Array_Destroy (&marked);
#endif

  return (result); 
}

/******************************************************************************
*
*  Function Name :          SConnect2
*
*    This routine returns TRUE if a fragment path exists between node and 
*    atom2.
*
*  Used to be :
*    
*    connect2
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SConnect2 
  (
  U8_t                          node,
  U8_t                          atom2,
  Xtr_t                        *xtr_p,
  Array_t                      *constant_p,
  Array_t                      *marked_p
  )
{
  U8_t                          degree;
  U8_t                          k;
  U8_t                          neighbor;
  
  Array_1d1_Put (marked_p, node, TRUE);

  degree = Xtr_Attr_NumNeighbors_Get (xtr_p, node);
  for (k = 0; k < degree; k++)
    {
    neighbor = Xtr_Attr_NeighborId_Get (xtr_p, node, k);
    if (neighbor == atom2)
       return (TRUE);
     
    if (Array_1d1_Get (marked_p, neighbor) == FALSE && 
	Array_1d1_Get (constant_p, neighbor) == FALSE)
      if (SConnect2 (neighbor, atom2, xtr_p, constant_p, marked_p) == TRUE)
        return (TRUE);
    }

  return (FALSE);  
}


/******************************************************************************
*
*  Function Name :          SConnect_Nodes
*
*    This routine tests whether "node1" and "node2" are connected.
*
*  Used to be :
*    
*    N/A (similar to neighbor_node)
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SConnect_Nodes 
  (
  U16_t				 node1,
  U16_t				 node2,
  Tsd_t 			*tsd_p,
  Boolean_t			*visit
  )
{
  U16_t				 neighbor;
  U8_t				 i;

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    neighbor = Tsd_Atom_NeighborId_Get (tsd_p, node1, i);
    if (neighbor != TSD_INVALID && !visit[neighbor])
      {
      visit[neighbor] = TRUE;
      if (neighbor == node2 || SConnect_Nodes (neighbor, node2, tsd_p, visit))
        {
        visit[neighbor] = FALSE;
        return (TRUE);
        }

      visit[neighbor] = FALSE;
      }
    }

  return (FALSE);
}

/******************************************************************************
*
*  Function Name :          SDist
*
*    This routine tests whether an attribute can be found at a given 
*    distance from the root atom of a fragment.
*
*  Used to be :
*    
*    dist
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SDist 
  (
  U8_t                          distance, 
  U8_t                          node, 
  U16_t                         attr,
  Array_t                      *dist_matrix_p,
  Array_t                      *constant_p,
  Xtr_t                        *xtr_p,
  Array_t                      *match_p
  )
{
  return (SDist_Or_SAtmdist (distance, node, attr, TRUE, dist_matrix_p, 
    constant_p, xtr_p, match_p));
}

/******************************************************************************
*
*  Function Name :             SDist_Or_SAtmdist
*
*    This routine tests SDist or SAtmdist depending on "which_dist".
* 
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate Memory
*    Deallocate Memory
*
******************************************************************************/

static Boolean_t  SDist_Or_SAtmdist 
  (
  U8_t                                distance, 
  U8_t                                node, 
  U16_t                               attribute,
  Boolean_t                           which_dist,
  Array_t                            *dist_matrix_p,
  Array_t                            *constant_p,
  Xtr_t                              *xtr_p,
  Array_t                            *match_p
  )
{
  SCandidate_t                       *candidates_p;
  SCandidate_t                       *cand_p;
  Boolean_t                           found;

  candidates_p = SGet_Candidate_List (distance, node, dist_matrix_p, 
    constant_p, xtr_p, match_p);
  cand_p = candidates_p;  
  found = FALSE;
  while (cand_p != NULL && !found)
    {
    if (attribute <= NUM_OF_SINGLE_ATTRIBUTES)
      found = SAsearch (xtr_p, attribute, cand_p->node, which_dist,
        node, distance, constant_p, dist_matrix_p);
    else
      {
      TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
        "SDist_Or_SAtmdist Error:  multiple attribute numbers is"
        " not supported."));
      }

    cand_p = cand_p->next;
    }
 
  SFree_Candidate_List (candidates_p);
 
  return (found); 
}

/******************************************************************************
*
*  Function Name :             SElarsub
* 
*    This routine ???.
*
*  Used to be :
*    
*    elarsub
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SElarsub 
  (
  Xtr_t					*subgoal_xtr_p,
  U16_t					*ring_nodes,
  float					*ratings,
  S16_t					*standings,
  S16_t					*nties,
  S16_t					*nce_ties,
  Boolean_t				 honly
  )
{
  float					 ind[6];
  float					 res[6];
  S16_t					 o_effect[6];
  S16_t					 min_standing;
  S16_t					 x_rating;
  S16_t					 y_rating;
  Boolean_t				 pi[6];
  Boolean_t				 non_h[6];
  Boolean_t				 between_m_groups[6];
  Boolean_t				 ok;
  U8_t					 i;
  U8_t					 j;
  U8_t					 m1;
  U8_t					 m2;
  U8_t					 o1;
  U8_t					 o2;
  U8_t					 p;
S16_t max_standing;
float min_rating, max_rating, rating_diff;

  for (i = 0; i < 6; i++)
    {
    ratings[i] = 999.0;
    standings[i] = 999;
    nties[i] = 0;
    nce_ties[i] = 0;
    o_effect[i] = 0;
    between_m_groups[i] = FALSE;
    }

  ok = SCalc_Efx_Etc (subgoal_xtr_p, ring_nodes, ind, res, pi, non_h);
  if (ok == FALSE)
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MINOR, (outbuf,
      "SElarsub Warning:  substrate does not meet all requirements"
      " - must be isolated (non-fused) benzene ring - all nodes failed."));
    return (FALSE);
    }

  for (i = 0; i < 6; i++)
    {
    o1 = (i + 1) % 6;
    m1 = (i + 2) % 6;
    p = (i + 3) % 6;
    m2 = (i + 4) % 6;
    o2 = (i + 5) % 6;

    if (!non_h[i] || !honly)
      {
      between_m_groups[i] = non_h[o1] && non_h[o2];
      if ((res[o1] < 0 || res[p] < 0) && res[o2] > 0)
        o_effect[i] = 1;
      
      if ((res[o2] < 0 || res[p] < 0) && res[o1] > 0)
        o_effect[i]++;

      ratings[i] = res[p] + 0.4 * (ind[m1] + ind[m2])
        + 0.2 * (res[m1] + res[m2] + ind[p]) + 0.5 * (ind[o1] + ind[o2])
        + 0.9 * (res[o1] + res[o2]);

      if (pi[p] || pi[o1] || pi[o2])
        ratings[i] -= 0.1;
      else
        if (pi[m1] || pi[m2])
          ratings[i] += 0.05;
      }
#ifdef _DEBUG_
printf("i=%d: ratings=%0.2f between_m_groups=%d\n",i,ratings[i],between_m_groups[i]);
for (j=0; j<6; j++) if (j!=i) printf("\tres[%d]=%0.2f ind[%d]=%0.2f pi[%d]=%d\n",j,res[j],j,ind[j],j,pi[j]);
#endif
    }  /*  End of for i */

  for (i = 0; i < 6; i++)
    {
    if (!non_h[i] || !honly)
      {
      x_rating = 27 * o_effect[i] - (S16_t) (45 * ratings[i]) 
        - 9 * (between_m_groups[i] ? 1 : 0);
      standings[i] = 1;
      for (j = 0; j < 6; j++)
        if ((!non_h[j] || !honly) && j != i)
          {
          y_rating = 27 * o_effect[j] - (S16_t) (45 * ratings[j])
            - 9 * (between_m_groups[j]? 1 : 0);
          if (y_rating > x_rating)
            standings[i]++;
          else 
            if (y_rating == x_rating)
              {
              nties[i]++;
              if (Name_ConstitutionalEquivilance_Get (subgoal_xtr_p,
                  ring_nodes[i], ring_nodes[j]) == TRUE)
                nce_ties[i]++;
              }
          }
      }
    }  /* End of for i */

  /* Adjust standings to eliminate gaps in ordinality. */
  for (i = 1; i < 7; i++)
    {
    min_standing = 6;
    for (j = 0; j < 6; j++)
      if (standings[j] >= i) 
        min_standing = MIN (min_standing, standings[j]);
   
    for (j = 0; j < 6; j++)
      if (standings[j] == min_standing)
        standings[j] = i;     
    }

/* NEW CODE */
  /* Readjust standings in proportion to ratings and to treat ratings as the approximation they are! */
  min_standing = 6;
  max_standing = 1;
  min_rating = 10.;
  max_rating = -10.;
  for (i = 0; i < 6; i++) if (ratings[i] != 999. && standings[i] != 999)
  {
    min_standing = MIN (min_standing, standings[i]);
    max_standing = MAX (max_standing, standings[i]);
    min_rating = MIN (min_rating, ratings[i]);
    max_rating = MAX (max_rating, ratings[i]);
  }
  if (max_standing > 1)
  {
    rating_diff = max_rating - min_rating;
    if (rating_diff < .04)
    {
      max_standing = 1;
      for (i = 0; i < 6; i++) if (ratings[i] != 999. && standings[i] != 999)
        standings[i] = 1;
    }
    else if (rating_diff < .08) max_standing = 2;
    else if (rating_diff < .12) max_standing = 3;
    else if (rating_diff < .16) max_standing = 4;
    else if (rating_diff < .2) max_standing = 5;
    else max_standing = 6;
    if (max_standing > 1) for (i = 0; i < 6; i++) if (ratings[i] != 999. && standings[i] != 999)
      standings[i] = 1 + (int) ((ratings[i] - min_rating) * (float) (max_standing - 1) / rating_diff + .499);
  }

  return (ok);
}

/******************************************************************************
*
*  Function Name :             SElec
* 
*    This routine returns estimates for the inductive and resonant effects of
*    a variable in a reaction pattern after the reaction pattern has been
*    matched to a goal compound.
*
*  Used to be :
*    
*    elec
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void  SElec 
  (
  U8_t                             pattern_node, 
  float                           *inductive, 
  float                           *resonant,
  Xtr_t                           *xtr_p,
  Xtr_t                           *goal_xtr_p,
  Array_t                         *match_p
  )
{
  U8_t                            *name;
  U16_t                            pattern_base;
  U16_t                            compound_base;

  if (Xtr_Attr_NumNeighbors_Get (goal_xtr_p, pattern_node) != 1)
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_TRACE, (outbuf,
      "SElec Error:  ambiguous input, pattern node not degree 1."));
    fprintf (stderr, "SElec Error:  ambiguous input, pattern node not degree 1 in schema %d.\n", global_schema);
    exit (-1);
    }

  pattern_base = Xtr_Attr_NeighborId_Get (goal_xtr_p, pattern_node, 0);
  compound_base = Array_1d16_Get (match_p, pattern_base);

  name = SFragnam (xtr_p, compound_base, Xtr_Attr_NeighborIndex_Find (xtr_p, 
    compound_base, Array_1d16_Get (match_p, pattern_node)));
  SIndefct ((char *)name, inductive, resonant);
#ifdef _MIND_MEM_
  mind_free ("name", "posttest", name);
#else
  Mem_Dealloc (name, strlen ((char *)name) + 1, GLOBAL);
#endif
 
  return; 
}

/******************************************************************************
*
*  Function Name :             SElecwd
* 
*    This routine returns the electron withdrawing value for a node in the 
*    goal pattern. Four categories are possible :
*         Strongly electron withdrawing ------------ return 2
*         Weakly electron withdrawing   ------------ return 1
*         Electronically neutral        ------------ return 0 
*         Electron donating             ------------ return -1
*
*  NOTE: The above is no longer true.  The conversion to int is done AFTER
*        the float values are summed.  This makes a lot more sense, and it
*        is particularly necessary if fuzzy values are to be used sensibly.
*        The conversion is deferred until a call to the new function
*        SElecwd_Convert.
*
*  Used to be :
*    
*    elecwd
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

/*
static S8_t  SElecwd 
*/
static float SElecwd
  (
  U8_t                                pattern_node,
  Xtr_t                              *xtr_p,
  Xtr_t                              *goal_xtr_p,
  Array_t                            *match_p
  )
{
  float                 inductive;
  float                 resonant;

  SElec (pattern_node, &inductive, &resonant, xtr_p, goal_xtr_p, match_p);
  return (inductive+resonant);
}

static S8_t SElecwd_Convert (float indres)
{
#ifdef _FDEBUG_
printf("ind+res=%0.2f\n",indres);
#endif
printf("ind+res=%0.2f\n",indres);
  if (indres >= 0.64) 
    return (25);
/*
  if (inductive + resonant >= 0.40) 
    return (2);
*/
  else if (indres >= 0.40)
    return (20 + (int) (20. * (indres - .4)));
  else if (indres >= 0.08)
/*
    return (1);
*/
    return (10 + (int) (3.0 * (indres - .08)));
/*
  else if (indres >= 0.04)
    return (10 + (int) (125. * (indres - .08)));
  else if (indres >= 0.00)
    return ((int) (125. * (indres)));
  else if (indres >= -0.05)
    return (0);
*/
  else if (indres >= .04) return (5 + (int) (250. * (indres - .06)));
  else if (indres >= -0.05) return (0);
  else if (indres >= -0.1) return (-5 + (int) (167. * (indres + .08)));
/*
  else if (inductive + resonant >= -0.10)
    return (0);
*/
/*
  else if (indres >= -0.15)
    return ((int) (100. * (indres + .05)));
*/
  else if (indres >= -0.2) return ((int) (-15 + (int) (70. * (indres + .2))));
  else 
/*
    return (-1);  
*/
    return (-15);  
}

/******************************************************************************
*
*  Function Name :          SExcess
*
*    This routine counts the number of instances of the indicated fgroup
*    that are reachable from the indicated node without traversing any
*    bonds between atoms indicated by the match map. If the count exceeds
*    the indicated limit then TRUE is returned.
*
*  Used to be :
*    
*    excess
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static Boolean_t  SExcess 
  ( 
  Xtr_t                          *target_p,
  Array_t                        *match_p,
  U16_t                           attribute, 
  U8_t                            node, 
  U8_t                            limit,
  U16_t			          matchsize
  )
{
  Array_t			inst;
  Array_t			marked;
  U16_t                         i;
  U16_t				num_instances;
  U16_t                         num_of_atoms;
  U8_t                          count;

  if (Xtr_FuncGroups_Get (target_p) == NULL)
    Xtr_FuncGroups_Put (target_p, FuncGroups_Create (target_p));

  if (attribute > Xtr_FuncGrp_NumSubstructures_Get (target_p))
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
      "SExcess Error:  attribute# is too large, returning FALSE."));
    return (FALSE);
    }

  if (node >= matchsize)
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
      "SExcess Error:  node identifying fragment is out of range of"
      " matchmap, returning FALSE."));
    return (FALSE);
    }

  if (Xtr_FuncGrp_NumInstances_Get (target_p, attribute) == 0)
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "SExcess Alert:  no instances of fgroup in goal molecule,"
      " returning FALSE."));
    return (FALSE);
    }

  num_of_atoms = Xtr_NumAtoms_Get (target_p);
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&marked", "posttest{11}", &marked, num_of_atoms, BITSIZE);
  Array_Set (&marked, FALSE);
  for (i = 0; i < matchsize; i++)
    Array_1d1_Put (&marked, Array_1d16_Get (match_p, i), TRUE);

  mind_Array_1d_Create ("&inst", "posttest{11}", &inst, num_of_atoms, BITSIZE);
  Array_Set (&inst, FALSE);
  num_instances = Xtr_FuncGrp_NumInstances_Get (target_p, attribute); 
  for (i = 1; i <= num_instances; i++)
    Array_1d1_Put (&inst, 
      Xtr_FuncGrp_SubstructureInstance_Get (target_p, attribute, i), TRUE);

  count = 0;
  SVisit (Array_1d16_Get (match_p, node), &count, &marked, &inst, target_p);
  mind_Array_Destroy ("&inst", "posttest", &inst);
  mind_Array_Destroy ("&marked", "posttest", &marked);
#else
  Array_1d_Create (&marked, num_of_atoms, BITSIZE);
  Array_Set (&marked, FALSE);
  for (i = 0; i < matchsize; i++)
    Array_1d1_Put (&marked, Array_1d16_Get (match_p, i), TRUE);

  Array_1d_Create (&inst, num_of_atoms, BITSIZE);
  Array_Set (&inst, FALSE);
  num_instances = Xtr_FuncGrp_NumInstances_Get (target_p, attribute); 
  for (i = 1; i <= num_instances; i++)
    Array_1d1_Put (&inst, 
      Xtr_FuncGrp_SubstructureInstance_Get (target_p, attribute, i), TRUE);

  count = 0;
  SVisit (Array_1d16_Get (match_p, node), &count, &marked, &inst, target_p);
  Array_Destroy (&inst);
  Array_Destroy (&marked);
#endif
  return (count > limit);    
}

/******************************************************************************
*
*  Function Name :          SFdist
*
*    This routine calculates the minimal distance between two fragment roots.
*
*  Used to be :
*    
*    fdist
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Memory Allocate
*
******************************************************************************/

static S8_t  SFdist 
  (
  U8_t                          node1, 
  U8_t                          node2, 
  Array_t                       *dist_matrix_p,
  Array_t                       *constant_p,
  Xtr_t                         *xtr_p,
  Array_t                       *match_p
  )
{
  SDistance_t             *dist_p;
  U16_t                    i;
  U16_t                    j;
  U16_t                    neighbor;
  U8_t                     atom1;
  U8_t                     atom2;
  U8_t                     degree;
  U8_t                     k;

  atom1 = Array_1d16_Get (match_p, node1);
  atom2 = Array_1d16_Get (match_p, node2);
  if (Array_1d32_Get (dist_matrix_p, node1) == (U32_t) NULL)
    {
    Array_1d32_Put (dist_matrix_p, node1, 
      SBuild_Distance_Structure (Xtr_NumAtoms_Get (xtr_p), atom1));
    }
    
  dist_p = (SDistance_t *) Array_1d32_Get (dist_matrix_p, node1);
  for (i = SDist_Depth_Get (dist_p); i < Xtr_NumAtoms_Get (xtr_p) &&
      Array_1d16_Get (SDist_Distance_Get (dist_p), atom2) == INVALID_NUM;
      ++i)
    {
    for (j = 0; j < Xtr_NumAtoms_Get (xtr_p); j++)
      if (Array_1d16_Get (SDist_Distance_Get (dist_p), j) == i)
        {
        degree = Xtr_Attr_NumNeighbors_Get (xtr_p, j);
        for (k = 0; k < degree; k++)
          {
          neighbor = Xtr_Attr_NeighborId_Get (xtr_p, j, k);
          if (Array_1d16_Get (SDist_Distance_Get (dist_p), neighbor) == 
	      INVALID_NUM)
            if (Array_1d1_Get (constant_p, neighbor) == FALSE) 
               Array_1d16_Put (SDist_Distance_Get (dist_p), neighbor, i + 1);
          }
        }

    SDist_Depth_Put (dist_p, i + 1);
    }  /* End of for i */
 
  if (Array_1d16_Get (SDist_Distance_Get (dist_p), atom2) == INVALID_NUM)
    {
    TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
      "SFdist Error:  invalid distance for atom %u.", atom2));
    }

  return (Array_1d16_Get (SDist_Distance_Get (dist_p), atom2));
}


/******************************************************************************
*
*  Function Name :               SFind_Best_Lower
*
*    This routine ???
*
*  Used to be :
*    
*    find_best_lower
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static void  SFind_Best_Lower 
  (
  U8_t 		       *frag_string,
  U16_t			lower_diff,
  Boolean_t		lower_diff_bndmult,
  S16_t		        lower_index,
  S16_t                *lower_row
  )
{
  char		       *sub_string;
  char		       *sub_string1;
  char		       *sub_string2;
  U16_t                 same;
  U16_t			diff;
  Boolean_t		cant_find;
  Boolean_t		desired_type;
  Boolean_t		lower_ok;
  
  diff = lower_diff;
  same = (lower_diff > 0) ? lower_diff - 1 : 0;
  if (lower_diff_bndmult == TRUE)
    diff++;

  if (diff + 2 > MAX_LENGTH - 1) 
    {
    *lower_row = lower_index;
    return;
    }

  sub_string = SSub_String ((char *) frag_string, diff, 3);
  desired_type = type[inverse_importance[atoi (sub_string)]];
#ifdef _MIND_MEM_
  mind_free ("sub_string", "posttest", sub_string);
#else
  Mem_Dealloc (sub_string, strlen (sub_string), GLOBAL);
#endif
  cant_find = FALSE;
  lower_ok = FALSE;
  for (*lower_row = lower_index; *lower_row < TABLE_LENGTH && !cant_find 
       && !lower_ok; (*lower_row)++)
    if (strlen (frag_info[*lower_row].frag_name) - 1 < diff)
      cant_find = TRUE;
    else
      {
      sub_string1 = SSub_String (frag_info[*lower_row].frag_name, 0, same);
      sub_string2 = SSub_String ((char *) frag_string, 0, same);
      if (strcmp (sub_string1, sub_string2) != 0)
        cant_find = TRUE;
      else
        if (diff + 2 > (U16_t)(strlen (frag_info[*lower_row].frag_name) - 1))
          lower_ok = TRUE;
#ifdef _MIND_MEM_
        else 
        {
        sub_string = SSub_String (frag_info[*lower_row].frag_name, diff, 3);
        if (desired_type == type[inverse_importance[atoi (sub_string)]])
          lower_ok = TRUE;
     
        mind_free ("sub_string", "posttest", sub_string);
        }

      mind_free ("sub_string1", "posttest", sub_string1);
      mind_free ("sub_string2", "posttest", sub_string2);
#else
        else 
        {
        sub_string = SSub_String (frag_info[*lower_row].frag_name, diff, 3);
        if (desired_type == type[inverse_importance[atoi (sub_string)]])
          lower_ok = TRUE;
     
        Mem_Dealloc (sub_string, strlen (sub_string) + 1, GLOBAL);
        }

      Mem_Dealloc (sub_string1, strlen (sub_string1) + 1, GLOBAL);
      Mem_Dealloc (sub_string2, strlen (sub_string2) + 1, GLOBAL); 
#endif
      }

  (*lower_row)--;
  if (lower_ok == FALSE) 
    (*lower_row)--;

  return;
}

/******************************************************************************
*
*  Function Name :               SFind_Best_Upper
*
*    This routine ???
*
*  Used to be :
*    
*    find_best_upper
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static void  SFind_Best_Upper
  (
  U8_t                  *frag_string,
  U16_t                  upper_diff,
  Boolean_t              upper_diff_bndmult,
  S16_t                  upper_index,
  Boolean_t             *upper_ok,
  S16_t                 *upper_row          
  )
{
  char                  *sub_string;
  char			*sub_string1;
  char			*sub_string2;
  U16_t                  same;
  U16_t                  diff;
  Boolean_t		 cant_find;
  Boolean_t              desired_type;

  diff = upper_diff;
  same = (diff > 0) ? diff - 1 : 0;
  if (diff >= 1000)
    {
    *upper_row = upper_index;
    *upper_ok = FALSE;
    return;
    }

  if (frag_string[diff] == '.')
    {
    *upper_row = upper_index;
    *upper_ok = TRUE;
    return;
    }

  if (upper_diff_bndmult == TRUE)
    diff++;

  if (diff + 2 > MAX_LENGTH - 1) 
    {
    *upper_row = upper_index;
    *upper_ok = FALSE;
    return;
    }

  sub_string = SSub_String ((char *) frag_string, diff, 3);
  desired_type = type[inverse_importance[atoi (sub_string)]];
#ifdef _MIND_MEM_
  mind_free ("sub_string", "posttest", sub_string);
#else
  Mem_Dealloc (sub_string, strlen (sub_string) + 1, GLOBAL);
#endif
  cant_find = FALSE;
  *upper_ok = FALSE;
  for (*upper_row = upper_index; *upper_row >= 0 && !cant_find && !(*upper_ok); 
       (*upper_row)--)
    if (strlen (frag_info[*upper_row].frag_name) - 1 < diff)
      cant_find = TRUE;
    else
      {
      sub_string1 = SSub_String (frag_info[*upper_row].frag_name, 0, same);
      sub_string2 = SSub_String ((char *) frag_string, 0, same);

      if (strcmp (sub_string1, sub_string2) != 0)
        cant_find = TRUE;
#ifdef _MIND_MEM_
      else
        {
        sub_string = SSub_String (frag_info[*upper_row].frag_name, diff, 3);
        if (desired_type == type[inverse_importance[atoi (sub_string)]])
          *upper_ok = TRUE;
     
        mind_free ("sub_string", "posttest", sub_string);
        }

      mind_free ("sub_string1", "posttest", sub_string1);
      mind_free ("sub_string2", "posttest", sub_string2);
#else
      else
        {
        sub_string = SSub_String (frag_info[*upper_row].frag_name, diff, 3);
        if (desired_type == type[inverse_importance[atoi (sub_string)]])
          *upper_ok = TRUE;
     
        Mem_Dealloc (sub_string, strlen (sub_string) + 1, GLOBAL);
        }

      Mem_Dealloc (sub_string1, strlen (sub_string1) + 1, GLOBAL);
      Mem_Dealloc (sub_string2, strlen (sub_string2) + 1, GLOBAL);
#endif
      }     

  (*upper_row)++;
  if (*upper_ok == FALSE)
    *upper_row = upper_index;

  return;
}

/******************************************************************************
*
*  Function Name :               SFind_Difference
*
*    This routine calculates the difference between frag_string and
*    frag_info[entry_index].frag_name. 
*
*  Used to be :
*    
*    find_difference
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void  SFind_Difference 
  (
  U8_t                                 *frag_string,
  S16_t                                  entry_index,
  U16_t                                *diff,
  Boolean_t                              *diff_bndmult
  )
{
  const char                           *frag_name;
  U8_t                                  i;
  U8_t                                  str_length;

  if (entry_index > TABLE_LENGTH - 1) 
    {
    *diff = 0;
    return;
    }

  frag_name = frag_info[entry_index].frag_name;
  str_length = (strlen ((char *) frag_string) < strlen (frag_name))
    ? strlen ((char *) frag_string) : strlen (frag_name);

  i = 0;
  *diff = 1000;
  *diff_bndmult = FALSE;
  while (i < str_length)
    {
    if (frag_string[i] == '.')
      {
      if (frag_name[i] != '.')
        {
        *diff = i;
        return;
        }

      i++;
      }
    else
      {
      if (frag_string[i] != frag_name[i])
        {
        if (frag_name[i] == '.')
          {
          SFind_Vvatom (frag_string, &i);
          *diff = i;
          return;
          }

        *diff = i;
        *diff_bndmult = TRUE;
        return;
        }

      i++;
      if (i + 2 > str_length - 1) 
        return;

      if (frag_string[i] == frag_name[i] && frag_string[i+1] == frag_name[i+1]
          && frag_string[i+2] == frag_name[i+2])
        i += 3;
      else
        {
        *diff = i;
        return;
        }     
      }  /*  End of else frag not '.' */
    }  /*  End of while i */

  return;
}

/******************************************************************************
*
*  Function Name :               SFind_Vvatom
*
*    This routine returns the position of the variable valence atom in 
*    frag_string.
*
*  Used to be :
*    
*    find_vvatom
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void  SFind_Vvatom 
  (
  U8_t          *frag_string, 
  U8_t          *index
  )
{
  U8_t           k;
  U8_t           num_of_dots;

  num_of_dots = 0;
  for (k = 0; k < *index; k++)
    if (frag_string[k] == '.')
      num_of_dots++;

  if (num_of_dots > 0)
    num_of_dots--;

  *index = 1;
  while (num_of_dots > 0)
    {
    *index += 3;
    while (frag_string[*index] == '.')
      (*index)++;

    (*index)++;
    num_of_dots--;
    }  

  return;
}

/******************************************************************************
*
*  Function Name :               SFragnam
*
*    This routine returns the name of the "index"th substituent of node
*    "compound_base" in the Xtr_t pointed by xtr_p.
*
*  Used to be :
*    
*    fragnam
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U8_t  *SFragnam 
  (
  Xtr_t             *xtr_p, 
  U16_t              compound_base, 
  U8_t               index
  )
{
  U8_t              *result;

  result = SFragnam_Or_SBlkfrgn (xtr_p, compound_base, index, FALSE);
   
  return (result);
}

/******************************************************************************
*
*  Function Name :               SFragnam_Or_SBlkfrgn
*
*    This routine executes "SFragnam" or "SBlkfrgn" depending on the Boolean_t
*    value of "which_case". The execution of the routine consists of 3 steps:
*      1. Build a tree for the substituent.
*      2. Sort the tree so that for the substituent of a given node, the more
*         important nodes electronically precede the less important ones.
*      3. Construct the fragment name for the sorted tree.
*      4. Free the tree.
*
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static U8_t  *SFragnam_Or_SBlkfrgn 
  (
  Xtr_t                         *xtr_p,
  U16_t                          node,
  U8_t                           index,
  Boolean_t                        which_case
  )
{
  STree_node_t                  *tree_head_p;
  U8_t                          *result;
  U16_t                          neighbor;
  U8_t                           depth;

  depth = 0;
  neighbor = Xtr_Attr_NeighborId_Get (xtr_p, node, index);
  tree_head_p = SPursue (neighbor, node, neighbor, node, &depth, xtr_p, which_case);
  SSort_Tree (tree_head_p, which_case, xtr_p);
  result = (U8_t *) SGet_Name (tree_head_p, which_case, xtr_p);
  SFree_Tree (tree_head_p);

  return (result);
}

/******************************************************************************
*
*  Function Name :               SFree_Candidate_List
*
*    This routine deallocates the candidate list.
*
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Deallocate memory
*
******************************************************************************/

static void  SFree_Candidate_List 
  (
  SCandidate_t       *candidates_p
  )
{
  SCandidate_t       *cand_p;

  while (candidates_p != NULL)
    {
    cand_p = candidates_p;
    candidates_p = candidates_p->next;
#ifdef _MIND_MEM_
    mind_free ("cand_p", "posttest", cand_p);
#else
    Mem_Dealloc (cand_p, SCANDIDATESIZE, GLOBAL); 
#endif
    }

  return;
}

/******************************************************************************
*
*  Function Name :               SFree_Distance_Matrix
*
*    This routine deallocates the distance matrix (including the SDistance_t
*    structures).
*
*  Used to be :
*    
*    N/A
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Deallocate memory
*
******************************************************************************/

static void  SFree_Distance_Matrix 
  (
  Array_t         *dist_matrix_p, 
  U16_t            pattern_size
  )
{
  SDistance_t     *dist_p;
  U16_t            count;

  for (count = 0; count < pattern_size; count++)
#ifdef _MIND_MEM_
    if (Array_1d32_Get (dist_matrix_p, count) != (U32_t) NULL)
      {
      dist_p = (SDistance_t *)Array_1d32_Get (dist_matrix_p, count);
      mind_Array_Destroy ("SDist_Distance_Get(dist_p)", "posttest", SDist_Distance_Get (dist_p));
      mind_free ("dist_p", "posttest", dist_p);
      }

  mind_Array_Destroy ("dist_matrix_p", "posttest", dist_matrix_p);
#else
    if (Array_1d32_Get (dist_matrix_p, count) != (U32_t) NULL)
      {
      dist_p = (SDistance_t *)Array_1d32_Get (dist_matrix_p, count);
      Array_Destroy (SDist_Distance_Get (dist_p));
      Mem_Dealloc (dist_p, SDISTANCESIZE, GLOBAL);
      }

  Array_Destroy (dist_matrix_p);
#endif
  return;
}

/******************************************************************************
*
*  Function Name :               SFree_Tree
*
*    This routine deallocates the tree pointed by "tree_head_p".
*
*  Used to be :
*    
*    free_tree
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Deallocate memory
*
******************************************************************************/

static void SFree_Tree
  (
  STree_node_t       *tree_head_p
  )
{
  STree_node_t       *son_node_p;
  STree_node_t       *next_son_p;

  son_node_p = tree_head_p->son_p;
  while (son_node_p != NULL)
    {
    next_son_p = son_node_p->brother_p;
    SFree_Tree (son_node_p);
    son_node_p = next_son_p;
    }

#ifdef _MIND_MEM_
  mind_free ("tree_head_p", "posttest", tree_head_p);
#else
  Mem_Dealloc (tree_head_p, STREENODESIZE, GLOBAL);
#endif
  return ;  
}

/******************************************************************************
*
*  Function Name :             SGet_Candidate_List
* 
*    This routine accepts as input the distance value, and returns a list
*    of match node candidates.
*
*  Used to be :
*    
*    get_candidate_list
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/

static SCandidate_t  *SGet_Candidate_List 
  (
  U8_t                          distance,
  U8_t                          node,
  Array_t                   	*dist_matrix_p,
  Array_t                       *constant_p,
  Xtr_t                         *xtr_p,
  Array_t                       *match_p
  )
{
  SCandidate_t                  *candidate_p;              
  SCandidate_t                  *candidate_list;
  SDistance_t                   *dist_p;
  U16_t                          atom;
  U16_t                          i;
  U16_t                          j;
  U16_t                          neighbor;
  U8_t                           degree;
  U8_t                           k;

  atom = Array_1d16_Get (match_p, node);

  if (distance == 0) 
    {
#ifdef _MIND_MEM_
    mind_malloc ("candidate_list", "posttest{12}", &candidate_list, SCANDIDATESIZE);
#else
    Mem_Alloc (SCandidate_t *, candidate_list, SCANDIDATESIZE, GLOBAL);
#endif
    candidate_list->node = atom;
    candidate_list->next = NULL;
    return (candidate_list);
    }

  if (Array_1d32_Get (dist_matrix_p, node) == (U32_t) NULL)
    Array_1d32_Put (dist_matrix_p, node, 
      SBuild_Distance_Structure (Xtr_NumAtoms_Get (xtr_p), atom));

  dist_p = (SDistance_t *)Array_1d32_Get (dist_matrix_p, node);
  if (distance > SDist_Depth_Get (dist_p))
    {
    for (i = SDist_Depth_Get (dist_p); i < distance; i++)
      {
      for (j = 0; j < Xtr_NumAtoms_Get (xtr_p); j++)
        if (Array_1d16_Get (SDist_Distance_Get (dist_p), j) == i)
          {
          degree = Xtr_Attr_NumNeighbors_Get (xtr_p, j);
          for (k = 0; k < degree; k++)
            {
            neighbor = Xtr_Attr_NeighborId_Get (xtr_p, j, k);
            if (Array_1d16_Get (SDist_Distance_Get (dist_p), neighbor) 
                == INVALID_NUM)
              if (Array_1d1_Get (constant_p, neighbor) == FALSE)
                Array_1d16_Put (SDist_Distance_Get (dist_p), neighbor, i + 1);
            }
          }

      SDist_Depth_Put (dist_p, i + 1);
      }  /* End of for i */
    }

  candidate_list = NULL;
  for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
    if (Array_1d16_Get (SDist_Distance_Get (dist_p), i) == distance)
      {
#ifdef _MIND_MEM_
      mind_malloc ("candidate_p", "posttest{12}", &candidate_p, SCANDIDATESIZE);
#else
      Mem_Alloc (SCandidate_t *, candidate_p, SCANDIDATESIZE, GLOBAL);
#endif
      candidate_p->node = i;
      candidate_p->next = candidate_list;
      candidate_list = candidate_p;
      }           

  return (candidate_list);    
}

/******************************************************************************
*
*  Function Name :               SGet_Name
*
*    This routine gets the name for "tree_node_p".
*
*  Used to be :
*    
*    get_name
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static char *SGet_Name 
  (
  STree_node_t                 *tree_head_p,
  Boolean_t                     which_case,
  Xtr_t                        *xtr_p
  )
{
  ListElement_t	               *node_p;
  ListElement_t	               *temp_p;
  List_t                       *list_p;
  STree_node_t                 *son_node_p;
  String_t                      name;	

  name = String_Create (NULL, 0);
  list_p = List_Create (LIST_NORMAL);  
  List_InsertAdd (list_p, NULL, tree_head_p);
  SAdd_Name (&name, tree_head_p, which_case, xtr_p);
  String_Concat_c (&name, ".");
  node_p = List_Front_Get (list_p);
  while (node_p != NULL)
    {
    son_node_p = (STree_node_t *) LstElem_ValueAdd_Get (node_p);
    son_node_p = son_node_p->son_p;
    while (son_node_p != NULL)
      {
      List_InsertAdd (list_p, List_Tail_Get (list_p), son_node_p); 
      SAdd_Name (&name, son_node_p, which_case, xtr_p);
      son_node_p = son_node_p->brother_p;
      }
    
    String_Concat_c (&name, ".");
    temp_p = node_p; 
    node_p = LstElem_Next_Get (node_p);
#ifdef _MIND_MEM_
    mind_free ("temp_p", "posttest", temp_p);
    }

  mind_free ("list_p", "posttest", list_p);
#else
    Mem_Dealloc (temp_p, LISTELEMENTSIZE, GLOBAL);
    }

  Mem_Dealloc (list_p, LISTSIZE, GLOBAL);
#endif

  return ((char *)String_Value_Get (name));
}

/******************************************************************************
*
*  Function Name :               SGetout
*
*    This routine begins with a single neighbor-node and sets a bit for it
*    and every other atom in that piece, so as to enable the removal of the
*    unwanted piece from the final TSD.
*
*  Used to be :
*    
*    getout
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void  SGetout 
  (
  U16_t 	node, 
  Tsd_t 	*new_tsd_p, 
  Array_t   	*out_p
  )
{
  U16_t			neighbor;
  U8_t 			i;

  Array_1d1_Put (out_p, node, TRUE);
  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    neighbor = Tsd_Atom_NeighborId_Get (new_tsd_p, node, i);
    if (neighbor != TSD_INVALID && Array_1d1_Get (out_p, neighbor) == FALSE)
      SGetout (neighbor, new_tsd_p, out_p);
    }

  return;
}

/******************************************************************************
*
*  Function Name :               SGet_Path_List
*
*    This routine returns a Path_List_t pointer for the pair of nodes.
*
*  Used to be :
*    
*    N/A (similar to path_pointer)
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Path_List_t  *SGet_Path_List 
  (
  Tsd_t				*tsd_p,
  U16_t				*nodes,
  Path_List_t			*path_list_head
  )
{
  Path_List_t			*path_list_p;
  Path_List_t			*temp_list_p;

  path_list_p = path_list_head;
  while (path_list_p != NULL)
    {
    if ((nodes[0] == path_list_p->begin_node 
        && nodes[1] == path_list_p->end_node) 
        ||(nodes[0] == path_list_p->end_node 
        && nodes[1] == path_list_p->begin_node))
      return (path_list_p);

    path_list_p = path_list_p->next;
    }

#ifdef _MIND_MEM_
  mind_malloc ("path_list_p", "posttest{13}", &path_list_p, PATHLISTSIZE);
#else
  Mem_Alloc (Path_List_t *, path_list_p, PATHLISTSIZE, GLOBAL);
#endif
  path_list_p->begin_node = nodes[0];
  path_list_p->end_node = nodes[1];
  path_list_p->num_paths = 0;
  path_list_p->pathnodes_p = NULL;
  path_list_p->next = NULL;
  SStore_Paths (tsd_p, path_list_p);
  if (path_list_head == NULL)
    path_list_head = path_list_p;
  else
    {
    temp_list_p = path_list_head;
    while (temp_list_p->next != NULL)
      temp_list_p = temp_list_p->next;

    temp_list_p = path_list_p;
    }

  return (path_list_p);
}

/******************************************************************************
*
*  Function Name :               SIllegal_Combination
*
*    This routine checks the illegal combinations of substructures for the
*    compound molecule, if any one is present, returns FALSE.
*
*  Used to be :
*    
*    illegal_conbination
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SIllegal_Combination 
  (
  Xtr_t           *xtr_p
  )
{
  U8_t             i;

  if (Xtr_FuncGroups_Get (xtr_p) == NULL)
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));     

  for (i = 0; i < NUM_OF_ILLEGAL_PAIRS; i++)  
     if (Xtr_FuncGrp_NumInstances_Get (xtr_p, illegal_pairs[i][0]) != 0 
         && Xtr_FuncGrp_NumInstances_Get (xtr_p, illegal_pairs[i][1]) != 0)
       return (TRUE);

  return (FALSE);
}

/******************************************************************************
*
*  Function Name :               SIllegal_Small_Rings
*
*    This routine checks the illegal small rings for the compound molecule,
*    if any one is present, returns FALSE. 
*
*  Used to be :
*    
*    illegal_small_rings
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SIllegal_Small_Rings 
  (
  Xtr_t 	*xtr_p
  )
{
  U8_t                       acetylenes;
  U8_t                       allenes;

  if (Xtr_Rings_Get (xtr_p) == NULL)
    Xtr_Rings_Set (xtr_p);

  if (Xtr_Rings_NumRingSys_Get (xtr_p) == 0)
    return (FALSE);

  if (Xtr_FuncGroups_Get (xtr_p) == NULL)
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));     

  acetylenes = Xtr_FuncGrp_NumInstances_Get (xtr_p, CC_TRIPLE);
  if (acetylenes > 0)
    if (SSmall_Ring (xtr_p, CC_TRIPLE, acetylenes) == TRUE)
      return (TRUE);

  allenes = Xtr_FuncGrp_NumInstances_Get (xtr_p, ALLENE);
  if (allenes > 0)
    if (SSmall_Ring (xtr_p, ALLENE, allenes) == TRUE)
      return (TRUE);

  if (SBredts_Rule_Violation (xtr_p) == TRUE)
    return (TRUE);
  
  return (FALSE);
}

/******************************************************************************
*
*  Function Name :               SIllegal_Substructure
*
*    This routine checks the illegal substructures for the compound molecule,
*    if any one is present, returns FALSE.
*
*    This routine 
*
*  Used to be :
*    
*    illegal_substructure
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SIllegal_Substructure 
  (
  Xtr_t             *xtr_p
  )
{
  U8_t               i;

  if (Xtr_FuncGroups_Get (xtr_p) == NULL)
    Xtr_FuncGroups_Put (xtr_p, FuncGroups_Create (xtr_p));     

  for (i = 0; i < NUM_OF_ILLEGAL_STRUCTURES; i++)
    if (Xtr_FuncGrp_NumInstances_Get (xtr_p, illegal_nums[i]) != 0) 
      return (TRUE);

 return (FALSE);
}

/******************************************************************************
*
*  Function Name :               SIndefct 
*
*    This routine returns estimates for the inductive and resonant effects
*    of a substituent on the basis of its fragment name.
*
*  Used to be :
*    
*    indefct
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/

static void  SIndefct 
  (
  char 		*frag_string_p, 
  float 	*inductive, 
  float 	*resonant
  )
{
  char                           *substring;
  U8_t				 *string_p;
  S16_t                           entry_index;
  S16_t                           lower_index;
  S16_t				  lower_row;
  S16_t                           upper_index;
  S16_t				  upper_row;
  U16_t                           eff_adj;
  U16_t                           lower_diff;
  U16_t                           upper_diff;
  Boolean_t                       lower_diff_bndmult;
  Boolean_t                       upper_diff_bndmult;
  Boolean_t			  upper_ok;

  eff_adj = 0;
  substring = SSub_String (frag_string_p, 0, 5); 
  if (strcmp (substring, "2  2.") == 0)
    string_p = SArom_Fusion (frag_string_p, &eff_adj);
  else
    string_p = (U8_t *)frag_string_p;

#ifdef _MIND_MEM_
  mind_free ("substring", "posttest", substring);
#else
  Mem_Dealloc ((char *)substring, strlen (substring) + 1, GLOBAL);
#endif

#ifdef _FDEBUG_
printf("string_p=\"%s\"\n",string_p);
#endif
  if (SBinary_Search (string_p, 0, TABLE_LENGTH - 1, &entry_index) == TRUE)
    {
    if (eff_adj > 0)
      {
      *inductive = frag_info[entry_index].inductive / eff_adj;
      *resonant = frag_info[entry_index].resonant / eff_adj;
      }
    else
      {
      *inductive = frag_info[entry_index].inductive;
      *resonant = frag_info[entry_index].resonant;
      }
#ifdef _FDEBUG_
printf("ind=%0.2f res=%0.2f\n",*inductive,*resonant);
#endif

    if (string_p != (U8_t *)frag_string_p)
#ifdef _MIND_MEM_
      mind_free ("string_p", "posttest", string_p);
#else
      Mem_Dealloc (string_p, strlen ((char *)string_p) + 1, GLOBAL);
#endif

    return;
    }
   
  if (strcmp ((char *)string_p, frag_info[entry_index].frag_name) > 0)
    {
    lower_index = entry_index;
    upper_index = entry_index - 1;
    }
  else
    {
    upper_index = entry_index;
    lower_index = entry_index + 1;
    }

  SFind_Difference (string_p, upper_index, &upper_diff, &upper_diff_bndmult);
  SFind_Difference (string_p, lower_index, &lower_diff, &lower_diff_bndmult);
  if (upper_diff >= lower_diff)
    SFind_Best_Upper (string_p, upper_diff, upper_diff_bndmult, 
      upper_index, &upper_ok, &upper_row);

  if (lower_diff >= upper_diff)
    SFind_Best_Lower (string_p, lower_diff, lower_diff_bndmult,
      lower_index, &lower_row);

  if (upper_diff > lower_diff)
    {
    if (eff_adj > 0)
      {
      *inductive = frag_info[upper_row].inductive / eff_adj;
      *resonant = frag_info[upper_row].resonant / eff_adj;
      }
    else
      {
      *inductive = frag_info[upper_row].inductive;
      *resonant = frag_info[upper_row].resonant;
      }

    if (string_p != (U8_t *)frag_string_p)
#ifdef _MIND_MEM_
      mind_free ("string_p", "posttest", string_p);
#else
      Mem_Dealloc (string_p, strlen ((char *)string_p) + 1, GLOBAL);
#endif

    return;
    }

  if ( (lower_diff > upper_diff) || !upper_ok )
    {
    if (eff_adj > 0)
      {
      *inductive = frag_info[lower_row].inductive / eff_adj;
      *resonant = frag_info[lower_row].resonant / eff_adj;
      }
    else
      {
      *inductive = frag_info[lower_row].inductive;
      *resonant = frag_info[lower_row].resonant;
      }
    }
  else
    {
    if (eff_adj > 0)
      {
      *inductive = ((frag_info[lower_row].inductive 
        + frag_info[upper_row].inductive) * 0.5) / eff_adj;
      *resonant = ((frag_info[lower_row].resonant 
        + frag_info[upper_row].resonant) * 0.5) / eff_adj;
      }
    else
      {
      *inductive = (frag_info[lower_row].inductive 
        + frag_info[upper_row].inductive) * 0.5;
      *resonant = (frag_info[lower_row].resonant 
        + frag_info[upper_row].resonant) * 0.5;
      }
    }

  if (string_p != (U8_t *)frag_string_p)
#ifdef _MIND_MEM_
    mind_free ("string_p", "posttest", string_p);
#else
    Mem_Dealloc (string_p, strlen ((char *)string_p) + 1, GLOBAL);
#endif

  return;
}

/******************************************************************************
*
*  Function Name :               SLookahead 
*
*    This routine examines the operator concatenation and attempts to short
*    circuit Boolean_t evaluation whereever possible.
*
*  Used to be :
*    
*    lookahead
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void  SLookahead 
  (
/*
  Boolean_t        stack_top_value, 
*/
  Zadehan_t        stack_top_value, 
  Posttest_t      *post_p, 
  U8_t            *op_p
  )
{
  S8_t             count;
  U8_t             index;
  U8_t             sym;
  Zadehan_t        true, false;

  true.fuzzy_components.TF = 1;
  true.fuzzy_components.conf = 100;
  false.fuzzy_components.TF = 0;
  false.fuzzy_components.conf = 100;
      
  if (*op_p >= Post_Length_Get (post_p)) 
    return;

  index = *op_p;
  count = 0;
  while (index < Post_Length_Get (post_p) && (count > 0 || index == *op_p))
    {
    sym = Post_Op_Get (post_p, index); 
    if (sym == OP_NOT) 
      index++;
    else 
      {
      if (sym == OP_AND || sym == OP_OR)
        {
        count--;
        index++;
        }
      else 
        if (sym != BOOLOP_EQ && sym != BOOLOP_XOR)
          {
          count++;
          index++;
          }     
        else
          return;
       }
    }  /* End of while */

  if (count == 0 && ((sym == OP_AND && Zadehan_Byteval (stack_top_value) == Zadehan_Byteval (false)) ||
      (sym == OP_OR && Zadehan_Byteval (stack_top_value) == Zadehan_Byteval (true))))
    {
    *op_p = index;
    SLookahead (stack_top_value, post_p, op_p); /* recursively call */
    }
  
  return;
}

/******************************************************************************
*
*  Function Name :               SLvgroup
*
*    This routine returns the leaving property of a particular leaving
*    group.
*
*  Used to be :
*    
*    lvgroup
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static S8_t  SLvgroup 
  (
  U8_t 				 node, 
  U8_t 				 distance, 
  U8_t 				 reaction_cond,
  Array_t		        *dist_matrix_p,
  Array_t			*constant_p,
  Xtr_t				*xtr_p,
  Array_t			*match_p
  )
{

  if (reaction_cond != PT_ACIDIC && reaction_cond != PT_BASIC
      && reaction_cond != PT_NEUTRAL)
    return (0);

  /* halogens */
  if (SDist (distance, node, 19, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (10); /* iodide */

  if (SDist (distance, node, 18, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (10); /* bromide */

  if (SDist (distance, node, 17, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (9); /* chloride */

  if (SDist (distance, node, 16, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (5); /* fluoride */

  /* sulfur */
  if (SDist (distance, node, 31, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (7); /* dialkyl sulfide */

  /* nitrogen */

  if (SDist (distance, node, 30, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (7); /* tertiary amine */

  if (SDist (distance, node, 23, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    if (reaction_cond == PT_ACIDIC)   
      return (6); /* ammonia */
    else 
      return (0); /* ammonia */

  if (SDist (distance, node, 24, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    if (reaction_cond == PT_ACIDIC)
      return (6); /* primary amine */
    else
      return (0); /* primary amine */

  if (SDist (distance, node, 25, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    if (reaction_cond == PT_ACIDIC)
      return (6); /* secondary amine */
    else
      return (0); /* secondary amine */

  /* oxygen */

  if (SDist (distance, node, 32,dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (10); /* sulfonate */

  if (SDist (distance, node, 21, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    if (reaction_cond == PT_ACIDIC)
      return (9); /* water */
    else
      return (2); /* hydroxide */

  if (SDist (distance, node, 33, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    if (reaction_cond == PT_ACIDIC)
      return (8); /* phenol */
    else
      return (3); /* phenoxide */
 
  if (SDist (distance, node, 34, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    if (reaction_cond == PT_ACIDIC)
      return (8); /* alcohol */
    else
      return (1); /* alkoxide */

  if (SDist (distance, node, 35, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (8); /* nitrate */

  if (SDist (distance, node, 36, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (8); /* phosphate */

  if (SDist (distance, node, 37, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (8); /* borate */

  if (SDist (distance, node, 15, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    if (reaction_cond == PT_ACIDIC)
      return (9); /* carboxylic acid */
    else
      return (2); /* carboxylate */

  return (0);
}

/******************************************************************************
*
*  Function Name :          SMigapt
*
*    This routine returns a number corresponding to a class of potentially
*    migrating substituents.
*
*  Used to be :
*    
*    migapt
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static S8_t  SMigapt 
  (
  U8_t 			         node,
  Array_t		       *dist_matrix_p,
  Array_t			*constant_p,
  Xtr_t				*xtr_p,
  Array_t			*match_p
  )
{
  if (SAtom (node, xtr_p, match_p) == 1)
    return (3);

  if (SDist (0, node, 3, dist_matrix_p, constant_p, 
      xtr_p, match_p) == TRUE)
    return (2);

  return (1);
}

/******************************************************************************
*
*  Function Name :          SN_Cetie
*
*    This routine ???.
*
*  Used to be :
*    
*    n_cetie
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static S16_t  SN_Cetie 
  (
  Xtr_t 			*subgoal_xtr_p, 
  U16_t 			 atom, 
  Boolean_t 			 honly
  )
{
  float				ratings[6];
  S16_t				standings[6];
  S16_t				nties[6];
  S16_t				nceties[6];
  

  if (SSetup_Ar (subgoal_xtr_p, atom, ratings, standings, nties,
      nceties, honly) == FALSE)
    return (0);
  else
    return (nceties[0]);
}




/******************************************************************************
*
*  Function Name :          SNpaths
*
*    This routine returns the total number of unique paths between the pair 
*    of nodes.
*
*  Used to be :
*    
*    npaths
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U16_t  SNpaths 
  (
  Tsd_t          *tsd_p, 
  U16_t          *nodes, 
  Path_List_t    *path_list_head)
{
  Path_List_t    *path_list_p;

  path_list_p = SGet_Path_List (tsd_p, nodes, path_list_head);
   
  return (path_list_p->num_paths);   
}

/******************************************************************************
*
*  Function Name :          SN_Tie
*
*    This routine ???.
*
*  Used to be :
*    
*    n_tie
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static S16_t  SN_Tie 
  (
  Xtr_t 			*subgoal_xtr_p,
  U16_t	 			 atom,
  Boolean_t			 honly
  )
{
  float				ratings[6];
  S16_t				standings[6];
  S16_t				nties[6];
  S16_t				nceties[6];
  
  if (SSetup_Ar (subgoal_xtr_p, atom, ratings, standings, nties,
      nceties, honly) == FALSE)
    return (0);
  else
    return (nties[0]);
}

/******************************************************************************
*
*  Function Name :          SOkring
*
*    This routine ???.
*
*  Used to be :
*    
*    okring
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SOkring 
  (
  Xtr_t                      	*subgoal_xtr_p, 
  U16_t				 atom,
  Boolean_t			 honly
  )
{
  float				ratings[6];
  S16_t				standings[6];
  S16_t				nties[6];
  S16_t				nceties[6];
  
  
  return (SSetup_Ar (subgoal_xtr_p, atom, ratings, standings, nties,
    nceties, honly));
}

/******************************************************************************
*
*  Function Name :          SPath
*
*    This routine returns a pointer to a selected path.
*
*  Used to be :
*    
*    path
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/

static U16_t  *SPath 
  (
  Tsd_t 			*tsd_p, 
  U16_t				*nodes,
  U16_t				 pathnum,
  U16_t				*size,
  Path_List_t			*path_list_head
  )
{
  Path_List_t			*path_list_p;
  Pathnodes_t			*path_p;
  U16_t				*pathnodes_p;
  U16_t				 i;

  path_list_p = SGet_Path_List (tsd_p, nodes, path_list_head);
  path_p = path_list_p->pathnodes_p;
  while (pathnum > 0)
    {
    pathnum--;
    path_p = path_p->next;
    }

#ifdef _MIND_MEM_
  mind_malloc ("pathnodes_p", "posttest{14}", &pathnodes_p, path_p->size);
#else
  Mem_Alloc (U16_t *, pathnodes_p, path_p->size, GLOBAL);
#endif
  for (i = 0; i < path_p->size; i++)
    pathnodes_p[i] = path_p->nodes[i];

  *size = path_p->size;

  return (pathnodes_p);
}

/******************************************************************************
*
*  Function Name :          SPursue
*
*    This routine builds a tree for the substituent.
*
*  Used to be :
*    
*    pursue
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/

static STree_node_t  *SPursue 
  (
  U16_t                                    node, 
  U16_t                                    father,
  U16_t                                    origin_node,
  U16_t                                    origin_father, 
  U8_t                                    *depth,
  Xtr_t                                   *xtr_p,
  Boolean_t                                which_case /* GAM 11/29/00 */
  )
{
  STree_node_t                            *tree_node_p;
  STree_node_t                            *last_son;
  STree_node_t                            *son_node_p;
  U16_t                                    neighbor;
  U8_t                                     i;
  U8_t                                     num_hnbrs; /* GAM 11/29/00 */

  (*depth)++;
  if ((*depth) > DEPTH_LIMIT)
    {
    (*depth)--;
    return (NULL);
    }

#ifdef _MIND_MEM_
  mind_malloc ("tree_node_p", "posttest{15}", &tree_node_p, STREENODESIZE);
#else
  Mem_Alloc (STree_node_t *, tree_node_p, STREENODESIZE, GLOBAL);
#endif
  tree_node_p->brother_p = NULL;
  tree_node_p->node = node;
  tree_node_p->bm = Xtr_Attr_NeighborBond_Find (xtr_p, node, father);
  tree_node_p->num_of_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, node);

  for ( i = num_hnbrs = 0, last_son = NULL; i < tree_node_p-> num_of_neighbors; i++)
    {
    neighbor = Xtr_Attr_NeighborId_Get (xtr_p, node, i);
    if (which_case && Xtr_Attr_Atomid_Get (xtr_p, neighbor) == 1) num_hnbrs++; /* GAM 11/29/00 */
    else if (neighbor != father && neighbor != origin_node 
        && neighbor != origin_father)
      {
      son_node_p = SPursue (neighbor, node, origin_node, origin_father,
        depth, xtr_p, which_case);
      if (son_node_p != NULL)
        {
        son_node_p->brother_p = last_son;
        last_son = son_node_p;
        }
      }
    }
  
/*tree_node_p->num_of_neighbors -= num_hnbrs; /* GAM 11/29/00 */
  tree_node_p->son_p = last_son;
  (*depth)--;

  return (tree_node_p);
}

/******************************************************************************
*
*  Function Name :          SRatng
*
*    This routine ???
*
*  Used to be :
*    
*    ratng
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static float  SRatng
  (
  Xtr_t 			*subgoal_xtr_p, 
  U16_t 			 atom, 
  Boolean_t 			 honly
  )
{
  float				ratings[6];
  S16_t				standings[6];
  S16_t				nties[6];
  S16_t				nceties[6];
  
  if (SSetup_Ar (subgoal_xtr_p, atom, ratings, standings, nties,
      nceties, honly) == FALSE)
    return (999.0);
  else
    return (ratings[0]);  
}

/******************************************************************************
*
*  Function Name :          SReposition
*
*    This routine copies a tsd-row from the old position into a new position
*    freed by removal of the unwanted piece and stores this new position in
*    the appropriate member of the news array.
*
*  Used to be :
*    
*    reposn
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void  SReposition 
  (
  U16_t 			begin_index, 
  U16_t 			end_index,
  Tsd_t			       *new_tsd_p,
  Array_t		       *news_p
  )
{
  U16_t				neighbor;
  U8_t				i;
  U8_t				j;

  Array_1d16_Put (news_p, end_index, begin_index);
  Tsd_RowCopy (new_tsd_p, begin_index, new_tsd_p, end_index);
  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    neighbor = Tsd_Atom_NeighborId_Get (new_tsd_p, begin_index, i);
    if (neighbor != TSD_INVALID)
      {
      j = 0;
      while (Tsd_Atom_NeighborId_Get (new_tsd_p, neighbor, j) != end_index)
        j++;
      
      Tsd_Atom_NeighborId_Put (new_tsd_p, neighbor, j, begin_index);
      }
    } 

  return;     
}

/******************************************************************************
*
*  Function Name :          SRing_System_Number
*
*    This routine returns the ring system number of the specific substructure
*    instance.
*
*  Used to be :
*    
*    ring_system_number
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U16_t  SRing_System_Number 
  (
  Xtr_t *sptr, 
  U16_t location
  )
{
   U16_t                i;
   U16_t		num_sys;

   num_sys = Xtr_Rings_NumRingSys_Get (sptr);
   for (i = 0; i < num_sys; i++)
     if (Ring_AtomInSpecific (sptr, i, location) == TRUE)
       return (i);

   return (INVALID_NUM);
}

/******************************************************************************
*
*  Function Name :          SRing_Tests_Passed
*
*    This routine ???.
*
*  Used to be :
*    
*    ring_tests_passed
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate Memory
*    Deallocate Memory
*
******************************************************************************/

static Boolean_t  SRing_Tests_Passed 
  (
  Xtr_t				*target_p,
  U16_t				 node
  )
{
  Array_t			 in_primary_rings;
  Array_t			 valid_primary_rings;
  U16_t				 i;
  U16_t				 j;
  U16_t				 neighbor_fail_count;
  U16_t				 num_primary_cycles;
  U16_t				 num_ring_sys;
  U16_t				 pass_count;
  U16_t				 ring_sys;
  Boolean_t			 neighbor_fail;
  Boolean_t			 test_passed;

  num_ring_sys = Xtr_Rings_NumRingSys_Get (target_p);
  if (num_ring_sys == 0)
    return (FALSE);

  if (Ring_AtomIn (target_p, node) == FALSE)
    return (FALSE);

  test_passed = FALSE;
  for (ring_sys = 0; !test_passed && ring_sys < num_ring_sys; ring_sys++)
    if (Ring_AtomInSpecific (target_p, ring_sys, node) == TRUE)
      {
      num_primary_cycles = Ringdef_NumPrimaryCycles_Find (target_p, ring_sys);
#ifdef _MIND_MEM_
      mind_Array_1d_Create ("&in_primary_rings", "posttest{16}", &in_primary_rings, num_primary_cycles, 
        sizeof (Boolean_t));
      mind_Array_1d_Create ("&valid_primary_rings", "posttest{16}", &valid_primary_rings, num_primary_cycles, 
        sizeof (U16_t));
#else
      Array_1d_Create (&in_primary_rings, num_primary_cycles, 
        sizeof (Boolean_t));
      Array_1d_Create (&valid_primary_rings, num_primary_cycles, 
        sizeof (U16_t));
#endif
      for (i = 0; i < num_primary_cycles; i++)
        Array_1d16_Put (&valid_primary_rings, i, INVALID_NUM);

      Ringdef_CycleMember_Set (target_p, ring_sys, node, &in_primary_rings);
      pass_count = 0;
      neighbor_fail_count = 0;
      for (i = 0; i < num_primary_cycles; i++)
        if (Array_1d1_Get (&in_primary_rings, i) == TRUE)
          {
          Array_1d16_Put (&valid_primary_rings, pass_count, i);
          pass_count++;
          }
     
      test_passed = (pass_count > 1);
      if (test_passed == TRUE)
        for (i = 0; i < Xtr_Attr_NumNeighbors_Get (target_p, node); i++)
          {
          Ringdef_CycleMember_Set (target_p,  ring_sys, 
            Xtr_Attr_NeighborId_Get (target_p, node, i), &in_primary_rings);
          neighbor_fail = TRUE;
          for (j = 0; neighbor_fail && j < num_primary_cycles; j++)
            if (Array_1d16_Get (&valid_primary_rings, j) != INVALID_NUM)
              neighbor_fail = !Array_1d1_Get (&in_primary_rings,
                Array_1d16_Get (&valid_primary_rings, j));

          if (neighbor_fail == TRUE)
            neighbor_fail_count++;
          }

      test_passed &= (neighbor_fail_count < 2);

#ifdef _MIND_MEM_
      mind_Array_Destroy ("&in_primary_rings", "posttest", &in_primary_rings);
      mind_Array_Destroy ("&valid_primary_rings", "posttest", &valid_primary_rings);
#else
      Array_Destroy (&in_primary_rings);
      Array_Destroy (&valid_primary_rings);
#endif
    }
  
  return (test_passed);
}

/******************************************************************************
*
*  Function Name :          SSediscn
*
*    This routine tests whether the nodes are all stereochemically
*    equivalent when disconnected from the goal pattern.
*
*  Used to be :
*    
*    sediscn
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t SSediscn 
  (
  Xtr_t 			*target_p, 
  U8_t 				 num_of_nodes, 
  Array_t			*nodes,
  Xtr_t				*goal_xtr_p,
  Array_t			*match_p
  )
{
  Boolean_t			result;

  result = SCediscn_Or_SSediscn (target_p, num_of_nodes, nodes, FALSE,
    goal_xtr_p, match_p);

  return (result);
}

/******************************************************************************
*
*  Function Name :           SSetup_Ar
*
*    This routine ???.
*
*  Used to be :
*    
*    setup_ar
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SSetup_Ar 
  (
  Xtr_t				*subgoal_xtr_p,
  U16_t				 node,
  float				*ratings,
  S16_t				*standings,
  S16_t				*nties,
  S16_t				*nceties,
  Boolean_t			 honly
  )
{
  U16_t                          ring_nodes[6];
  U8_t				 i;
  U8_t				 j;
  Boolean_t			 ok;

  ring_nodes[0] = node;
  for (i = 1; i < 6; i++) 
    {
    ok = FALSE;
    for (j = 0; !ok && 
         j < Xtr_Attr_NumNeighbors_Get (subgoal_xtr_p, ring_nodes[i-1]);
         j++)
      {
      if (Xtr_Attr_NeighborBond_Get (subgoal_xtr_p, ring_nodes[i-1], j) == 
          BOND_RESONANT)
        {
        ring_nodes[i] = Xtr_Attr_NeighborId_Get (subgoal_xtr_p,
          ring_nodes[i-1], j);
        ok = (i == 1);
        if (ok == FALSE)
          ok = (ring_nodes[i] != ring_nodes[i-2]);
        }
      }

    if (ok == FALSE)
      {
      TRACE (R_POSTTEST, DB_CHEMISTRY, TL_DETAIL, (outbuf,
        "Setup_Ar Error:  node is not in an aromatic ring."));
      return (FALSE);
      }
    }  /*  End of for i */

  ok = SElarsub (subgoal_xtr_p, ring_nodes, ratings, standings,
    nties, nceties, honly);

  return (ok); 
}

/******************************************************************************
*
*  Function Name :           SSmallest_Common_Ring_Size
*
*    This routine returns the size of smallest common ring  which "atom1" 
*    and "atoms" are in.
*
*  Used to be :
*    
*    smallest_common_ring_size
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static U16_t  SSmallest_Common_Ring_Size 
  (
  Xtr_t 	*xtr_p, 
  U16_t 	atom1, 
  U16_t 	atom2
  )
{
  U16_t				i;
  U16_t                         index;
  U16_t				num_of_rings;
  U16_t                         size;


  if (Ring_AtomIn (xtr_p, atom1) && Ring_AtomIn (xtr_p, atom2))
    {
    num_of_rings = Xtr_Rings_NumRingSys_Get (xtr_p);
    for (i = 0; i < num_of_rings; i++)
      {
      Ringdef_MinPrimaryRing_Find (xtr_p, i, atom1, atom2, &index, &size);
      if (size > 0) 
        return (size);
      }
    }
  
  return (0);
}

/******************************************************************************
*
*  Function Name :           SSmall_Ring
*
*    This routine tests whether small rings exist for a specific syntheme.
*
*  Used to be :
*    
*    small_ring
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static Boolean_t  SSmall_Ring 
  (
  Xtr_t 	*sptr, 
  U8_t 		syntheme, 
  U8_t 		num_of_instances
  )
{
  U16_t                    location[MAX_NUM_OF_INSTANCES];
  U16_t                    ring_system[MAX_NUM_OF_INSTANCES];
  U16_t                    ring_num;
  U16_t                    size;
  U8_t                     i;
  U8_t                     j;
  Boolean_t                ok;

  if (Xtr_FuncGroups_Get (sptr) == NULL)
    Xtr_FuncGroups_Put (sptr, FuncGroups_Create (sptr));

  for (i = 0, ok = TRUE; ok && (i < num_of_instances); i++)
    {
    location[i] = Xtr_FuncGrp_SubstructureInstance_Get (sptr, syntheme, i+1);
    ring_system[i] = SRing_System_Number (sptr, location[i]);
    for (j = 0; ok && (j < i); j++)
      if (ring_system[i] != INVALID_NUM)
        if (ring_system[i] == ring_system[j])
          {
          Ringdef_MinPrimaryRing_Find (sptr, ring_system[i], location[i],
            location[j], &ring_num, &size);
          if (ring_num != INVALID_NUM) 
            if (size < 9)
              ok = FALSE;
          }
    }

  return (!ok);  
}

/******************************************************************************
*
*  Function Name :           SSort_Tree
*
*    This routine sorts the tree of the substituent.
*
*  Used to be :
*    
*    sort
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*    Deallocate memory
*
******************************************************************************/
 
static void  SSort_Tree 
  (
  STree_node_t 		*tree_head_p, 
  Boolean_t 		which_case, 
  Xtr_t 		*xtr_p
  )
{
  STree_node_t          *son_node_p;
  List_t		*name_list_p;
  ListElement_t		*last_node_p;
  ListElement_t		*node1_p;
  ListElement_t		*node2_p;
  STree_node_t		*brother_p;
  STree_node_t		*last_brother_p;
  U16_t			i;
  U16_t			j;
  U16_t			num_brothers;
  Boolean_t             interchange;

  name_list_p = List_Create (LIST_NORMAL);
  num_brothers = 0;
  son_node_p = tree_head_p->son_p;
  while (son_node_p != NULL)
    {
    SSort_Tree (son_node_p, which_case, xtr_p);
    son_node_p = son_node_p->brother_p;
    ++num_brothers;
    }

  son_node_p = tree_head_p->son_p;
  for (i = 0; i < num_brothers; ++i)
    {
    List_InsertAdd (name_list_p, List_Tail_Get (name_list_p),
       SGet_Name (son_node_p, which_case, xtr_p));
    son_node_p = son_node_p->brother_p;
    }

  /* insert a dummy node */
  List_InsertAdd (name_list_p, NULL, NULL);

  brother_p = tree_head_p->brother_p;
  tree_head_p->brother_p = tree_head_p->son_p;
  interchange = TRUE;
  for (i = num_brothers; i >= 2 && interchange == TRUE; --i)
    {
    interchange = FALSE;
    last_brother_p = tree_head_p;
    last_node_p = List_Front_Get (name_list_p);
    node1_p = LstElem_Next_Get (last_node_p);
    node2_p = LstElem_Next_Get (node1_p); 
    for (j = 2; j <= i; ++j)
      {
      if (which_case == FALSE) 
        {
        if (strcmp ((char *)LstElem_ValueAdd_Get (node1_p), 
	    (char *)LstElem_ValueAdd_Get (node2_p)) < 0) 
          {
          SSwap_Tree_Nodes (last_brother_p, last_node_p);
          interchange = TRUE;
          }
        }
      else
        if (strcmp ((char *)LstElem_ValueAdd_Get (node1_p), 
                (char *)LstElem_ValueAdd_Get (node2_p)) > 0) 
          {
          SSwap_Tree_Nodes (last_brother_p, last_node_p);
          interchange = TRUE;
          }
      last_brother_p = last_brother_p->brother_p; 
      last_node_p = LstElem_Next_Get (last_node_p);
      node1_p = LstElem_Next_Get (last_node_p);
      node2_p = LstElem_Next_Get (node1_p);
      }
  }
  tree_head_p->son_p = tree_head_p->brother_p;
  tree_head_p->brother_p = brother_p;

  List_Destroy (name_list_p);
  return;
}

/******************************************************************************
*
*  Function Name :           SStdng
*
*    This routine ???.
*
*  Used to be :
*    
*    stdng
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static S16_t  SStdng
  (
  Xtr_t 			*subgoal_xtr_p, 
  U16_t 			 atom, 
  Boolean_t 			 honly
  )
{
  float				ratings[6];
  S16_t				standings[6];
  S16_t				nties[6];
  S16_t				nceties[6];
  

  if (SSetup_Ar (subgoal_xtr_p, atom, ratings, standings, nties,
      nceties, honly) == FALSE)
    return (999);
  else
    return (standings[0]);
/* AROM2 is not set up to handle fuzziness!
    return (10 * standings[0]);
*/
}

/******************************************************************************
*
*  Function Name :           SStore_Paths
*
*    This routine finds all distinctive paths between the pair of nodes.
*
*  Used to be :
*    
*    N/A (sort of similar to store_path)
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/
 
static void  SStore_Paths (Tsd_t *tsd_p, Path_List_t *path_list_p)
{
  Boolean_t			*visit;
  U16_t				 i;
  U16_t				*node_stack;
  U16_t				 stack_top;

  
  if (path_list_p->begin_node == path_list_p->end_node)
     return;

#ifdef _MIND_MEM_
  mind_malloc ("visit", "posttest{17}", &visit, tsd_p->num_atoms);
#else
  Mem_Alloc (Boolean_t *, visit, tsd_p->num_atoms, GLOBAL);
#endif
  for (i = 0; i < tsd_p->num_atoms; i++)
    visit[i] = FALSE;

  visit[path_list_p->begin_node] = TRUE;

#ifdef _MIND_MEM_
  if (!SConnect_Nodes (path_list_p->begin_node, path_list_p->end_node, 
                       tsd_p, visit))
    /*  Destroy visits before returning (DK).  */
        {
        mind_free ("visit", "posttest", visit);
        return;
        }

  mind_malloc ("node_stack", "posttest{17}", &node_stack, tsd_p->num_atoms);
#else
  if (!SConnect_Nodes (path_list_p->begin_node, path_list_p->end_node, 
                       tsd_p, visit))
    /*  Destroy visits before returning (DK).  */
        {
        Mem_Dealloc (visit, tsd_p->num_atoms, GLOBAL);
        return;
        }

  Mem_Alloc (U16_t *, node_stack, tsd_p->num_atoms, GLOBAL);
#endif

  node_stack[0] = path_list_p->begin_node;
  stack_top = 0;

  STrace_Paths (path_list_p->begin_node, path_list_p->end_node, tsd_p,
                visit, node_stack, &stack_top, path_list_p);

  /*  Destroy visits and node_stack before returning (DK).  */
#ifdef _MIND_MEM_
  mind_free ("visit", "posttest", visit);
  mind_free ("node_stack", "posttest", node_stack);
#else
  Mem_Dealloc (visit, tsd_p->num_atoms, GLOBAL);
  Mem_Dealloc (node_stack, tsd_p->num_atoms, GLOBAL);
#endif
  return;    
}

/******************************************************************************
*
*  Function Name :           STrace_Paths
*
*    This routines is called by SStore_Paths to find all distinctive paths
*    between the pair of nodes. 
*
*  Used to be :
*    
*    N/A (similar to trace_paths)
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/
  
static void  STrace_Paths
  (
  U16_t				 node1,
  U16_t				 node2,
  Tsd_t				*tsd_p,
  Boolean_t			*visit,
  U16_t				*node_stack,
  U16_t				*stack_top,
  Path_List_t			*path_list_p
  )
{
  Pathnodes_t			*pathnodes_p;
  Pathnodes_t			*temp_pathnodes_p;
  U16_t				 j;
  U16_t				 neighbor;
  U8_t				 i;

  for (i = 0; i < MX_NEIGHBORS; i++)
    {
    neighbor = Tsd_Atom_NeighborId_Get (tsd_p, node1, i);
    if (neighbor != TSD_INVALID && !visit[neighbor])
      {
      visit[neighbor] = TRUE;
      if (neighbor == node2) /* one path found */
        {
        (path_list_p->num_paths)++;
#ifdef _MIND_MEM_
        mind_malloc ("pathnodes_p", "posttest{18}", &pathnodes_p, PATHNODESSIZE);
        pathnodes_p->next = NULL;
        pathnodes_p->size = (*stack_top) + 2;
        mind_malloc ("pathnodes_p->nodes", "posttest{18}", &pathnodes_p->nodes, pathnodes_p->size);
#else
        Mem_Alloc (Pathnodes_t *, pathnodes_p, PATHNODESSIZE, GLOBAL);
        pathnodes_p->next = NULL;
        pathnodes_p->size = (*stack_top) + 2;
        Mem_Alloc (U16_t *, pathnodes_p->nodes, pathnodes_p->size, GLOBAL);
#endif
        for (j = 0; j < pathnodes_p->size - 1; j++)
          pathnodes_p->nodes[j] = node_stack[j];

        pathnodes_p->nodes[pathnodes_p->size - 1] = node2;
        if (path_list_p->pathnodes_p == NULL)
          path_list_p->pathnodes_p = pathnodes_p;
        else
          {
          temp_pathnodes_p = path_list_p->pathnodes_p;
          while (temp_pathnodes_p->next != NULL)
            temp_pathnodes_p = temp_pathnodes_p->next;

          temp_pathnodes_p->next = pathnodes_p;
          }
        }
      else if (SConnect_Nodes (neighbor, node2, tsd_p, visit))
        {
        (*stack_top)++;
        node_stack[*stack_top] = neighbor;
        STrace_Paths (neighbor, node2, tsd_p, visit, node_stack, stack_top,
          path_list_p);
        (*stack_top)--;
        }

      visit[neighbor] = FALSE;
      }
    }

  return;
}

/******************************************************************************
*
*  Function Name :           SSub_String
*
*    This routine returns a substring of "string" (from "start" index to
*    "last" index).
*
*    Changed SSub_String to reflect its original PL1 definition:  the 
*    routine now returns a substring of of string beginning at start 
*    and with length len.  If start is beyond the length of the string,
*    an empty string is return.  If the substring len extends beyond
*    the length of string, it is truncated.
*
*  Used to be :
*    substr (PL1 builtin)
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/
 
static char  *SSub_String 
  (
  const char *string, 
  U16_t start, 
  U16_t sublen
  )
{
  char                   *sub_string;
  U16_t                   length;

  length = strlen (string);

  if (start >= length)
    {
    /*  Return the empty string  */
#ifdef _MIND_MEM_
    mind_malloc ("sub_string", "posttest{19}", &sub_string, 1);
#else
    Mem_Alloc (char *, sub_string, 1, GLOBAL);
#endif
    *sub_string = '\0';
    return (sub_string);
    }

  if (sublen == (U16_t) INFINITY || start + sublen > length)  
    sublen = length - start;

#ifdef _MIND_MEM_
  mind_malloc ("sub_string", "posttest{19a}", &sub_string, sublen + 1);
#else
  Mem_Alloc (char *, sub_string, sublen + 1, GLOBAL);
#endif
  memcpy (sub_string, string + start, sublen);
  sub_string[sublen] = '\0';

  return (sub_string);
}

/******************************************************************************
*
*  Function Name :           SSwap_Tree_Nodes
*
*    This routine swaps two tree nodes.
*
*  Used to be :
*    
*    swap_tree_nodes
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/
 
static void  SSwap_Tree_Nodes 
  (
  STree_node_t                          *tree_p,
  ListElement_t                         *node_p
  )
{
  ListElement_t		*node2_p;
  ListElement_t		*node3_p;
  ListElement_t		*node4_p;
  STree_node_t		*tree2_p;
  STree_node_t		*tree3_p;
  STree_node_t		*tree4_p;

  node2_p = LstElem_Next_Get (node_p);
  node3_p = LstElem_Next_Get (node2_p);
  node4_p = LstElem_Next_Get (node3_p);

  LstElem_Next_Put (node2_p, node4_p);
  LstElem_Next_Put (node3_p, node2_p);
  LstElem_Next_Put (node_p, node3_p);

  tree2_p = tree_p->brother_p;
  tree3_p = tree2_p->brother_p;
  tree4_p = tree3_p->brother_p;

  tree2_p->brother_p = tree4_p;
  tree3_p->brother_p = tree2_p;
  tree_p->brother_p = tree3_p;
}

/******************************************************************************
*
*  Function Name :           SThree_Unique
*
*    This routine ???.
*
*  Used to be :
*    
*    three_unique
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate Memory
*    Deallocate Memory
*
******************************************************************************/

static Boolean_t  SThree_Unique 
  (
  Tsd_t				*tsd_p,
  U16_t				*nodes,
  U16_t				*neighbor_node,
  U16_t				 count2,
  Path_List_t			*path_list_head
  )
{
  Pathnodes_t			*node_strings;
  U16_t				*pathnodes;
  U16_t				 i;
  U16_t				 index;
  U16_t				 j;
  U16_t				 k;
  U16_t				 l;
  U16_t				 m;
  U16_t				 max_size;
  U16_t				 n;
  U16_t				 num_paths;
  U16_t				 pic3;
  U16_t				 size;
  Boolean_t			 found;
#ifdef _MIND_MEM_
  char varname[100];


  if (count2 < 3) 
    return (FALSE);

  /* for storing pathnodes */
  num_paths = SNpaths (tsd_p, nodes, path_list_head); 
  mind_malloc ("node_strings", "posttest{20}", &node_strings, count2 * sizeof (Pathnodes_t));
#else


  if (count2 < 3) 
    return (FALSE);

  /* for storing pathnodes */
  num_paths = SNpaths (tsd_p, nodes, path_list_head); 
  Mem_Alloc (Pathnodes_t *, node_strings, count2 * sizeof (Pathnodes_t), 
    GLOBAL);
#endif
  for (i = 0; i < count2; i++)
    {
    /*  Changed conditions for loop exit to ensure 
        deallocation of pathnodes (DK).  */
    node_strings[i].nodes = NULL;
    for (j = 0; node_strings[i].nodes == NULL && j < num_paths; j++)  
      {
      pathnodes = SPath (tsd_p, nodes, j, &size, path_list_head); 
#ifdef _MIND_MEM_
      if (pathnodes[1] == neighbor_node[i])
        {
        sprintf (varname, "node_strings[%d].nodes", i);
        mind_malloc (varname, "posttest{20}", &node_strings[i].nodes, size - 2);
        for (k = 1; k < size - 1; k++)
          node_strings[i].nodes[k - 1] = pathnodes[k];

        node_strings[i].size = size - 2;
        }

      mind_free ("pathnodes", "posttest", pathnodes);
#else
      if (pathnodes[1] == neighbor_node[i])
        {
        Mem_Alloc (U16_t *, node_strings[i].nodes, size - 2, GLOBAL);
        for (k = 1; k < size - 1; k++)
          node_strings[i].nodes[k - 1] = pathnodes[k];

        node_strings[i].size = size - 2;
        }

      Mem_Dealloc (pathnodes, size, GLOBAL);
#endif
      }  /*  End of for j */
    }  /*  End of for i */
  
  if (count2 == 4)
    {
    found = FALSE;
    for (i = 0; !found && (i < 4); i++)
      for (j = 0; !found && (j < node_strings[i].size); j++)
        {
        pic3 = node_strings[i].nodes[j];     
        for (k = i+1; !found && (k < 4); k++)
          for (l = 0; !found && (l < node_strings[k].size); l++)
            if (pic3 == node_strings[k].nodes[l])
              {
              found = TRUE;
              if (node_strings[i].size > node_strings[k].size) 
                m = i;
              else
                m = k;

              for (n = m+1; n < 4; n++)
                {
                neighbor_node[n-1] = neighbor_node[n];                
#ifdef _MIND_MEM_
                sprintf (varname, "node_strings[%d].nodes", n - 1);
                mind_free (varname, "posttest", node_strings[n-1].nodes);
                node_strings[n-1].size = node_strings[n].size; 
                mind_malloc (varname, "posttest{20a}", &node_strings[n-1].nodes, node_strings[n-1].size);
#else
                Mem_Dealloc (node_strings[n-1].nodes, node_strings[n-1].size, 
                  GLOBAL);
                node_strings[n-1].size = node_strings[n].size; 
                Mem_Alloc (U16_t *, node_strings[n-1].nodes,
                           node_strings[n-1].size, GLOBAL);
#endif
                for (index = 0; index < node_strings[n-1].size; index++)
                  node_strings[n-1].nodes[index] = node_strings[n].nodes[index];
                }
              }
        }  /* End of for j */

    if (!found)
      {
      max_size = 0;
      for (i = 0; i < 4; i++)
        if (max_size < node_strings[i].size)
          max_size = node_strings[i].size;

      for (i = 0; !found && (i < 4); i++)
        if (node_strings[i].size == max_size)
          {
          found = TRUE;
          for (j = i + 1; j < 4; j++)
            {
            neighbor_node[j-1] = neighbor_node[j];
#ifdef _MIND_MEM_
            sprintf (varname, "node_strings[%d].nodes", j - 1);
            mind_free (varname, "posttest", node_strings[j-1].nodes);
            node_strings[j-1].size = node_strings[j].size;
            mind_malloc (varname, "posttest{20b}", &node_strings[j-1].nodes, node_strings[j-1].size);
#else
            Mem_Dealloc (node_strings[j-1].nodes, node_strings[j-1].size, 
              GLOBAL);
            node_strings[j-1].size = node_strings[j].size;
            Mem_Alloc (U16_t *, node_strings[j-1].nodes,
                       node_strings[j-1].size, GLOBAL);
#endif
            for (index = 0; index < node_strings[j-1].size; index++)
              node_strings[j-1].nodes[index] = node_strings[j].nodes[index];
            }
          }
      }  /* End of if not found */
    }  /* End of if count2 == 4 */

#ifdef _MIND_MEM_
  for (i = 0; i < 3; i++)
    for (j = 0; j < node_strings[i].size; j++)
      {
      pic3 = node_strings[i].nodes[j];
      for (k = i+1; k < 3; k++)
        for (l = 0; l < node_strings[k].size; l++)
          if (pic3 == node_strings[k].nodes[l])
            {
            for (index = 0; index < count2; index++)
              {
              sprintf (varname, "node_strings[%d].nodes", index);
              mind_free (varname, "posttest", node_strings[index].nodes);
              }
            mind_free ("node_strings", "posttest", node_strings);
            return (FALSE);
            }
      }  /* End of for j */

  for (index = 0; index < count2; index++)
     {
     sprintf (varname, "node_strings[%d].nodes", index);
     mind_free (varname, "posttest", node_strings[index].nodes);
     }

  mind_free ("node_strings", "posttest", node_strings);
#else
  for (i = 0; i < 3; i++)
    for (j = 0; j < node_strings[i].size; j++)
      {
      pic3 = node_strings[i].nodes[j];
      for (k = i+1; k < 3; k++)
        for (l = 0; l < node_strings[k].size; l++)
          if (pic3 == node_strings[k].nodes[l])
            {
            for (index = 0; index < count2; index++)
              Mem_Dealloc (node_strings[index].nodes, node_strings[index].size, 
                GLOBAL);
            Mem_Dealloc (node_strings, count2 * sizeof (Pathnodes_t), GLOBAL);
            return (FALSE);
            }
      }  /* End of for j */

  for (index = 0; index < count2; index++)
     Mem_Dealloc (node_strings[index].nodes, node_strings[index].size, GLOBAL);

  Mem_Dealloc (node_strings, count2 * sizeof (Pathnodes_t), GLOBAL);
#endif

  return (TRUE);
}






/******************************************************************************
*
*  Function Name :           SVan_Der_Waal
*
*    This routine returns the van_der_waal string for an atomid.
*
*  Used to be :
*    
*    van_der_waal
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    Allocate memory
*
******************************************************************************/

static String_t SVan_Der_Waal 
  (
  U16_t              atomid
  )
{
  String_t           vdw_name;

  if (atomid > 54)
    vdw_name = String_Create ("300", 3);
  else
    vdw_name = String_Create (vdw[atomid], strlen (vdw[atomid]));

  return (vdw_name);  
}

/*******************************************************************************
*
*  Function Name :           SVisit
*
*    This routine counts the number of instances of the indicated fgroup
*    that are reachable from the indicated node without traversing any
*    bonds between atoms indicated by the matchmap.
*
*  Used to be :
*    
*    visit
*
*  Implicit Inputs :
*
*    N/A
*
*  Implicit Outputs :
*
*    N/A
*  
*  Return Values :
*
*    N/A
*
*  Side Effects :
*
*    N/A
*
******************************************************************************/

static void  SVisit 
  (
  U16_t                            node,
  U8_t                            *count,
  Array_t                         *marked_p, 
  Array_t                         *inst_p,
  Xtr_t                           *target_p
  )
{
  U16_t                            neighbor;
  U8_t                             i;

  Array_1d1_Put (marked_p, node, TRUE);
  if (Array_1d1_Get (inst_p, node) == TRUE) 
    (*count)++;
  
  for (i = 0; i < Xtr_Attr_NumNeighbors_Get (target_p, node); i++)
    {
    neighbor = Xtr_Attr_NeighborId_Get (target_p, node, i);
    if (Array_1d1_Get (marked_p, neighbor) == FALSE)
      SVisit (neighbor, count, marked_p, inst_p, target_p);
    }
  
  return;
}

/****************************************************************************
*
*  Function Name:                 Condition_Dump
*
*    This routine prints a formatted dump of a Condition_t to the
*    specified file.
*
*  Used to be:
*
*    N/A:
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
void Condition_Dump
  (
  Condition_t  *cond_p,                    /* Condition_t to format */
  FileDsc_t    *filed_p                    /* File to dump to */
  )
{
  FILE         *f;                         /* Temporary */
  U8_t          i;                         /* Counter */

  f = IO_FileHandle_Get (filed_p);
  if (cond_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Condition\n");
    return;
    }

/* Fix annoying error where it doesn't matter! */
  switch (Cond_Type_Get (cond_p))
    {
    case PT_TYPE_ELECWD :
    case PT_TYPE_BULKY :
      DEBUG_ADDR (R_POSTTEST, DB_REACTREAD, cond_p);
      if (Cond_Count_Get (cond_p) + Cond_Count2_Get (cond_p)> MX_NODES)
        IO_Exit_Error (R_REACTION, X_SYNERR,
          "Too many items stuffed into a Condition_t");
      break;
    case PT_TYPE_AT_CONEQ :
    case PT_TYPE_AT_STREQ :
    case PT_TYPE_DISC_CONEQ :
    case PT_TYPE_DISC_STREQ :
      DEBUG_ADDR (R_POSTTEST, DB_REACTREAD, cond_p);
      if (Cond_Count_Get (cond_p)> MX_NODES)
        IO_Exit_Error (R_REACTION, X_SYNERR,
          "Too many items stuffed into a Condition_t");
    default :
      break;
    }

  switch (Cond_Type_Get (cond_p))
    {
    case PT_TYPE_ELECWD :
      fprintf (f, "Electron-withdrawing condition\n\t");
      for (i = 0;  i < Cond_Count_Get (cond_p); i++)
        fprintf (f, "%hu ", Cond_ElecWith_Prime_Get (cond_p, i));

      if (Cond_Base_Exists (cond_p) == TRUE)
        fprintf (f, "(atom list)\n\t\tcompare by %s with\n\t%d (value)\n",
          SOp[Cond_Op_Get (cond_p)], (S8_t) Cond_Base_Get (cond_p));
      else
        {
        fprintf (f, "(atom list)\n\t\tcompare by %s with\n\t",
          SOp[Cond_Op_Get (cond_p)]);

        for (i = 0; i < Cond_Count2_Get (cond_p); i++)
          fprintf (f, "%hu ", Cond_ElecWith_Second_Get (cond_p, i));

        fprintf (f, "(atom list)\n");
        }
      break;

    case PT_TYPE_NUMMOLEC :
      fprintf (f, "Number of reacting molecules condition\n");
      fprintf (f, "\tCheck for %hu reacting molecules\n",
        Cond_NumMolecules_Get (cond_p));
      break;

    case PT_TYPE_BULKY :
      fprintf (f, "Substituent bulk condition\n\t");
      if (Cond_Count_Get (cond_p) == 1)
        fprintf (f, "%hu\n", Cond_Bulk_Prime_Get (cond_p, 0));
      else
        fprintf (f, "%hu and %hu\n", Cond_Bulk_Prime_Get (cond_p, 0),
          Cond_Bulk_Prime_Get (cond_p, 1));

      fprintf (f, "\t\tcompare by %s with\n\t", SOp[Cond_Op_Get (cond_p)]);
      if (Cond_Base_Exists (cond_p) == TRUE)
        fprintf (f, "%s\n", SOutput[Cond_Base_Get (cond_p)]);
      else
        if (Cond_Count2_Get (cond_p) == 1)
          fprintf (f, "%hu\n", Cond_Bulk_Second_Get (cond_p, 0));
        else
          fprintf (f, "%hu and %hu\n", Cond_Bulk_Second_Get (cond_p, 0),
            Cond_Bulk_Second_Get (cond_p, 1));
      break;

    case PT_TYPE_DIST :
      fprintf (f, "Distance to substituent condition\n");
      fprintf (f, "\tCheck for FG# %u at distance %hu from atom %hu\n",
        Cond_Dist_FuncGrp_Get (cond_p), Cond_Dist_Value_Get (cond_p, 0),
        Cond_Dist_Base_Get (cond_p));

      if (Cond_Dist_Sling_Get (cond_p) == TRUE)
        fprintf (f, "This condition provides its own Sling\n");
      break;

    case PT_TYPE_PATHLEN :
      fprintf (f, "Path length condition\n\t%hu and %hu\n\t\t",
        Cond_Path_Prime_Get (cond_p, 0), Cond_Path_Prime_Get (cond_p, 1));
      if (Cond_Path_Connected_Get (cond_p) == TRUE)
        fprintf (f, "are connected across non-matched part of molecule\n");
      else
        if (Cond_Base_Exists (cond_p) == TRUE)
          fprintf (f, "compare by %s with\n\t%hu (# of bonds)\n",
            SOp[Cond_Op_Get (cond_p)], Cond_Base_Get (cond_p));
        else
          fprintf (f, "compare by %s with\n\t%hu %hu (atom list)\n",
            SOp[Cond_Op_Get (cond_p)], Cond_Path_Second_Get (cond_p, 0),
            Cond_Path_Second_Get (cond_p, 1));
      break;

    case PT_TYPE_ALKYNE :
      fprintf (f, "Alkyne (ring) condition\n");
      fprintf (f, "\tCheck if atoms %hu and %hu are in a ring\n",
        Cond_Alkyne_Prime_Get (cond_p), Cond_Alkyne_Second_Get (cond_p));
      break;

    case PT_TYPE_ALLENE :
      fprintf (f, "Allene (ring) condition\n");
      fprintf (f, "\tCheck if atoms %hu and %hu are in a ring\n",
        Cond_Allene_Prime_Get (cond_p), Cond_Allene_Second_Get (cond_p));
      break;

    case PT_TYPE_CARBONIUM :
      fprintf (f, "Carbonium ion stability condition\n");
      fprintf (f, "\t%hu and %hu\n\t\tCompared by %s with\n\t%hu and %hu\n",
        Cond_Stability_Prime_Get (cond_p, 0), Cond_Stability_Prime_Get (cond_p,
        1), SOp[Cond_Op_Get (cond_p)], Cond_Stability_Second_Get (cond_p, 0),
        Cond_Stability_Second_Get (cond_p, 1));
      break;

    case PT_TYPE_LVNGROUP :
      fprintf (f, "Leaving group ability condition\n");
      fprintf (f, "\tatom %hu at distance %hu in a %s environment\n"
        "\t\tcompared by %s with\n\t",
        Cond_LeavingGrp_Prime_Get (cond_p), Cond_LeavingGrp_PrimeDist_Get (
        cond_p), SOutput[Cond_LeavingGrp_PrimePh_Get (cond_p)],
        SOp[Cond_Op_Get (cond_p)]);
      if (Cond_Base_Exists (cond_p) == TRUE)
        fprintf (f, "%d (value)\n", Cond_Base_Get (cond_p));
      else
        fprintf (f, "atom %hu at distance %hu in a %s environment\n",
          Cond_LeavingGrp_Second_Get (cond_p), Cond_LeavingGrp_SecondDist_Get (
          cond_p), SOutput[Cond_LeavingGrp_SecondPh_Get (cond_p)]);
      break;

    case PT_TYPE_MIGRATAP :
      fprintf (f, "Migratory ability condition\n");
      fprintf (f, "\tgroup at %hu compared by %s with ",
        Cond_MigratoryApt_Prime_Get (cond_p), SOp[Cond_Op_Get (cond_p)]);
      if (Cond_Base_Exists (cond_p) == TRUE)
        fprintf (f, "%hu (value)\n", Cond_Base_Get (cond_p));
      else
        fprintf (f, "group at %hu\n", Cond_MigratoryApt_Second_Get (cond_p));
      break;

    case PT_TYPE_ATOM :
      fprintf (f, "Atom (matching) condition\n");
      fprintf (f, "\tatom %hu has a %s at distance %hu\n",
        Cond_Atom_Node_Get (cond_p), Atomid2Symbol (Cond_Atom_Id_Get (cond_p)),
        Cond_Atom_Distance_Get (cond_p));
      break;

    case PT_TYPE_FG_XCESS:
      fprintf (f, "Excess functional group condition\n");
      fprintf (f, 
        "\tfragment with node %hu has more than %hu instances of FG #%u\n",
        Cond_Excess_Node_Get (cond_p), Cond_Excess_Count_Get (cond_p),
        Cond_Excess_FGNum_Get (cond_p));
      break;

    case PT_TYPE_AT_CONEQ :
      fprintf (f, "Constitutionally equivilant atoms condition\n\tCheck if");
      for (i = 0; i < Cond_Count_Get (cond_p); i++)
        fprintf (f, " %hu", Cond_AtomsCE_Get (cond_p, i));

      if (Cond_Goal_Get (cond_p) == TRUE)
        fprintf (f, "\n\tare all const. equiv. in the goal pattern\n");
      else
        fprintf (f, "\n\tare all const. equiv. in the subgoal pattern\n");
      break;

    case PT_TYPE_AT_STREQ :
      fprintf (f, "Stereochemically equivilant atoms condition\n\tCheck if");
      for (i = 0; i < Cond_Count_Get (cond_p); i++)
        fprintf (f, " %hu", Cond_AtomsSE_Get (cond_p, i));

      if (Cond_Goal_Get (cond_p) == TRUE)
        fprintf (f, "\n\tare all stereo. equiv. in the goal pattern\n");
      else
        fprintf (f, "\n\tare all stereo. equiv. in the subgoal pattern\n");
      break;

    case PT_TYPE_FG_CONEQ :
      fprintf (f, "Functional group constitutional equivilance condition\n");
      fprintf (f, "\tCheck if all instances of %hu are const. equiv. in the ",
        Cond_FuncGrpCE_Get (cond_p));
      if (Cond_Goal_Get (cond_p) == TRUE)
        fprintf (f, "goal pattern\n");
      else
        fprintf (f, "subgoal pattern\n");
      break;

    case PT_TYPE_FG_STREQ :
      fprintf (f, "Functional group stereochemical equivilance condition\n");
      fprintf (f, "\tCheck if all instances of %hu are stereo. equiv. in the ",
        Cond_FuncGrpSE_Get (cond_p));
      if (Cond_Goal_Get (cond_p) == TRUE)
        fprintf (f, "goal pattern\n");
      else
         fprintf (f, "subgoal pattern\n");
     break;

    case PT_TYPE_DISC_CONEQ :
      fprintf (f, "Constitutionally equivilant disconnected atoms condition\n");
      fprintf (f, "\tCheck if");
      for (i = 0; i < Cond_Count_Get (cond_p); i++)
        fprintf (f, " %hu", Cond_DiscCE_Get (cond_p, i));

      if (Cond_Goal_Get (cond_p) == TRUE)
        fprintf (f, "\n\tare all const. equiv. in the goal pattern\n");
      else
        fprintf (f, "\n\tare all const. equiv. in the subgoal pattern\n");
      break;

    case PT_TYPE_DISC_STREQ :
      fprintf (f, "Stereochemically equivilant disconnected atoms condition\n");
      fprintf (f, "\tCheck if");
      for (i = 0; i < Cond_Count_Get (cond_p); i++)
        fprintf (f, " %hu", Cond_DiscSE_Get (cond_p, i));

      if (Cond_Goal_Get (cond_p) == TRUE)
        fprintf (f, "\n\tare all stereo. equiv. in the goal pattern\n");
      else
        fprintf (f, "\n\tare all stereo. equiv. in the subgoal pattern\n");
      break;

    case PT_TYPE_FG_CNT :
      fprintf (f, "Functional group comparison condition\n\t");
      fprintf (f, 
        "Check if # instances of FG #%d in goal are %s those in the subgoal\n",
        Cond_FuncGrpCnt_Get (cond_p), SOp[Cond_Op_Get (cond_p)]);
      break;

    case PT_TYPE_AROMSUB :
      fprintf (f, "Aromatic substitution condition\n\t");
      if (Cond_AromatSub_Type_Get (cond_p) == PT_AROM1 ||
          Cond_AromatSub_Type_Get (cond_p) == PT_AROM4)
        fprintf (f, " Type %s Hydrogen %hu for node %hu\n",
          SOutput[Cond_AromatSub_Type_Get (cond_p)],
          Cond_AromatSub_Hydrogen_Get (cond_p), 
          Cond_AromatSub_Node_Get (cond_p));
      else
        fprintf (f, " Type %s Hydrogen %hu for node %hu are %s metric %d\n",
          SOutput[Cond_AromatSub_Type_Get (cond_p)],
          Cond_AromatSub_Hydrogen_Get (cond_p), 
          Cond_AromatSub_Node_Get (cond_p),
          SOp[Cond_Op_Get (cond_p)], Cond_AromatSub_Metric_Get (cond_p));
       break;

    default :
      TRACE (R_POSTTEST, DB_CHEMISTRY, TL_MAJOR, (outbuf,
        "SCediscn Error:  illegal condition type %lu.", 
        Cond_Type_Get (cond_p)));
      break;
    }

  if (Cond_Negate_Get (cond_p) == TRUE)
    fprintf (f, "Condition is negative\n\n");
  else
    fprintf (f, "Condition is positive\n\n");

  return;
}

/****************************************************************************
*
*  Function Name:                 Posttest_Dump
*
*    This routine prints a formatted dump of a Test_t to the specified
*    file.
*
*  Used to be:
*
*    N/A:
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
void Posttest_Dump
  (
  Posttest_t   *test_p,                    /* Test_t to format */
  FileDsc_t    *filed_p                    /* File to dump to */
  )
{
  FILE         *f;                         /* Temporary */
  U8_t          i;                         /* Counter */
  U8_t          t;                         /* Temporary */

  f = IO_FileHandle_Get (filed_p);
  if (test_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Test_t\n");
    return;
    }

  DEBUG_ADDR (R_POSTTEST, DB_REACTREAD, test_p);
  fprintf (f, 
    "Post-transform test :\tease % 3d\tyield % 3d\tconfidence % 3d\n"
    "\tresult: %s,%s \ttest:\n",
    Post_EaseAdj_Get (test_p), Post_YieldAdj_Get (test_p),
    Post_ConfidenceAdj_Get (test_p), SResult[Post_Result_Get (test_p)],
    SResult[Post_Stop_Get (test_p)]);

  for (i = 0; i < Post_Length_Get (test_p); i++)
    {
    t = Post_Op_Get (test_p, i);
    if (t == OP_AND)
      fprintf (f, "& ");
    else
      if (t == OP_OR)
        fprintf (f, "| ");
    else
      if (t == OP_NOT)
        fprintf (f, "~ ");
    else
      if (t >= PT_TEST_ADD)
        fprintf (f, "T%hu ", t - PT_TEST_ADD);
    else
      fprintf (f, "C%d ", t);
    }

  fprintf (f, "\n");

  return;
}
/* End of Posttest_Dump */

static int fuzzy_strcmp (U8_t *str1, U8_t *str2)
{
  char *s1, *s2, c1, c2;
  int pos, diff;

  for (s1 = (char *) str1, s2 = (char *) str2, pos = 0; *s1 == *s2; s1++, s2++, pos++);
  c1 = *s1 == '\0' ? '-' : *s1; /* '-' immediately precedes '.' in ASCII */
  c2 = *s2 == '\0' ? '-' : *s2;
  if (pos == 0) pos = 1;
  diff = (5 * (c1 - c2) / pos + 5 * (c1 - c2)) / 10; /* This is a very crude heuristic! */
}

static Zadehan_t Zadehan_AND (Zadehan_t val1, Zadehan_t val2)
{
  Zadehan_t result;

  result.fuzzy_components.TF = 1;
#ifdef _FDEBUG_
printf("\nZadehan_AND: MIN(%d,%d)=%d\n\n", val1.fuzzy_components.TF ? val1.fuzzy_components.conf : 100 - val1.fuzzy_components.conf,
val2.fuzzy_components.TF ? val2.fuzzy_components.conf : 100 - val2.fuzzy_components.conf,
MIN (val1.fuzzy_components.TF ? val1.fuzzy_components.conf : 100 - val1.fuzzy_components.conf,
val2.fuzzy_components.TF ? val2.fuzzy_components.conf : 100 - val2.fuzzy_components.conf));
#endif
  result.fuzzy_components.conf = MIN (val1.fuzzy_components.TF ? val1.fuzzy_components.conf : 100 - val1.fuzzy_components.conf,
    val2.fuzzy_components.TF ? val2.fuzzy_components.conf : 100 - val2.fuzzy_components.conf);
  if (result.fuzzy_components.conf < 50)
  {
    result.fuzzy_components.TF = !result.fuzzy_components.TF;
    result.fuzzy_components.conf = 100 - result.fuzzy_components.conf;
  }

  return (result);
}

static Zadehan_t Zadehan_OR (Zadehan_t val1, Zadehan_t val2)
{
  Zadehan_t result;

  result.fuzzy_components.TF = 1;
  result.fuzzy_components.conf = MAX (val1.fuzzy_components.TF ? val1.fuzzy_components.conf : 100 - val1.fuzzy_components.conf,
    val2.fuzzy_components.TF ? val2.fuzzy_components.conf : 100 - val2.fuzzy_components.conf);
  if (result.fuzzy_components.conf < 50)
  {
    result.fuzzy_components.TF = !result.fuzzy_components.TF;
    result.fuzzy_components.conf = 100 - result.fuzzy_components.conf;
  }
  return (result);
}

static Zadehan_t Zadehan_EQ (Zadehan_t val1, Zadehan_t val2)
{
  Zadehan_t result;

  result = Zadehan_XOR (val1, val2);
  result.fuzzy_components.TF = !result.fuzzy_components.TF;
  return (result);
}

static Zadehan_t Zadehan_XOR (Zadehan_t val1, Zadehan_t val2)
{
  Zadehan_t result1, result2;

  result1 = Zadehan_OR (val1, val2);
  result2 = Zadehan_AND (val1, val2);
  result2.fuzzy_components.TF = !result2.fuzzy_components.TF;
  return (Zadehan_AND (result1, result2));
}

static char *fuzzy_dump (Zadehan_t fuzzy)
{
  static char fdvar[100];

  sprintf (fdvar, "%x (%c: %d%%)", fuzzy.fuzzy_value, fuzzy.fuzzy_components.TF ? 'T' : 'F', fuzzy.fuzzy_components.conf);

  return (fdvar);
}

static char *fuzzy_dump2 (U8_t fuzzy)
{
  Zadehan_t ftemp;
  static char fdvar[100];

  ftemp.fuzzy_value = fuzzy;
  sprintf (fdvar, "%x (%c: %d%%)", ftemp.fuzzy_value, ftemp.fuzzy_components.TF ? 'T' : 'F', ftemp.fuzzy_components.conf);

  return (fdvar);
}

static Zadehan_t Zadehan_Normalized (Zadehan_t in_fuzzy)
{
  Zadehan_t out_fuzzy;

  out_fuzzy = in_fuzzy;
  if (out_fuzzy.fuzzy_value < 101 /* TRUE w/ conf of 50 */)
  {
    out_fuzzy.fuzzy_components.TF = !out_fuzzy.fuzzy_components.TF;
    out_fuzzy.fuzzy_components.conf = 100 - out_fuzzy.fuzzy_components.conf;
  }
  return (out_fuzzy);
}

/* End of POSTTEST.C */



