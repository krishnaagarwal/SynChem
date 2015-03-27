#define _H_RXLFORM_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook, Gerald A. Miller
*
*  Module Name:                     RXLFORM.H
*
*    This header contains the declarations needed by the module
*    rxlform.c.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    17-Feb-2000
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


#define MAX_TEXT 48
#define ET_UNKNOWN 0
#define ET_SCHNAME 1
#define ET_COMMENT 2
#define ET_REFERENCE 3
#define ET_INVALID 4
#define ET_EYC 5
#define ET_FLAGS 6
#define ET_TYPE 7
#define ET_MAXDIF 8

/* callbacks */
void Lib_CB (Widget, XtPointer, XtPointer);
void Chap_CB (Widget, XtPointer, XtPointer);
void Schema_CB (Widget, XtPointer, XtPointer);
void Detail_CB (Widget, XtPointer, XtPointer);
void Other_CB (Widget, XtPointer, XtPointer);
void Search_CB (Widget, XtPointer, XtPointer);
void Confirm_CB (Widget, XtPointer, XtPointer);
void MBDismiss_CB (Widget, XtPointer, XtPointer);
void redraw_pattern (Widget, XtPointer, XtPointer);

/* other functions */
void RxlEdit_Create_Form (Widget, XtAppContext, int);
int Search_Pos (char *, int, Boolean_t);
void change_password ();
void change_clearance ();
void disable_schema ();
void delete_schema ();
void disable_delete (Boolean_t);
void new_schema ();
void maintenance ();
void add_new_user ();
void delete_user ();
void list_users ();
void update_schemata (Boolean_t);
void mdraw(Dsp_Molecule_t *, Tsd_t *, Display *, Drawable, GC, int, int, int, int, U32_t *, U32_t *, Boolean_t*, Boolean_t);
U32_t root_and_syntheme(U32_t, U32_t *, U32_t *);
void fixdsp (Dsp_Molecule_t *);
int days(int);
void date_calc(long, char *, int *, int *, int *);
void time_calc(long, char *, int *, int *, int *);
void wrap_line (char **, char *, int);
void skipwrap (int *);
void no_schemata (char *);
void run_isthere (Boolean_t);
void backup_rxnlib ();
void refresh_schema (Boolean_t, Boolean_t, Boolean_t);
Boolean_t check_dots (int, char *, Boolean_t, Boolean_t);

#ifdef _GLOBAL_DEF_
static XtAppContext schlC;
static Widget tl, libchap_form, liblistw, chaplistw, schlistw, textw, draww[2], srch_popup, mess_box, conf_box, othermb, otherpdm;
static Pixmap drawp[2];
static int schlist_pos = 1, schlist_bottom, srch_pos, NSch, save_NSch;
static short glob_width, glob_height, glob_x, glob_y;
static int schinx[1000000];
static char schbuf[5000];
static char srch_str[500];
static Boolean_t srch_fwd, add_set = FALSE, exit_set = FALSE, disable_set = FALSE, delete_set = FALSE;
static Display *disp;
static GC gc;
static Boolean_t new_schema_flag = FALSE, last_schema_closed = TRUE;

int   lib_num = 0, chap_num = 0, sch_num = 0, sch_count[3][41]={{0},{0},{0}}, last_schnum[3][40]={{0},{0},{0}};
char *libname[]  = {"Basic",
                    "Developmental",
                    "New (Non-Legacy)"},
     *chapname[] = {"Alcohol",
                    "Carbonyl",
                    "Carbon-Carbon Double Bond",
                    "Ether",
                    "Carboxylic Acid",
                    "Carbon-Carbon Triple Bond",
                    "Carboxylic Ester",
                    "Halogen",
                    "Peroxide",
                    "Carbon-Nitrogen Single Bond",
                    "Carbon-Nitrogen Double Bond",
                    "Carbon-Nitrogen Triple Bond",
                    "Nitrogen-Nitrogen Double or Triple Bond",
                    "Aromatic Nitrogen; Pyridine",
                    "Carboxylic Amide",
                    "Organosulfur",
                    "Nitrogen-Oxygen Bond (Single or Double)",
                    "Pyrylium; Thiapyrylium",
                    "Aryne",
                    "Organomagnesium",
                    "Organolithium",
                    "Organophosphorus",
                    "Pyrazole; Imidazole; Triazole; Tetrazole; N-N Single Bond",
                    "Oxazole; Isoxazole; Thiazole; Isothiazole",
                    "Furan; Pyrrole; Thiophene",
                    "Organosilicon",
                    "Special Hydrocarbons",
                    "Size 3 Carbocycle",
                    "Size 4 Carbocycle",
                    "Size 5 Carbocycle",
                    "Size 6 Carbocycle",
                    "Size 7 Carbocycle",
                    "Size 8 Carbocycle",
                    "Specific Methylenes (Methyl, 1-Ethyl or Activated)",
                    "O-Esters of Nitrogen, Sulfur or Phosphorus Acids",
                    "",
                    "Organotransition-Metal",
                    "",
                    "Carbene & Nitrene",
                    "General Aromatic Ring"};
#else
extern int   lib_num, chap_num, sch_num, sch_count[3][41];
extern char *libname[], *chapname[];
#endif
