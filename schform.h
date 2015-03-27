#define _H_SCHFORM_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook, Gerald A. Miller
*
*  Module Name:                     SCHFORM.H
*
*    This header contains the declarations needed by the module schform.c and
*    others that call it.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    01-Mar-2000
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


#define MAX_SCH_TEXT 100
#define NUM_BUTTONS_PER_ROW 3
#define NUM_BUTTON_ROWS 4
#define NUM_BUTTONS (NUM_BUTTONS_PER_ROW * NUM_BUTTON_ROWS + 1)
#define SCHNAME_LEN 256
#define MAX_TEMPLATES 50

#define SCHEDIT_SAVE 1
#define SCHEDIT_CLOSE 2
#define SCHEDIT_INDIRECT 4
#define SCHEDIT_CANCEL 8

void SchEdit_Create_Form (Widget, XtAppContext, int, int *, Boolean_t, Boolean_t, Boolean_t);
void HOnC_CB (Widget, XtPointer, XtPointer);
void Pretran_CB (Widget, XtPointer, XtPointer);
void Postran_CB (Widget, XtPointer, XtPointer);
void SchName_CB (Widget, XtPointer, XtPointer);
void SaveSch_CB (Widget, XtPointer, XtPointer);
void ExitSch_CB (Widget, XtPointer, XtPointer);
void DrwPatt_CB (Widget, XtPointer, XtPointer);
void Lib_CB (Widget, XtPointer, XtPointer);
void Chap_CB (Widget, XtPointer, XtPointer);
void Schema_CB (Widget, XtPointer, XtPointer);
void Detail_CB (Widget, XtPointer, XtPointer);
void Other_CB (Widget, XtPointer, XtPointer);
void Search_CB (Widget, XtPointer, XtPointer);
void Confirm_CB (Widget, XtPointer, XtPointer);
void MBDismiss_CB (Widget, XtPointer, XtPointer);
void redraw_schpatt (Widget, XtPointer, XtPointer);
void Pretran_Update (Boolean_t);
void Pretran_Cont ();
void SaveSch_Cont ();
void ExitSch_Cont ();
void DrwPatt_Cont ();
void val_enable (Boolean_t, Boolean_t);
void exit_enable (Boolean_t);

void init_templates ();
void get_text(int,Boolean_t);
void get_tests(int,Boolean_t);
void get_patts(int,Boolean_t);
void get_dsp_stats (Dsp_Molecule_t *, int *, int *, int *, int *, int *);
void nbr_dfs (Dsp_Atom_t *, int, Boolean_t *, int *, int *, int *, int *, int);
Boolean_t Sch_Tsds_Store (Tsd_t *, Tsd_t *, U32_t *, U32_t *, int, char *);
Boolean_t must_compress_tsd (Tsd_t *);
Boolean_t nodes_in_same_piece (Tsd_t *, U32_t, U32_t);
Boolean_t disconnected_atom (Tsd_t *, U32_t);
Boolean_t all_pieces_rooted (Tsd_t *, U32_t *, int);
void visit_nbrs (Tsd_t *, U32_t, Boolean_t *);

#ifdef _GLOBAL_DEF_

#ifndef _SCHCOMP_
Boolean_t template[MAX_TEMPLATES][MX_FUNCGROUPS], template_complete[4][MAX_TEMPLATES];
char *tempname[MAX_TEMPLATES], *tempabbr[MAX_TEMPLATES];
int NUM_TEMPLATES;

#else

extern Boolean_t template[MAX_TEMPLATES][MX_FUNCGROUPS], template_complete[4][MAX_TEMPLATES];
extern char *tempname[MAX_TEMPLATES], *tempabbr[MAX_TEMPLATES];
extern int NUM_TEMPLATES;

#endif

static Widget tl, /*topform[2] = {(Widget) NULL, (Widget) NULL},*/ topform = (Widget) NULL, schpb[NUM_BUTTONS], sch_savelbl[2],
  schpatt_form, textw, draww, testw, close_box, conf_box;
static Pixmap drawp;
static char schbuf[2][10000];
static char *tempfname[MAX_TEMPLATES];
static Display *disp;
static GC gc;
static int curr_schema, prev_schema, num_schemata, sch_syntheme = 1;
static Boolean_t low_h = TRUE, dont_care = FALSE, template_valid[4][MAX_TEMPLATES], must_close_schema = FALSE, tsd_altered = FALSE;
static Boolean_t new_schema_local, last_closed_local, name_incomplete, pretests_incomplete, patterns_incomplete,schema_open = FALSE,
  new_schema_open = FALSE, eyc_incomplete;
static XtAppContext schlC;
static const char *TEST_RULE = "________________________________________\n",
  *TEXT_RULE = "____________________________________________________________________________________________________\n";
static U32_t roots[MX_ROOTS], synthemes[MX_ROOTS];

#else

extern Boolean_t template[MAX_TEMPLATES][MX_FUNCGROUPS], template_complete[4][MAX_TEMPLATES];
extern char *tempname[MAX_TEMPLATES], *tempabbr[MAX_TEMPLATES];
extern int NUM_TEMPLATES;

#endif
