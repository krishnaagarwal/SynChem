#ifndef _H_SUBMIT_TEMPLATES_ 
#define _H_SUBMIT_TEMPLATES_  1 
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_TEMPLATES.H  
*  
*    This header file defines the information needed for the    
*    compound templates database.  
*      
*  Creation Date:  
*  
*     25-Aug-1997  
*  
*  Authors: 
*      Daren Krebsbach 
*        based on version by Nader Abdelsadek
*  
*****************************************************************************/  

/*** Literal Values ***/ 

#define CTP_TEMPLATES_FILENM   "targets.template"
#define CTP_TEMPLATES_MAXLEN   255
#define CTP_FIELD_SEP          "%"
#define CTP_RECORD_SEP         "\n"

#define CTP_TEMPLATES_TITLE    "Templates"
#define CTP_TEMPLATES_VISIBLE  10

#define CTP_PB_DISMISS         "dismiss"
#define CTP_PB_SAVE            "save"
#define CTP_PB_SELECT          "select"

typedef struct comp_template_s
  {
  String_t                 name;
  Sling_t                  sling;
  struct comp_template_s  *next;
  }  Template_t;

/*  Macro Prototypes for Template_t
  String_t     Template_Name_Get  (Template_t *);
  void         Template_Name_Put  (Template_t *, String_t);
  Template_t  *Template_Next_Get  (Template_t *);
  void         Template_Next_Put  (Template_t *, Template_t *);
  Sling_t      Template_Sling_Get (Template_t *);
  void         Template_Sling_Put (Template_t *, Sling_t);
*/

#define  Template_Name_Get(t_p)\
   (t_p)->name
#define  Template_Name_Put(t_p, value)\
   (t_p)->name = (value)

#define  Template_Next_Get(t_p)\
   (t_p)->next
#define  Template_Next_Put(t_p, value)\
   (t_p)->next = (value)

#define  Template_Sling_Get(t_p)\
   (t_p)->sling
#define  Template_Sling_Put(t_p, value)\
   (t_p)->sling = (value)

typedef struct compound_templates_cb_s
  {
  Widget      formdg;
  Widget      list;
  Widget      select_pb;
  Widget      dismiss_pb;
  Widget      save_pb;
  Template_t *templates;
  U32_t       num_temps;
  Boolean_t   created;
  Boolean_t   list_loaded;
  Boolean_t   list_modified;
  }  TemplateCB_t;

/*  Macro Prototypes for TemplateEntry_t and TemplateCB_t

Boolean_t   TemplateCB_Created_Get      (TemplateCB_t *);
void        TemplateCB_Created_Put      (TemplateCB_t *, Boolean_t);
Widget      TemplateCB_DismissPB_Get    (TemplateCB_t *);
void        TemplateCB_DismissPB_Put    (TemplateCB_t *, Widget);
Widget      TemplateCB_FormDg_Get       (TemplateCB_t *);
void        TemplateCB_FormDg_Put       (TemplateCB_t *, Widget);
Widget      TemplateCB_List_Get         (TemplateCB_t *);
void        TemplateCB_List_Put         (TemplateCB_t *, Widget);
Boolean_t   TemplateCB_ListLoaded_Get   (TemplateCB_t *);
void        TemplateCB_ListLoaded_Put   (TemplateCB_t *, Boolean_t);
Boolean_t   TemplateCB_ListModified_Get (TemplateCB_t *);
void        TemplateCB_ListModified_Put (TemplateCB_t *, Boolean_t);
U32_t       TemplateCB_NumTemps_Get     (TemplateCB_t *);
void        TemplateCB_NumTemps_Put     (TemplateCB_t *, U32_t);
Widget      TemplateCB_SavePB_Get       (TemplateCB_t *);
void        TemplateCB_SavePB_Put       (TemplateCB_t *, Widget);
Widget      TemplateCB_SelectPB_Get     (TemplateCB_t *);
void        TemplateCB_SelectPB_Put     (TemplateCB_t *, Widget);
Template_t *TemplateCB_Templates_Get    (TemplateCB_t *);
void        TemplateCB_Templates_Put    (TemplateCB_t *, Template_t *);
*/

#define  TemplateCB_Created_Get(cucb)\
   (cucb)->created
#define  TemplateCB_Created_Put(cucb, value)\
   (cucb)->created = (value)
#define  TemplateCB_DismissPB_Get(cucb)\
   (cucb)->dismiss_pb
#define  TemplateCB_DismissPB_Put(cucb, value)\
   (cucb)->dismiss_pb = (value)
#define  TemplateCB_FormDg_Get(cucb)\
   (cucb)->formdg
#define  TemplateCB_FormDg_Put(cucb, value)\
   (cucb)->formdg = (value)
#define  TemplateCB_List_Get(cucb)\
   (cucb)->list
#define  TemplateCB_List_Put(cucb, value)\
   (cucb)->list = (value)
#define  TemplateCB_ListLoaded_Get(cucb)\
   (cucb)->list_loaded
#define  TemplateCB_ListLoaded_Put(cucb, value)\
   (cucb)->list_loaded = (value)
#define  TemplateCB_ListModified_Get(cucb)\
   (cucb)->list_modified
#define  TemplateCB_ListModified_Put(cucb, value)\
   (cucb)->list_modified = (value)
#define  TemplateCB_NumTemps_Get(cucb)\
   (cucb)->num_temps
#define  TemplateCB_NumTemps_Put(cucb, value)\
   (cucb)->num_temps = (value)
#define  TemplateCB_SavePB_Get(cucb)\
   (cucb)->save_pb
#define  TemplateCB_SavePB_Put(cucb, value)\
   (cucb)->save_pb = (value)
#define  TemplateCB_SelectPB_Get(cucb)\
   (cucb)->select_pb
#define  TemplateCB_SelectPB_Put(cucb, value)\
   (cucb)->select_pb = (value)
#define  TemplateCB_Templates_Get(cucb)\
   (cucb)->templates
#define  TemplateCB_Templates_Put(cucb, value)\
   (cucb)->templates = (value)


/* Function Prototypes */

void      Templates_Create      (TemplateCB_t *, Widget);
void      Templates_Destroy     (TemplateCB_t  *);
Boolean_t TemplatesList_Load    (TemplateCB_t *, char *);
void      TemplatesList_Save    (TemplateCB_t *, char *);

/* Callback Prototypes */  
void   Template_Dismiss_CB      (Widget, XtPointer, XtPointer); 
void   Template_Save_CB         (Widget, XtPointer, XtPointer); 
void   Template_Select_CB       (Widget, XtPointer, XtPointer); 

#endif
 
/*** End Of SUBMIT_TEMPLATES.H ***/

