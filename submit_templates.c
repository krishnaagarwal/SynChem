/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_TEMPLATES.C
*  
*    Creates and updates the compound templates database.  
*      
*  Creation Date:  
*  
*     26-Aug-1997  
*  
*  Authors: 
*      Daren Krebsbach 
*        based on version by Nader Abdelsadek
*  
*  Routines:
*
*    Templates_Create
*    Templates_Destroy
*    TemplatesList_Load
*    TemplatesList_Save
*
*    Template_Dismiss_CB
*    Template_Save_CB
*    Template_Select_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*******************************************************************************/  
#include <string.h>
#include <Xm/PushB.h> 
#include <Xm/RowColumn.h> 
#include <Xm/Text.h> 
#include <Xm/LabelG.h> 
#include <Xm/Form.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/List.h>
#include <Xm/TextF.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"
 
#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_APP_RESRC_ 
#include "app_resrc.h" 
#endif 

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif
 
#ifndef _H_SUBMIT_TEMPLATES_
#include "submit_templates.h" 
#endif

#ifndef _H_SUBMIT_ 
#include "submit.h" 
#endif

/****************************************************************************
*  
*  Function Name:                 Templates_Create
*  
*****************************************************************************/  

void Templates_Create 
  (
  TemplateCB_t     *templates_p,
  Widget            parent
  )
{
  XmString           lbl_str;
  Widget             formdg, sep;

  /* Create the Template Dialog */
  lbl_str = XmStringCreateLocalized (CTP_TEMPLATES_TITLE);
  formdg = XmCreateFormDialog (parent, "TemplateFmDg", NULL, 0);
  XtVaSetValues (formdg,
    XmNdialogStyle,   XmDIALOG_FULL_APPLICATION_MODAL, 
    XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNresizePolicy,  XmRESIZE_ANY,
    XmNresizable,     True,
    XmNautoUnmanage,  False, 
    XmNdialogTitle,   lbl_str, 
    NULL); 
  XmStringFree (lbl_str);
   TemplateCB_FormDg_Put (templates_p, formdg);

  /* Create a scrolled templates list */
  TemplateCB_List_Put (templates_p, 
    XmCreateScrolledList (formdg, "TemplateScLst",  NULL, 0));
  XtVaSetValues (TemplateCB_List_Get (templates_p),
    XmNfontList, SynAppR_FontList_Get (&GSynAppR),
    XmNselectionPolicy, XmSINGLE_SELECT,
    NULL);
  XtAddCallback (TemplateCB_List_Get (templates_p), XmNdefaultActionCallback, 
    Template_Select_CB, (XtPointer) templates_p);

  sep = XtVaCreateManagedWidget("TemplateSep",
    xmSeparatorWidgetClass, formdg,
    XmNorientation,         XmHORIZONTAL,
    XmNseparatorType,       XmSINGLE_LINE,
    XmNshadowThickness,     AppDim_SepLarge_Get (&GAppDim),
    NULL);

  /* Create the Dismiss button for the templates dialog */
  lbl_str = XmStringCreateLocalized (CTP_PB_DISMISS);
  TemplateCB_DismissPB_Put (templates_p, 
    XtVaCreateManagedWidget ("TemplatePB",
    xmPushButtonWidgetClass, formdg, 
    XmNlabelType,            XmSTRING,  
    XmNlabelString,          lbl_str,
    NULL));
  XmStringFree (lbl_str);
  XtAddCallback (TemplateCB_DismissPB_Get (templates_p), XmNactivateCallback, 
    Template_Dismiss_CB, (XtPointer) templates_p);

  /* Create the Save button for the templates dialog */
  lbl_str = XmStringCreateLocalized (CTP_PB_SAVE);
  TemplateCB_SavePB_Put (templates_p, 
    XtVaCreateManagedWidget("TemplatePB", 
    xmPushButtonWidgetClass, formdg,
    XmNlabelType,            XmSTRING,  
    XmNlabelString,          lbl_str,
    NULL));
  XmStringFree (lbl_str);
  XtAddCallback (TemplateCB_SavePB_Get (templates_p), XmNactivateCallback, 
    Template_Save_CB, (XtPointer) templates_p);

  /* Create the Select button for the templates dialog */
  lbl_str = XmStringCreateLocalized (CTP_PB_SELECT);
  TemplateCB_SelectPB_Put (templates_p, 
    XtVaCreateManagedWidget("TemplatePB", 
    xmPushButtonWidgetClass, formdg,
    XmNlabelType,            XmSTRING,  
    XmNlabelString,          lbl_str,
    NULL));
  XmStringFree (lbl_str);
  XtAddCallback (TemplateCB_SelectPB_Get (templates_p), XmNactivateCallback, 
    Template_Select_CB, (XtPointer) templates_p);

  XtVaSetValues (TemplateCB_List_Get (templates_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (sep,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        TemplateCB_List_Get (templates_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (TemplateCB_SelectPB_Get (templates_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        sep,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     10,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    30,
    NULL);

  XtVaSetValues (TemplateCB_DismissPB_Get (templates_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        TemplateCB_SelectPB_Get (templates_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     TemplateCB_SelectPB_Get (templates_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     40,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    60,
    NULL);

  XtVaSetValues (TemplateCB_SavePB_Get (templates_p),
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
    XmNtopWidget,        TemplateCB_SelectPB_Get (templates_p),
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    XmNbottomWidget,     TemplateCB_SelectPB_Get (templates_p),
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_POSITION,
    XmNleftPosition,     70,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_POSITION,
    XmNrightPosition,    90,
    NULL);

  XtManageChild (TemplateCB_List_Get (templates_p));
  TemplateCB_Created_Put (templates_p, TRUE);

  return;
}
/* End of Templates_Create */

/****************************************************************************
*  
*  Function Name:                 Templates_Destroy
*  
*****************************************************************************/  

void Templates_Destroy
  (
  TemplateCB_t *tcb_p 
  )
{
  Template_t  *back_p;
  Template_t  *temp_p;
  U32_t        c_i;

  temp_p = TemplateCB_Templates_Get (tcb_p);
  for (c_i = 0; c_i < TemplateCB_NumTemps_Get (tcb_p) && temp_p != NULL; c_i++)
    {
    String_Destroy (Template_Name_Get (temp_p));
    Sling_Destroy (Template_Sling_Get (temp_p));
    back_p = temp_p;
    temp_p = Template_Next_Get (temp_p);
    free (back_p);
    }

  TemplateCB_NumTemps_Put (tcb_p, 0);
  TemplateCB_Templates_Put (tcb_p, NULL);
  TemplateCB_ListModified_Put (tcb_p, FALSE);
  TemplateCB_ListLoaded_Put (tcb_p, FALSE);

  return;
}
/* End of Templates_Destroy */

/****************************************************************************
*  
*  Function Name:                 TemplatesList_Load
*
*  Assume templates are sorted by name in file.
*  
*****************************************************************************/  

Boolean_t TemplatesList_Load
  (
  TemplateCB_t *tcb_p, 
  char         *dir
  )
{
  FILE           *f_p;
  Template_t     *newtemp_p;
  Template_t     *tail_p;
  XmStringTable   compound_list;
  U32_t           c_i, count;
  int             visible;
  char            filename[256];
  char            buff [CTP_TEMPLATES_MAXLEN + 1];

  sprintf (filename, "%s%c%s", dir, '/', CTP_TEMPLATES_FILENM);
#ifdef _WIN32
  f_p = fopen (gccfix (filename), "r");
#else
  f_p = fopen (filename, "r");
#endif
  if (f_p == NULL)
    {
    char mess[256];

    sprintf (mess, "Unable to open template file:\n%s",  filename);
    InfoWarn_Show (mess);
    return (FALSE);
    }

  if (fgets (buff, CTP_TEMPLATES_MAXLEN, f_p) == NULL)
    {
    char mess[256];

    sprintf (mess, "Unable to read template file:\n%s",  filename);
    InfoWarn_Show (mess);
    fclose (f_p);
    return (FALSE);
    }

  sscanf (buff, "%lu", &count);
  tail_p = NULL;

  for (c_i = 0; c_i < count 
      && (fgets (buff, CTP_TEMPLATES_MAXLEN, f_p) != NULL); c_i++)
    {
    String_t     tmpstr;
    char        *name;
    char        *sling;

    name = strtok (buff, CTP_FIELD_SEP);
    sling = strtok (NULL, CTP_RECORD_SEP);

    newtemp_p = (Template_t *) malloc (sizeof (Template_t));
    if (newtemp_p == NULL)
      {
      fprintf (stderr, "Unable to allocate memory for template.\n");
      exit (-1);
      }

    Template_Name_Put (newtemp_p, String_Create (name, 0));
    tmpstr = String_Create (sling, 0);
    Template_Sling_Put (newtemp_p, String2Sling (tmpstr));
    Template_Next_Put (newtemp_p, NULL);
    String_Destroy (tmpstr);

    if (tail_p == NULL)
      {
      TemplateCB_Templates_Put (tcb_p, newtemp_p);
      tail_p = newtemp_p;
      }
    else
      {
      Template_Next_Put (tail_p, newtemp_p);      
      tail_p = newtemp_p;
      }
    }

  if (c_i != count)
    {
    char mess[256];

    TemplateCB_NumTemps_Put (tcb_p, c_i);
    sprintf (mess, "Only %lu of %lu templates read.",  c_i, count);
    count = c_i;
    InfoMess_Show (mess);
    }
  else
    {
    TemplateCB_NumTemps_Put (tcb_p, count);
    }

  TemplateCB_ListModified_Put (tcb_p, FALSE);
  TemplateCB_ListLoaded_Put (tcb_p, TRUE);
  fclose (f_p);

  compound_list = (XmStringTable) XtMalloc (count * sizeof (XmString));
  for (c_i = 0, newtemp_p = TemplateCB_Templates_Get (tcb_p); 
       c_i < count && newtemp_p != NULL; 
       c_i++, newtemp_p = Template_Next_Get (newtemp_p))
    compound_list[c_i] = XmStringCreateLocalized ((char *) String_Value_Get (
      Template_Name_Get (newtemp_p)));

  visible = (int)((count > CTP_TEMPLATES_VISIBLE) ? CTP_TEMPLATES_VISIBLE 
    : count);
  XtVaSetValues (TemplateCB_List_Get (tcb_p),
    XmNitems,            compound_list,
    XmNitemCount,        count,
    XmNvisibleItemCount, visible,
    NULL);

  /*  Destroy the table??  */
  for (c_i = 0; c_i < count; c_i++)
    XmStringFree(compound_list[c_i]);
  XtFree ((char *) compound_list);

  return (TRUE);

}
/* End of TemplatesList_Load */

/****************************************************************************
*  
*  Function Name:                 TemplatesList_Save
*
*  Assume templates are sorted by name in the linked list.
*  
*****************************************************************************/  

void TemplatesList_Save
  (
  TemplateCB_t *tcb_p, 
  char         *dir
  )
{
  FILE        *f_p;
  Template_t  *temp_p;
  U32_t        c_i;
  char         filename[256];

  sprintf (filename, "%s%c%s", dir, '/', CTP_TEMPLATES_FILENM);
#ifdef _WIN32
  f_p = fopen (gccfix (filename), "w");
#else
  f_p = fopen (filename, "w");
#endif
  if (f_p == NULL)
    {
    char mess[256];

    sprintf (mess, "Unable to open template file:\n%s",  filename);
    InfoWarn_Show (mess);
    return;
    }


  fprintf (f_p, "%lu\n", TemplateCB_NumTemps_Get (tcb_p));
  temp_p = TemplateCB_Templates_Get (tcb_p);
  for (c_i = 0; c_i < TemplateCB_NumTemps_Get (tcb_p) && temp_p != NULL; 
       c_i++, temp_p = Template_Next_Get (temp_p))
    {
    fprintf (f_p, "%s%s%s\n", 
      (char *) String_Value_Get (Template_Name_Get (temp_p)),
      CTP_FIELD_SEP, 
      (char *) Sling_Name_Get (Template_Sling_Get (temp_p)));
    }

  fclose (f_p);
  return;

}
/* End of TemplatesList_Save */


/****************************************************************************
*  
*  Function Name:                 Template_Dismiss_CB
*  
*****************************************************************************/  

void Template_Dismiss_CB 
  (  
  Widget      w, 
  XtPointer   client_data, 
  XtPointer   call_data 
  ) 
{ 
  TemplateCB_t *tcb_p; 

  tcb_p = (TemplateCB_t *) client_data;

  XtUnmanageChild (TemplateCB_FormDg_Get (tcb_p));
 
  return;
}  
/* End of Template_Dismiss_CB */

/****************************************************************************
*  
*  Function Name:                 Template_Save_CB
*  
*****************************************************************************/  

void Template_Save_CB 
  (  
  Widget      w, 
  XtPointer   client_data, 
  XtPointer   call_data 
  ) 
{ 
  TemplateCB_t *tcb_p; 

  tcb_p = (TemplateCB_t *) client_data;

  XtUnmanageChild (TemplateCB_FormDg_Get (tcb_p));

  return;
}  
/* End of Template_Save_CB */

/****************************************************************************
*  
*  Function Name:                 Template_Select_CB
*  
*****************************************************************************/  

void Template_Select_CB 
  (  
  Widget      w, 
  XtPointer   client_data, 
  XtPointer   call_data 
  ) 
{ 
  TemplateCB_t *tcb_p; 

  tcb_p = (TemplateCB_t *) client_data;

  XtUnmanageChild (TemplateCB_FormDg_Get (tcb_p));
 
  return;
}
/* End of Template_Select_CB */

/*  End of SUBMIT_TEMPLATES.C  */

