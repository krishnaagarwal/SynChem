#define _H_LOGIN_
/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook, Gerald A. Miller
*
*  Module Name:                     LOGIN.H
*
*    This header contains the constants and data structures used
*    in security clearance for users of the SYNCHEM KB editor GUI.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    05-Jan-2000
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


#ifndef _H_RCB_
#include "rcb.h"
#endif

#define USERSDB FCB_SEQDIR_USER ("/usersdb.dat")
#define TEMPDB  FCB_SEQDIR_USER ("/usersdb.tmp")

#define USER_LEN 12
#define PW_LEN 12
#define LOGIN_TRIES 3
#define SEED 0xe7

#define CURRENT_PW 0
#define OLD_PW 1
#define DUP_PW 2
#define NEW_PW 3
#define NUM_PW_TYPES 4

#define NOTIFY 1
#define MODIFY 2
#define DUP_NOTIFY 3
#define DUP_MODIFY 4
#define OLD_NOTIFY 5
#define OLD_MODIFY 6
#define NEW_NOTIFY 7
#define NEW_MODIFY 8

#define USER 1
#define PW 2
#define PW_MISMATCH 3
#define OLD_PW_MISMATCH 4
#define DUP 5
#define SELF 6
#define NO_SUCH_USER 7
#define CANCELLATION 8
#define FAILURE 9

#define ELogin 0
#define EAddUser 1
#define EChangePassword 2
#define EChangeLevel 3
#define EDelUser 4
#define EListUsers 5
#define ELoginOrCancel 6
#define NUM_LOGIN_ENTRIES 7
#define ELoggedIn NUM_LOGIN_ENTRIES

#define CHG_PW_LEV 1
#define LIST_USR_LEV 2
#define ADD_SCH_LEV 3
#define DIS_SCH_LEV 4
#define EDIT_TRANSFORM_LEV 5
#define EDIT_PRETRAN_LEV 5
#define EDIT_POSTRAN_LEV 5
#define DEL_SCH_LEV 5
#define LIST_LEV_LEV 6
#define MAINT_LEV 7
#define ADD_USR_LEV 9
#define DEL_USR_LEV 9
#define CHG_LEV_LEV 9
#define LIST_PW_LEV 9

/* "Entry" prototype macros
void Login (Widget, void (*)(), Widget);
void AddUser (Widget);
void ChangePassword (Widget);
void ChangeLevel (Widget);
void DelUser (Widget);
void ListUsers (Widget);
void LoginOrCancel (Widget, void (*)(), Widget);

void LoginFail (Widget);
void AddUserFail (Widget, int);
void ChangePasswordFail (Widget, int);
void ChangeLevelFail (Widget, int);
void DelUserFail (Widget, int);
void ListUsersFail (Widget);
void AlreadyLoggedIn (Widget);
void LoginOrCancelFail (Widget, int);
*/

#define Login(widg, func, mwidg) \
  login ((widg), (func), ELogin, (mwidg))
#define AddUser(widg) \
  login ((widg), NULL, EAddUser, NULL)
#define ChangePassword(widg) \
  login ((widg), NULL, EChangePassword, NULL)
#define ChangeLevel(widg) \
  login ((widg), NULL, EChangeLevel, NULL)
#define DelUser(widg) \
  login ((widg), NULL, EDelUser, NULL)
#define ListUsers(widg) \
  login ((widg), NULL, EListUsers, NULL)
#define LoginOrCancel(widg, func, mwidg) \
  login ((widg), (func), ELoginOrCancel, (mwidg))

#define LoginFail(widg) \
  access_denied ((widg), ELogin, 0)
#define AddUserFail(widg, reas) \
  access_denied ((widg), EAddUser, (reas))
#define ChangePasswordFail(widg, reas) \
  access_denied ((widg), EChangePassword, (reas))
#define ChangeLevelFail(widg, reas) \
  access_denied ((widg), EChangeLevel, (reas))
#define DelUserFail(widg, reas) \
  access_denied ((widg), EDelUser, (reas))
#define ListUsersFail(widg, reas) \
  access_denied ((widg), EListUsers, (reas))
#define AlreadyLoggedIn(widg) \
  access_denied ((widg), ELoggedIn, 0)
#define LoginOrCancelFail(widg, reas) \
  access_denied ((widg), ELoginOrCancel, (reas))

char *UserID ();
void login (Widget, void (*)(), int, Widget);
void access_denied (Widget, int, int);
void Login_From_Main (char *);

#ifdef _GLOBAL_DEF_
Boolean_t logged_in = FALSE, login_failed = FALSE;
int clearance_level = 0;

static Widget tl, login_popup[NUM_LOGIN_ENTRIES], denied_box, usr_list, usr_win, usr_dlg, mgw;
static Boolean_t retry_login = FALSE, first_denial = TRUE,
  first_login[NUM_LOGIN_ENTRIES] = {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};
static void (*rest_func[NUM_LOGIN_ENTRIES])() = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static char userstr[32] = {0}, pwstr[NUM_PW_TYPES][32] = {{0}, {0}, {0}, {0}};
static int file_pos = -1, login_attempts = 0;

void User_CB (Widget, XtPointer, XtPointer);
void PW_CB (Widget, XtPointer, XtPointer);
void Login_CB (Widget, XtPointer, XtPointer);
void StoreUsPW_CB (Widget, XtPointer, XtPointer);
void ChangePW_CB (Widget, XtPointer, XtPointer);
void ChangeLvl_CB (Widget, XtPointer, XtPointer);
void DelUser_CB (Widget, XtPointer, XtPointer);
void ListUsers_CB (Widget, XtPointer, XtPointer);
void Denied_CB (Widget, XtPointer, XtPointer);
void Cancel_CB (Widget, XtPointer, XtPointer);
void UnmapList_CB (Widget, XtPointer, XtPointer);
Boolean_t store_user_pw (char *, char *, Boolean_t, int);
Boolean_t user_pw_found (char *, char **, int *);
Boolean_t delete_udb_rec (char *);
Boolean_t replace_pw (char *, char *);
Boolean_t replace_lvl (char *, char *, int);
void add_user (char *, char *, int);
void write_user_record (FILE *, char *, char *, int);
void fill_udb_string (char *, int);
int next_char (U8_t *, int, int, Boolean_t);
void length_exceeded (char *, int);
char *get_udb_line (char *, FILE *, int);
void put_udb_line (char *, FILE *, int);
#else
extern Boolean_t logged_in, login_failed;
extern int clearance_level;
#endif
