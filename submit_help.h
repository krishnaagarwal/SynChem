#ifndef _H_SUBMIT_HELP_
#define _H_SUBMIT_HELP_ 1
/****************************************************************************
*  
* Copyright (C) 1997 Synchem Group at SUNY-Stony Brook, Daren Krebsbach  
*  
*  
*  Module Name:               SUBMIT_HELP.H  
*  
*    This header file defines the information needed for the    
*    help facility of the job submission module.  
*      
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
#define SMH_YES_RESPONSE   1
#define SMH_NO_RESPONSE    2


/*** Function Prototype Declarations ***/

char  *HelpText_Get        (Widget);
void   ContextHelp_CB      (Widget, XtPointer, XtPointer); 
int    AskUser             (Widget, char  *, char *);
void   AskUser_Response_CB (Widget, XtPointer, XtPointer);

#endif

/*** End Of SUBMIT_HELP.H ***/
