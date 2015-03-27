#define _H_AVLFORM_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook, Gerald A. Miller
*
*  Module Name:                     AVLFORM.H
*
*    This header contains the constants and data structures used
*    by the available compounds editor GUI for SYNCHEM.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    15-Sep-2000
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

/*
#define AVL_DICT_COMMAND concat (concat (glob_main_dir, "/avldict "), FCB_SEQDIR_AVLC (""))
*/
#define AVL_DICT_COMMAND concat (concat (exe_dir, "/avldict "), FCB_SEQDIR_AVLC (""))
#ifdef _ENCRYPT_
#define AVL_BKUP_COMMAND concat (concat ("cp ", FCB_SEQDIR_AVLC ("/avlcomp_info ")), FCB_SEQDIR_AVLC ("/avlcomp_info.save"))
#else
#define AVL_BKUP_COMMAND concat (concat ("cp ", FCB_SEQDIR_AVLC ("/avlcomp.isam_info ")), FCB_SEQDIR_AVLC ("/avlcomp.save_info"))
#endif

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
void AvLib_CB (Widget, XtPointer, XtPointer);
void AvlCmp_CB (Widget, XtPointer, XtPointer);
void AvlDraw_CB (Widget, XtPointer, XtPointer);
void AvlSearchDraw_CB (Widget, XtPointer, XtPointer);
void AvlStore_CB (Widget, XtPointer, XtPointer);
void AvlSkipInfo_CB (Widget, XtPointer, XtPointer);
void AvEdit_CB (Widget, XtPointer, XtPointer);
void AvOther_CB (Widget, XtPointer, XtPointer);
void AvlSearch_CB (Widget, XtPointer, XtPointer);
void AvlConfirm_CB (Widget, XtPointer, XtPointer);
void AvlMBDismiss_CB (Widget, XtPointer, XtPointer);
void redraw_compound (Widget, XtPointer, XtPointer);

/* other functions */
void AvEdit_Cont ();
Boolean_t Avl_Record_Empty (int);
void Avl_Delete (int, Boolean_t);
void Avl_Write_Record (int);
int Avl_Best_FreeRec (int);
void Avl_Init_Free_VarLen ();
U16_t Avl_Free_VarLen (int);
void Avl_Drawn_Sling (char *);
void backup_avllib ();
void Avl_Record_Write (int);
void Find_Avl (char *);
void AvlEdit_Create_Form (Widget, XtAppContext, char *);
int AvSearch_Pos (char *, int, Boolean_t, Boolean_t);
void delete_avlcmp ();
void av_disable_delete (Boolean_t);
void new_avlcmp ();
void update_compounds (Boolean_t);
void cdraw(Dsp_Molecule_t *, Tsd_t *, Display *, Drawable, GC, int, int, int, int, Boolean_t);
void avfixdsp (Dsp_Molecule_t *);
int days(int);
void no_avlcomps (char *);

#ifdef _GLOBAL_DEF_
static XtAppContext schlC;
static Widget tl, misc_form, liblistw, avllistw, textw, draww[3], srch_popup, mess_box, conf_box, othermb, otherpdm, info_popup,
	info_lbl[2], info_txt[3], srch_draw_pb[2], modpb[3], srch_txt, glob_txt_widget, drawwin[3];
static Pixmap drawp[3];
static int avllist_pos = 1, avllist_bottom, srch_pos, editing_record;
static short glob_width, glob_height, glob_x, glob_y;
static int avlinx[1000000];
static U16_t avlfree[1000000];
static char avlbuf[1000];
static char srch_str[500];
static Boolean_t srch_fwd, add_set = FALSE, exit_set = FALSE, disable_set = FALSE, delete_set = FALSE, db_modified = FALSE,
  srch_struct_exists = FALSE;
static Display *disp;
static GC gc;
static Boolean_t new_avlcmp_flag = FALSE;
typedef struct
{
  int library;
  Sling_t sling;
  char *name;
  char *catalogn;
}
Avl_Record_t;

Avl_Record_t *Avl_Record_Get (int);

#define Avl_Comp_Library_Get(art) \
  (art)->library
#define Avl_Comp_CanSling_Get(art) \
  (art)->sling
#define Avl_Comp_Name_Get(art) \
  (art)->name
#define Avl_Comp_CatNumber_Get(art) \
  (art)->catalogn

int   alib_num = 0, cmp_num = 0, cmp_count[3]={0, 0, 0}, last_cmpnum[3]={0, 0, 0};
char *alibname[]  = {"Basic",
                     "Supplemental",
                     "Local"};
#else
extern int   lib_num, cmp_num, cmp_count[3];
extern char *libname[];
#endif
