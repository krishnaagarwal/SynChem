#ifndef _H_POSTTEST_
#define _H_POSTTEST_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     POSTTEST.H
*
*    This module is the abstraction for the post transform tests.  These
*    are the tests that determine whether a valid molecule was produced
*    and in some cases where it is, the value of the reaction may be
*    lowered due to chemical difficulties.
*
*    Routines are found in POSTTEST.C unless otherwise noted.
*
*  Creation Date:
*
*    01-Jan-1993
*
*  Authors:
*
*    Lichan Hong      (rewritten based on others PLI code)
*    Daren Krebsbach  (extensive debugging--yet more to do)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

/*** Literal Values ***/

#define OP_GT   1
#define OP_GE   2
#define OP_LT   3
#define OP_LE   4
#define OP_EQ   5
#define OP_NE   6
#define MX_OP   7

#define OP_AND     255
#define OP_OR      254
#define OP_NOT     253
#define OP_NOPASS  252
#define BOOLOP_EQ  251
#define BOOLOP_XOR 250

#define PT_TYPE_ELECWD     1
#define PT_TYPE_NUMMOLEC   2
#define PT_TYPE_BULKY      3
#define PT_TYPE_DIST       4
#define PT_TYPE_PATHLEN    5
#define PT_TYPE_ALKYNE     6
#define PT_TYPE_ALLENE     7
#define PT_TYPE_CARBONIUM  8
#define PT_TYPE_LVNGROUP   9
#define PT_TYPE_MIGRATAP   10
#define PT_TYPE_ATOM       11
#define PT_TYPE_FG_XCESS   12
#define PT_TYPE_AT_CONEQ   13
#define PT_TYPE_AT_STREQ   14
#define PT_TYPE_FG_CONEQ   15
#define PT_TYPE_FG_STREQ   16
#define PT_TYPE_DISC_CONEQ 17
#define PT_TYPE_DISC_STREQ 18
#define PT_TYPE_FG_CNT     19
#define PT_TYPE_AROMSUB    20
#define PT_TYPE_BRIDGEHEAD 21

#define COND_INVALID       (U8_t)-101
#define MX_NODES           8
#define PT_TEST_ADD        128

#define PT_ROOT            0
#define PT_DIRECT          1

#define PT_ACIDIC          1
#define PT_NEUTRAL         7
#define PT_BASIC           14

#define PT_METHYL          17
#define PT_ETHYL           18
#define PT_ISOPROPYL       19
#define PT_T_BUTYL         20

#define PT_AROM1           25
#define PT_AROM2           26
#define PT_AROM3           27
#define PT_AROM4           28
#define PT_AROM5           29

#define PT_NOMIG           33
#define PT_ALKYL           34
#define PT_ARYL            35
#define PT_PROTON          36
#define PT_MAXIDENT        37

#define MAX_NUM_OF_CONDITIONS  129
#define UNDEFINED              2
 
#define MAX_POST_LENGTH        200
 
#define MAX_NUM_OF_POSTTESTS   121
 
#define NUM_OF_SINGLE_ATTRIBUTES    140
 
#define MAX_NUM_OF_INSTANCES        100
 
#define TABLE_LENGTH             236
#define MAX_LENGTH		  62
 
#define INVALID_NUM	(U16_t)-1

/*** Data Structures ***/

/* Reaction condition structure, one for each type of chemical knowledge to
   be tested for.  The interpretation goes as follows:
   - bitvec field: contains information on 
     - Count1 : # atoms in first part of specific condition
     - Count2 : # atoms in second part of specific condition
     - Goal   : condition applies to goal pattern (matched)
     - Negate : condition result is negated
     - Op     : standard binary relational operator to apply, Boolean result
     - Type   : which condition is this  (tag for interpretation of rest of
                data-structure)

   Unless specified, all node indices are relative to the goal pattern.

   PT_TYPE_ELECWD Electron withdrawing effect of Count1 substituents 
   rooted at nodes Cond_ElecWith_Prime Op Cond_Base if a number or
   Count2 nodes rooted at Cond_ElecWith_Second.

   PT_TYPE_NUMMOLEC Number of reacting molecules, equal to Cond_NumMolecules.

   PT_TYPE_BULKY Bulkiness (as in steric hindrance) of Count1 substituents
   rooted at nodes Cond_Bulk_Prime Op Cond_Base if a specific group or
   Count2 nodes rooted at Cond_Bulk_Second.

   PT_TYPE_DIST Distance to an attribute, Cond_Dist_FuncGrp, is
   Cond_Dist_Value from Cond_Dist_Base.  ??? Sling value ???

   PT_TYPE_PATHLEN The two nodes in Cond_Path_Prime are either
   (Cond_Path_Connected is TRUE) not connected by the unmatched portion of
   the molecule, (Cond_Base_Exists is TRUE) Op Cond_Base bond lengths, or
   (Cond_Count2 != 0) Op the distance between the two Cond_Path_Second
   nodes.

   PT_TYPE_ALKYNE The nodes Cond_Alkyne_Prime and Cond_Alkyne_Second are
   in a small ring.

   PT_TYPE_ALLENE The nodes Cond_Allene_Prime and Cond_Allene_Second are
   in a small ring.

   PT_TYPE_CARBONIUM The stability of the carbonium ion defined by
   the nodes Cond_Stability_Prime is Op the carbonium ion defined by
   the nodes Cond_Stability_Second.

   PT_TYPE_LVNGROUP Under reaction conditions Cond_LeavingGrp_PrimePh
   there is a leaving group at distance Cond_LeavingGrp_PrimeDist in
   the substituent rooted at Cond_LeavingGrp_Prime in the goal pattern
   whose leaving ability is Op Cond_Base or the leaving ability of the
   group Cond_LeavingGrp_Second at distance Cond_LeavingGrp_SecondDist.

   PT_TYPE_MIGRATAP The group defined by node Cond_MigratoryApt_Prime has
   migratory aptitude Op Cond_Base or the migratory aptitude of the group
   defined by node Cond_MigratoryApt_Second.

   PT_TYPE_ATOM Atom at distance Cond_Atom_Distance from Cond_Atom_Node
   is Cond_Atom_Id.

   PT_TYPE_FG_XCESS The functional group Cond_Excess_FGNum has strictly
   more instances than Cond_Excess_Count in the piece of the molecule
   rooted at node Cond_Excess_Node.

   PT_TYPE_AT_CONEQ The Cond_Count nodes rooted at Cond_AtomsCE in
   Cond_Goal (TRUE => Goal, FALSE => Subgoal) are constitutionally equivalent.

   PT_TYPE_AT_STREQ The Cond_Count nodes rooted at Cond_AtomsSE in
   Cond_Goal (TRUE => Goal, FALSE => Subgoal) are stereochemically equivalent.

   PT_TYPE_FG_CONEQ All instances of Cond_FuncGrpCE in the Cond_Goal
   (TRUE => Goal, FALSE => Subgoal) are constitutionally equivalent.

   PT_TYPE_FG_STREQ All instances of Cond_FuncGrpSE in the Cond_Goal
   (TRUE => Goal, FALSE => Subgoal) are stereochemically equivalent.

   PT_TYPE_DISC_CONEQ The Cond_Count nodes rooted at Cond_DiscCE are
   constitutionally equivalent when disconnected from the goal pattern.

   PT_TYPE_DISC_CONEQ The Cond_Count nodes rooted at Cond_DiscSE are
   stereochemically equivalent when disconnected from the goal pattern.

   PT_TYPE_FG_CNT The number of instances of Cond_FuncGrp_Cnt in the
   goal is Op the number in the subgoal.

   PT_TYPE_AROMSUB Not sure yet (waiting for Jerry as of 14-Oct-93)

   PT_TYPE_BRIDGEHEAD As Aromsub
*/

typedef struct s_reactcond
  {
  U32_t         bitvec;                    /* Masked bit-vector */
  union u_rcond
    {
    struct s_cond_1                        /* One 16-bit fld, x 8-bit flds */
      {
      U16_t     bit16;
      U8_t      bit8[MX_NODES - 2];
      } t1;
    struct s_cond_2                        /* Array of 8-bit flds */
      {
      U8_t      bit8[MX_NODES];
      } t2;
    } u1;                                  /* Union, one entry for each area */
  } Condition_t;
#define CONDITIONSIZE sizeof (Condition_t)

#define CondM_Count       0x0000003f
#define CondM_Count2      0x003f0000
#define CondM_Goal        0x01000000
#define CondM_Negate      0x02000000
#define CondM_Op          0x00000700
#define CondM_Type        0x0000f800

#define CondV_Count       0
#define CondV_Count2      16
#define CondV_Op          8
#define CondV_Type        11

/** Field Access Macros for Condition_t **/

/* Macro Prototypes
   Boolean_t Cond_Base_Exists             (Condition_t *);
   U8_t      Cond_Base_Get                (Condition_t *);
   void      Cond_Base_Put                (Condition_t *, U8_t);

   U8_t      Cond_Count_Get               (Condition_t *);
   void      Cond_Count_Put               (Condition_t *, U8_t);
   U8_t      Cond_Count2_Get              (Condition_t *);
   void      Cond_Count2_Put              (Condition_t *, U8_t);
   Boolean_t Cond_Goal_Get                (Condition_t *);
   void      Cond_Goal_Put                (Condition_t *, Boolean);
   Boolean_t Cond_Negate_Get              (Condition_t *);
   void      Cond_Negate_Put              (Condition_t *, Boolean);
   U8_t      Cond_Op_Get                  (Condition_t *);
   void      Cond_Op_Put                  (Condition_t *, U8_t);
   U8_t      Cond_Type_Get                (Condition_t *);
   void      Cond_Type_Put                (Condition_t *, U8_t);

   U8_t      Cond_Alkyne_Prime_Get        (Condition_t *);
   void      Cond_Alkyne_Prime_Put        (Condition_t *, U8_t);
   U8_t      Cond_Alkyne_Second_Get       (Condition_t *);
   void      Cond_Alkyne_Second_Put       (Condition_t *, U8_t);

   U8_t      Cond_Allene_Prime_Get        (Condition_t *);
   void      Cond_Allene_Prime_Put        (Condition_t *, U8_t);
   U8_t      Cond_Allene_Second_Get       (Condition_t *);
   void      Cond_Allene_Second_Put       (Condition_t *, U8_t);

   Boolean_t Cond_AromatSub_Hydrogen_Get  (Condition_t *);
   void      Cond_AromatSub_Hydrogen_Put  (Condition_t *, Boolean);
   S16_t     Cond_AromatSub_Metric_Get    (Condition_t *);
   void      Cond_AromatSub_Metric_Get    (Condition_t *, S16_t);
   U8_t      Cond_AromatSub_Node_Get      (Condition_t *);
   void      Cond_AromatSub_Node_Put      (Condition_t *, U8_t);
   U8_t      Cond_AromatSub_Type_Get      (Condition_t *);
   void      Cond_AromatSub_Type_Put      (Condition_t *, U8_t);

   U8_t      Cond_Atom_Distance_Get       (Condition_t *);
   void      Cond_Atom_Distance_Put       (Condition_t *, U8_t);
   U16_t     Cond_Atom_Id_Get             (Condition_t *);
   void      Cond_Atom_Id_Put             (Condition_t *, U16_t);
   U8_t      Cond_Atom_Node_Get           (Condition_t *);
   void      Cond_Atom_Node_Put           (Condition_t *, U8_t);

   U8_t      Cond_AtomsCE_Get             (Condition_t *, U8_t);
   void      Cond_AtomsCE_Put             (Condition_t *, U8_t, U8_t);

   U8_t      Cond_AtomsSE_Get             (Condition_t *, U8_t);
   void      Cond_AtomsSE_Put             (Condition_t *, U8_t, U8_t);

   U8_t      Cond_Bulk_Prime_Get          (Condition_t *, U8_t);
   void      Cond_Bulk_Prime_Put          (Condition_t *, U8_t, U8_t);
   U8_t      Cond_Bulk_Second_Get         (Condition_t *, U8_t);
   void      Cond_Bulk_Second_Put         (Condition_t *, U8_t, U8_t);
 
   U8_t      Cond_DiscCE_Get              (Condition_t *, U8_t);
   void      Cond_DiscCE_Put              (Condition_t *, U8_t, U8_t);

   U8_t      Cond_DiscSE_Get              (Condition_t *, U8_t);
   void      Cond_DiscSE_Put              (Condition_t *, U8_t, U8_t);

   U8_t      Cond_Dist_Base_Get           (Condition_t *);
   void      Cond_Dist_Base_Put           (Condition_t *, U8_t);
   U16_t     Cond_Dist_FuncGrp_Get        (Condition_t *);
   void      Cond_Dist_FuncGrp_Put        (Condition_t *, U16_t);
   Boolean   Cond_Dist_Sling_Get          (Condition_t *);
   void      Cond_Dist_Sling_Put          (Condition_t *, Boolean);
   U8_t      Cond_Dist_Value_Get          (Condition_t *);
   void      Cond_Dist_Value_Put          (Condition_t *, U8_t);

   U8_t      Cond_ElecWith_Prime_Get      (Condition_t *, U8_t);
   void      Cond_ElecWith_Prime_Put      (Condition_t *, U8_t, U8_t);
   U8_t      Cond_ElecWith_Second_Get     (Condition_t *, U8_t);
   void      Cond_ElecWith_Second_Put     (Condition_t *, U8_t, U8_t);

   U8_t      Cond_Excess_Count_Get        (Condition_t *);
   void      Cond_Excess_Count_Put        (Condition_t *, U8_t);
   U8_t      Cond_Excess_FGNum_Get        (Condition_t *);
   void      Cond_Excess_FGNum_Put        (Condition_t *, U8_t);
   U16_t     Cond_Excess_Node_Get         (Condition_t *);
   void      Cond_Excess_Node_Put         (Condition_t *, U16_t);

   U16_t     Cond_FuncGrpCnt_Get          (Condition_t *);
   void      Cond_FuncGrpCnt_Put          (Condition_t *, U16_t);

   U16_t     Cond_FuncGrpCE_Get           (Condition_t *);
   void      Cond_FuncGrpCE_Put           (Condition_t *, U16_t);

   U16_t     Cond_FuncGrpSE_Get           (Condition_t *);
   void      Cond_FuncGrpSE_Put           (Condition_t *, U16_t);

   U8_t      Cond_LeavingGrp_Prime_Get      (Condition_t *);
   void      Cond_LeavingGrp_Prime_Put      (Condition_t *, U8_t);
   U8_t      Cond_LeavingGrp_PrimePh_Get    (Condition_t *);
   void      Cond_LeavingGrp_PrimePh_Put    (Condition_t *, U8_t);
   U8_t      Cond_LeavingGrp_PrimeDist_Get  (Condition_t *);
   void      Cond_LeavingGrp_PrimeDist_Put  (Condition_t *, U8_t);
   U8_t      Cond_LeavingGrp_Second_Get     (Condition_t *);
   void      Cond_LeavingGrp_Second_Put     (Condition_t *, U8_t);
   U8_t      Cond_LeavingGrp_SecondPh_Get   (Condition_t *);
   void      Cond_LeavingGrp_SecondPh_Put   (Condition_t *, U8_t);
   U8_t      Cond_LeavingGrp_SecondDist_Get (Condition_t *);
   void      Cond_LeavingGrp_SecondDist_Put (Condition_t *, U8_t);

   U8_t      Cond_MigratoryApt_Prime_Get  (Condition_t *);
   void      Cond_MigratoryApt_Prime_Put  (Condition_t *, U8_t);
   U8_t      Cond_MigratoryApt_Second_Get (Condition_t *);
   void      Cond_MigratoryApt_Second_Put (Condition_t *, U8_t);

   U8_t      Cond_NumMolecules_Get        (Condition_t *);
   void      Cond_NumMolecules_Put        (Condition_t *, U8_t);

   Boolean_t Cond_Path_Connected_Get      (Condition_t *);
   void      Cond_Path_Connected_Put      (Condition_t *, Boolean);
   U8_t      Cond_Path_Prime_Get          (Condition_t *, U8_t);
   void      Cond_Path_Prime_Put          (Condition_t *, U8_t, U8_t);
   U8_t      Cond_Path_Second_Get         (Condition_t *, U8_t);
   void      Cond_Path_Second_Put         (Condition_t *, U8_t, U8_t);

   U8_t      Cond_Stability_Prime_Get     (Condition_t *, U8_t);
   void      Cond_Stability_Prime_Put     (Condition_t *, U8_t, U8_t);
   U8_t      Cond_Stability_Second_Get    (Condition_t *, U8_t);
   void      Cond_Stability_Second_Put    (Condition_t *, U8_t, U8_t);
*/

#define Cond_Base_Exists(cond_p)\
  ((cond_p)->u1.t2.bit8[MX_NODES - 1] == COND_INVALID ? FALSE : TRUE)

#define Cond_Base_Get(cond_p)\
  (cond_p)->u1.t2.bit8[MX_NODES - 1]

#define Cond_Base_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[MX_NODES - 1] = (value)

#define Cond_Count_Get(cond_p)\
  ((U8_t)(((cond_p)->bitvec & CondM_Count) >> CondV_Count))

#define Cond_Count_Put(cond_p, value)\
  (cond_p)->bitvec &= ~CondM_Count;\
  (cond_p)->bitvec |= (CondM_Count & ((value) << CondV_Count));

#define Cond_Count2_Get(cond_p)\
  ((U8_t)(((cond_p)->bitvec & CondM_Count2) >> CondV_Count2))

#define Cond_Count2_Put(cond_p, value)\
  (cond_p)->bitvec &= ~CondM_Count2;\
  (cond_p)->bitvec |= (CondM_Count2 & ((value) << CondV_Count2));

#define Cond_Goal_Get(cond_p)\
  ((cond_p)->bitvec & CondM_Goal ? TRUE : FALSE)

#define Cond_Goal_Put(cond_p, value)\
  if ((value) == FALSE)\
    (cond_p)->bitvec &= ~CondM_Goal;\
  else\
    (cond_p)->bitvec |= CondM_Goal;

#define Cond_Negate_Get(cond_p)\
  ((cond_p)->bitvec & CondM_Negate ? TRUE : FALSE)

#define Cond_Negate_Put(cond_p, value)\
  if ((value) == FALSE)\
    (cond_p)->bitvec &= ~CondM_Negate;\
  else\
    (cond_p)->bitvec |= CondM_Negate;

#define Cond_Op_Get(cond_p)\
  ((U8_t)(((cond_p)->bitvec & CondM_Op) >> CondV_Op))

#define Cond_Op_Put(cond_p, value)\
  (cond_p)->bitvec &= ~CondM_Op;\
  (cond_p)->bitvec |= (CondM_Op & ((value) << CondV_Op));

#define Cond_Type_Get(cond_p)\
  ((U8_t)(((cond_p)->bitvec & CondM_Type) >> CondV_Type))

#define Cond_Type_Put(cond_p, value)\
  (cond_p)->bitvec &= ~CondM_Type;\
  (cond_p)->bitvec |= (CondM_Type & ((value) << CondV_Type));

/* Alkyne rings are defined by two nodes.  This check is applied to the
   goal pattern.
*/

#define Cond_Alkyne_Prime_Get(cond_p)\
  (cond_p)->u1.t2.bit8[0]

#define Cond_Alkyne_Prime_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[0] = (value)

#define Cond_Alkyne_Second_Get(cond_p)\
  (cond_p)->u1.t2.bit8[1]

#define Cond_Alkyne_Second_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[1] = (value)

/* Allene rings are defined by two nodes.  This check is applied to the
   goal pattern.
*/

#define Cond_Allene_Prime_Get(cond_p)\
  (cond_p)->u1.t2.bit8[0]

#define Cond_Allene_Prime_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[0] = (value)

#define Cond_Allene_Second_Get(cond_p)\
  (cond_p)->u1.t2.bit8[1]

#define Cond_Allene_Second_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[1] = (value)

/* Aromatic substitution is parseable, but we are waiting for email from
   Jerry before providing a sensible interpretation.
*/

#define Cond_AromatSub_Hydrogen_Get(cond_p)\
  (cond_p)->u1.t1.bit8[0]

#define Cond_AromatSub_Hydrogen_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[0] = (value)

#define Cond_AromatSub_Metric_Get(cond_p)\
  (S16_t)(cond_p)->u1.t1.bit16

#define Cond_AromatSub_Metric_Put(cond_p, value)\
  (cond_p)->u1.t1.bit16 = (S16_t)(value)

#define Cond_AromatSub_Node_Get(cond_p)\
  (cond_p)->u1.t1.bit8[1]

#define Cond_AromatSub_Node_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[1] = (value)

#define Cond_AromatSub_Type_Get(cond_p)\
  (cond_p)->u1.t1.bit8[2]

#define Cond_AromatSub_Type_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[2] = (value)

/* Atoms are tested to be at a given distance from the root index.  A
   distance of 0 corresponds to that node being of that element.  This
   test is applied to the goal pattern.
*/

#define Cond_Atom_Distance_Get(cond_p)\
  (cond_p)->u1.t1.bit8[1]

#define Cond_Atom_Distance_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[1] = (value)

#define Cond_Atom_Id_Get(cond_p)\
  (cond_p)->u1.t1.bit16

#define Cond_Atom_Id_Put(cond_p, value)\
  (cond_p)->u1.t1.bit16 = (value)

#define Cond_Atom_Node_Get(cond_p)\
  (cond_p)->u1.t1.bit8[0]

#define Cond_Atom_Node_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[0] = (value)

/* Constitutionally equivilant atoms are a list of node indexes.
   This test may be applied to the goal or subgoal pattern.
*/

#define Cond_AtomsCE_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_AtomsCE_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

/* Stereochemically equivilant atoms are a list of node indexes.
   This test may be applied to the goal or subgoal pattern.
*/

#define Cond_AtomsSE_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_AtomsSE_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

/* Substituent bulk checks for steric hindrance by comparing the 'bulkiness'
   of one substituent against a fixed value or against another substituent.
   No comment made on which pattern this may be applied to.
*/

#define Cond_Bulk_Prime_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_Bulk_Prime_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

#define Cond_Bulk_Second_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx + Cond_Count_Get (cond_p)]

#define Cond_Bulk_Second_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx + Cond_Count_Get (cond_p)] = (value)

/* Disconnected atoms that are constitutionally equivilant are a list of
   node indexes.  This test may be applied to the goal or subgoal pattern.
*/

#define Cond_DiscCE_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_DiscCE_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

/* Disconnected atoms that are stereochemically equivilant are a list of
   node indexes.   This test may be applied to the goal or subgoal pattern.
*/

#define Cond_DiscSE_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_DiscSE_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

/* Attributes (not quite the same as functional groups, for the moment) are
   an index at a distance from a base.  This test is applied to the goal
   pattern.
*/

#define Cond_Dist_Base_Get(cond_p)\
  (cond_p)->u1.t1.bit8[0]

#define Cond_Dist_Base_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[0] = (value)

#define Cond_Dist_FuncGrp_Get(cond_p)\
  (cond_p)->u1.t1.bit16

#define Cond_Dist_FuncGrp_Put(cond_p, value)\
  (cond_p)->u1.t1.bit16 = (value)

#define Cond_Dist_Sling_Get Cond_Goal_Get

#define Cond_Dist_Sling_Put Cond_Goal_Put

#define Cond_Dist_Value_Get(cond_p, idx)\
  (cond_p)->u1.t1.bit8[(idx) + 1]

#define Cond_Dist_Value_Put(cond_p, idx, value)\
  (cond_p)->u1.t1.bit8[(idx) + 1] = (value)

/* Electronic-withdrawing effects are computed from a list of nodes and
   the sum of their effects.  This may be compared to a fixed number or
   to another list.  Both checks apply to the goal pattern.
*/

#define Cond_ElecWith_Prime_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_ElecWith_Prime_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

#define Cond_ElecWith_Second_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx + Cond_Count_Get (cond_p)]

#define Cond_ElecWith_Second_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx + Cond_Count_Get (cond_p)] = (value)

/* Excess functional group detection checks for more than a given number
   of a specific group in a piece of the goal pattern.
*/

#define Cond_Excess_Count_Get(cond_p)\
  (cond_p)->u1.t1.bit8[0]

#define Cond_Excess_Count_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[0] = (value)

#define Cond_Excess_FGNum_Get(cond_p)\
  (cond_p)->u1.t1.bit16

#define Cond_Excess_FGNum_Put(cond_p, value)\
  (cond_p)->u1.t1.bit16 = (value)

#define Cond_Excess_Node_Get(cond_p)\
  (cond_p)->u1.t1.bit8[1]

#define Cond_Excess_Node_Put(cond_p, value)\
  (cond_p)->u1.t1.bit8[1] = (value)

/* Functional group counting compares the number of instances of a given
   group between the goal and subgoal molecules.
*/

#define Cond_FuncGrpCnt_Get(cond_p)\
  (cond_p)->u1.t1.bit16

#define Cond_FuncGrpCnt_Put(cond_p, value)\
  (cond_p)->u1.t1.bit16 = (value)

/* Functional groups that are constitutionally equivilant is a Func. Grp
   index.  This test may be applied to the goal or subgoal pattern.
*/

#define Cond_FuncGrpCE_Get(cond_p)\
  (cond_p)->u1.t1.bit16

#define Cond_FuncGrpCE_Put(cond_p, value)\
  (cond_p)->u1.t1.bit16 = (value)

/* Functional groups that are stereochemically equivilant is a Func. Grp
   index.  This test may be applied to the goal or subgoal pattern.
*/

#define Cond_FuncGrpSE_Get(cond_p)\
  (cond_p)->u1.t1.bit16

#define Cond_FuncGrpSE_Put(cond_p, value)\
  (cond_p)->u1.t1.bit16 = (value)

/* Leaving group ability checks that the goal pattern has a leaving group
   at a specified distance whose ability under the given conditions is at
   least X, or that compares with a second leaving group at a second
   distance with a second condition.
*/

#define Cond_LeavingGrp_Prime_Get(cond_p)\
  (cond_p)->u1.t2.bit8[0]

#define Cond_LeavingGrp_Prime_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[0] = (value)

#define Cond_LeavingGrp_PrimePh_Get(cond_p)\
  (cond_p)->u1.t2.bit8[1]

#define Cond_LeavingGrp_PrimePh_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[1] = (value)

#define Cond_LeavingGrp_PrimeDist_Get(cond_p)\
  (cond_p)->u1.t2.bit8[2]

#define Cond_LeavingGrp_PrimeDist_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[2] = (value)

#define Cond_LeavingGrp_Second_Get(cond_p)\
  (cond_p)->u1.t2.bit8[3]

#define Cond_LeavingGrp_Second_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[3] = (value)

#define Cond_LeavingGrp_SecondPh_Get(cond_p)\
  (cond_p)->u1.t2.bit8[4]

#define Cond_LeavingGrp_SecondPh_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[4] = (value)

#define Cond_LeavingGrp_SecondDist_Get(cond_p)\
  (cond_p)->u1.t2.bit8[5]

#define Cond_LeavingGrp_SecondDist_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[5] = (value)

/* Migratory aptitude compares the ability to migrate of the group of
   atoms at one node with a fixed number or with another group at
   another node.  This test is applied to the goal pattern.
*/

#define Cond_MigratoryApt_Prime_Get(cond_p)\
  (cond_p)->u1.t2.bit8[0]

#define Cond_MigratoryApt_Prime_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[0] = (value)

#define Cond_MigratoryApt_Second_Get(cond_p)\
  (cond_p)->u1.t2.bit8[1]

#define Cond_MigratoryApt_Second_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[1] = (value)

/* Number of molecules tests how many reacting molecules there are.
   This test is sort of independent of a specific transform pattern.
*/

#define Cond_NumMolecules_Get(cond_p)\
  (cond_p)->u1.t2.bit8[0]

#define Cond_NumMolecules_Put(cond_p, value)\
  (cond_p)->u1.t2.bit8[0] = (value)

/* The path condition checks to see if two nodes are connected by a path
   of a specified length.  This length may be checked against the non-
   matched part of the molecule, or a fixed length, or it may be compared
   against the path between two other nodes.
*/

#define Cond_Path_Connected_Get Cond_Goal_Get

#define Cond_Path_Connected_Put Cond_Goal_Put

#define Cond_Path_Prime_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_Path_Prime_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

#define Cond_Path_Second_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx + 2]

#define Cond_Path_Second_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx + 2] = (value)

/* The stability condition checks the stability of two carbonium ions
   from the goal pattern against each other.  It takes two nodes to
   specify a carbonium ion.
*/

#define Cond_Stability_Prime_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx]

#define Cond_Stability_Prime_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx] = (value)

#define Cond_Stability_Second_Get(cond_p, idx)\
  (cond_p)->u1.t2.bit8[idx + 2]

#define Cond_Stability_Second_Put(cond_p, idx, value)\
  (cond_p)->u1.t2.bit8[idx + 2] = (value)

/** End of Field Access Macros for Condition_t **/

/* Reaction test structure.  A test is composed of one or more conditions
   in a propositional sentence.  It has a result, fail or pass, usually
   with effects (ease, yield, confidence).

   There is one non-intuitive operator, that is OP_NOPASS, this means that
   no tests have passed so far, ie NOT T01 AND NOT T02 ... AND NOT T(n-1)
   also T01~T02~&T03~&...T(n-1)~&
*/

typedef struct s_posthead
  {
  S16_t         ease_adj;
  S16_t         yield_adj;                 /* Yield adjustment */
  S16_t         confidence_adj;            /* Confidence adjustment */
  U8_t          flags;                     /* Flags */
  U8_t          length;                    /* # operators and operands */
  } PosttestHead_t;                        /* Header to make IO easier */
#define POSTTESTHEADSIZE sizeof (PosttestHead_t)

typedef struct s_reacttest
  {
  PosttestHead_t h;                        /* Header to make IO easier */
  U8_t         *ops;                       /* Operand/tor list */
  } Posttest_t;
#define POSTTESTSIZE sizeof (Posttest_t)

#define PostM_Result  0x1
#define PostM_Stop    0x2

/** Field Access Macros for Posttest_t **/
/* Macro Prototypes
   S16_t     Post_ConfidenceAdj_Get (Posttest_t *);
   void      Post_ConfidenceAdj_Put (Posttest_t *, S16_t);
   S16_t     Post_EaseAdj_Get       (Posttest_t *);
   void      Post_EaseAdj_Put       (Posttest_t *, S16_t);
   PosttestHead_t *Post_Head_Get  (Posttest_t *);
   U8_t      Post_Length_Get        (Posttest_t *);
   void      Post_Length_Put        (Posttest_t *, U8_t);
   U8_t      Post_Op_Get            (Posttest_t *, U8_t);
   void      Post_Op_Put            (Posttest_t *, U8_t, U8_t); 
   U8_t     *Post_OpHandle_Get      (Posttest_t *);
   Boolean_t Post_Result_Get        (Posttest_t *);
   void      Post_Result_Put        (Posttest_t *, Boolean_t);
   Boolean_t Post_Stop_Get          (Posttest_t *);
   void      Post_Stop_Put          (Posttest_t *, Boolean_t);
   S16_t     Post_YieldAdj_Get      (Posttest_t *);
   void      Post_YieldAdj_Put      (Posttest_t *, S16_t);
*/

#define Post_ConfidenceAdj_Get(post_p)\
  (post_p)->h.confidence_adj

#define Post_ConfidenceAdj_Put(post_p, value)\
  (post_p)->h.confidence_adj = (value)

#define Post_EaseAdj_Get(post_p)\
  (post_p)->h.ease_adj

#define Post_EaseAdj_Put(post_p, value)\
  (post_p)->h.ease_adj = (value)

#define Post_Head_Get(post_p)\
  (&(post_p)->h) /* not a very useful macro w/o parens */

#define Post_Length_Get(post_p)\
  (post_p)->h.length

#define Post_Length_Put(post_p, value)\
  (post_p)->h.length = (value)

#define Post_Op_Get(post_p, idx)\
  (post_p)->ops[idx]

#define Post_Op_Put(post_p, idx, value)\
  (post_p)->ops[idx] = (value)

#define Post_OpHandle_Get(post_p)\
  (post_p)->ops

#define Post_Result_Get(post_p)\
  (((post_p)->h.flags & PostM_Result) ? TRUE : FALSE)

#define Post_Result_Put(post_p, value)\
  if ((value) == FALSE)\
    (post_p)->h.flags &= ~PostM_Result;\
  else\
    (post_p)->h.flags |= PostM_Result

#define Post_Stop_Get(post_p)\
  (((post_p)->h.flags & PostM_Stop) ? TRUE : FALSE)

#define Post_Stop_Put(post_p, value)\
  if ((value) == FALSE)\
    (post_p)->h.flags &= ~PostM_Stop;\
  else\
    (post_p)->h.flags |= PostM_Stop

#define Post_YieldAdj_Get(post_p)\
  (post_p)->h.yield_adj

#define Post_YieldAdj_Put(post_p, value)\
  (post_p)->h.yield_adj = (value)

/** End of Field Access Macros for Posttest_t **/


/* Distance structure, one for each atom in the goal_xtr */

typedef struct s_distance
{
   U8_t         depth_of_search;
   Array_t  	distance;		/* WORDSIZE array */
} SDistance_t;
#define SDISTANCESIZE sizeof (SDistance_t)

/*  macro prototypes */
/*
  U8_t	SDist_Depth_Get		(SDistance_t *);
  void  SDist_Depth_Put		(SDistance_t *, U8_t);
  Array_t *SDist_Distance_Get	(SDistance_t *);
  void  SDist_Distance_Put	(SDistance_t *, Array_t);
*/

#define SDist_Depth_Get(sdist_p)\
  (sdist_p)->depth_of_search
  
#define SDist_Depth_Put(sdist_p, value)\
  (sdist_p)->depth_of_search = (value)

#define SDist_Distance_Get(sdist_p)\
  &((sdist_p)->distance)

#define SDist_Distance_Put(sdist_p, value)\
  (sdist_p)->distance = (value)


/* Candidate node structure, used in SDist_Or_SAtmdist */

typedef struct s_candidate
{
   U16_t               node;
   struct s_candidate *next;
}  SCandidate_t;

#define SCANDIDATESIZE sizeof (SCandidate_t)



/* Attribute structure, used in Initialize_SAtable_Attr_Xtr and SAtable */

#define CURRENT_NUM_OF_ATTRIBUTES 66

typedef struct s_attribute
{ 
  const char   *sling_name;
  U16_t         root;
  Xtr_t		*xtr_p;
} SAttribute_t;

#define NUM_OF_ILLEGAL_STRUCTURES   32

#define NUM_OF_ILLEGAL_PAIRS      69


/* Tree_node structure, used in SFragnam_Or_SBlkfrgn */

#define DEPTH_LIMIT                 5
         
typedef struct s_tree_node
{
  U16_t               node;
  U8_t                bm;
  U8_t                num_of_neighbors;
  struct s_tree_node *brother_p;
  struct s_tree_node *son_p;
} STree_node_t;
#define STREENODESIZE sizeof (STree_node_t)

/* Fragment structure, used in SIndefct */


typedef struct s_fragment
{
  const char      *frag_name;
  float            inductive;
  float            resonant;
} SFragment_t;


/* Pathnodes structure, used in SThree_Unique */

typedef struct s_pathnodes
{
    U16_t			*nodes;
    U16_t			 size;
    struct s_pathnodes          *next;
} Pathnodes_t;                

#define PATHNODESSIZE sizeof (Pathnodes_t)


/* Path_List structure, used in SBrghead */

typedef struct s_path_list
{
  U16_t			num_paths;
  U16_t			begin_node;
  U16_t			end_node;
  Pathnodes_t          *pathnodes_p;
  struct s_path_list   *next;
} Path_List_t;

#define PATHLISTSIZE sizeof (Path_List_t)

/*** Routine Prototypes ***/

void      Condition_Dump (Condition_t *, FileDsc_t *);
void      Destroy_SAtable_Attr_Xtr (void);  
void      Destroy_Substituent_Table (void);
void      Posttest_Dump (Posttest_t *, FileDsc_t *);

Boolean_t Posttest_Check (Xtr_t *, Xtr_t *, Xtr_t *, Array_t *, Array_t *,
  Array_t *, Array_t *, Array_t *, U16_t, U16_t, U32_t, S16_t *, S16_t *, 
  S16_t *, SShotInfo_t *);

/*** Global Variables ***/



/* End of Posttest.H */
#endif
