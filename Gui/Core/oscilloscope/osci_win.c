/**  ******************************************************************************  * @file         osci_win.c  * @created by   Werner Wolfrum  * @date         07.03.2019  * @brief        Main window and menu behaviour for oscilloscope functionality  * @template     game_win.c                  Original file name, author, description, copyright for template below.  ******************************************************************************  *//**
  ******************************************************************************
  * @file    game_win.c
  * @author  MCD Application Team
  * @brief   game functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "osci_res.c"

#include "oszi.h"

#include "../../../Modules/osci_recorder/osci_if.h"


/** @addtogroup GAME_MODULE
  * @{
  */

/** @defgroup GAME
  * @brief game routines
  * @{
  */
  
/* External variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos);

/* Private typedef -----------------------------------------------------------*/
K_ModuleItem_Typedef  oscilloscope =
{
  7,
  " oscilloscope",
  open_osci,
  0,
  Startup,
  NULL,
}
;

/* Private defines -----------------------------------------------------------*/
#define ID_FRAMEWIN_INFO        (GUI_ID_USER + 0x01)
#define ID_IMAGE_INFO           (GUI_ID_USER + 0x02)
#define ID_PROCESS_TIMER        (GUI_ID_USER + 0x03)

#define ID_BUTTON_EXIT          (GUI_ID_USER + 0x04)
#define ID_BUTTON_TIME          (GUI_ID_USER + 0x05)
#define ID_BUTTON_CHANNEL1      (GUI_ID_USER + 0x06)
#define ID_BUTTON_CHANNEL2      (GUI_ID_USER + 0x07)
#define ID_BUTTON_TRIGGER       (GUI_ID_USER + 0x08)
#define ID_BUTTON_RUN           (GUI_ID_USER + 0x09)#define ID_BUTTON_RECORD        (GUI_ID_USER + 0x0A)
#define ID_BUTTON_MORE          (GUI_ID_USER + 0x0B)
#if 0 // WW: Preparation for next implementation
#define ID_BUTTON_BACK          (GUI_ID_USER + 0x0C)
#define ID_BUTTON_CURSOR        (GUI_ID_USER + 0x0D)#define ID_BUTTON_FFT           (GUI_ID_USER + 0x0E)
#define ID_BUTTON_SAVE          (GUI_ID_USER + 0x0F)#define ID_BUTTON_SEND          (GUI_ID_USER + 0x10)#define ID_BUTTON_VERSION       (GUI_ID_USER + 0x11)
#define ID_BUTTON_HELP          (GUI_ID_USER + 0x12)#define ID_BUTTON_CONF          (GUI_ID_USER + 0x13)#endif
#define GUI_ID_LEFT             (GUI_ID_USER + 0x20)
#define GUI_ID_RIGHT            (GUI_ID_USER + 0x21)
#define GUI_ID_UP               (GUI_ID_USER + 0x22)
#define GUI_ID_DOWN             (GUI_ID_USER + 0x23)

#define CLIENT_COLOR      GUI_TRANSPARENT
#define GRID_COLOR        GUI_BROWN

#define RECORDER_MODE_REC_DISABLED            0x00
#define RECORDER_MODE_REC_IDLE                0x01
#define RECORDER_MODE_RECORDING               0x02
#define RECORDER_MODE_PLAY_IDLE               0x03
#define RECORDER_MODE_PLAYING                 0x04


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static WM_HWIN   _hFrame;
static WM_HWIN   hButton[8];
static WM_HTIMER hProcessTimer = 0;
static WM_HWIN   hCurrentDialog = 0;

/* Time Dialog handling ------------------------------------------------------*/
static WM_HWIN   _TimeDialog = 0;
WM_HWIN CreateTimeDialog(WM_HWIN hParent);
void TimeDialog_SetSelection(TIME_SETTING_e timeSel);

/* Channel Dialog handling ---------------------------------------------------*/
static WM_HWIN   _ChannelDialog = 0;
WM_HWIN CreateChannelDialog(WM_HWIN hParent);
void ChannelDialog_SetConfig(CHANNEL_CONFIG_t* config);


static WM_HWIN   _TriggerDialog = 0;
WM_HWIN CreateTriggerDialog(WM_HWIN hParent);
void TriggerDialog_SetConfig(TRIGGER_CONFIG_t* config);

static WM_HWIN   _CursorDialog = 0;
WM_HWIN CreateCursorDialog(WM_HWIN hParent);
void CursorDialog_SetConfig(CURSOR_CONFIG_t* config);


static uint8_t  RecorderMode = RECORDER_MODE_REC_DISABLED;
static char     FileName[256];



/* Invalidate Board */
static void _InvalidateOsci(void) {
  WM_InvalidateWindow(WM_GetClientWindow(_hFrame));
}




/**
  * @brief Handle PID 
  * @param  x: X position     
  * @param  y: Y position
  * @param Pressed: touch status
  * @retval None
  */
static void _HandlePID(int x, int y, int Pressed)
{
   static int _IsInHandlePID;

   if (_IsInHandlePID++ == 0)
   {
      if(Pressed)
	  {
         if( hCurrentDialog != 0 )
         {
            GUI_RECT rect;
            WM_GetWindowRectEx(hCurrentDialog, &rect);
            if( (x < rect.x0) || (x > rect.x1) ||
                (y < rect.y0) || (y > rect.y1)	)
            {
               // Touch outside window
               WM_HideWindow(hCurrentDialog);
               hCurrentDialog = 0;
            }
         }
      }
   }
   _IsInHandlePID--;
}

/**
  * @brief _OnTouch 
  * @param pMsg : pointer to data structure
  * @retval None
  */
static void _OnTouch(WM_MESSAGE* pMsg)
{
  const GUI_PID_STATE* pState = (const GUI_PID_STATE*)pMsg->Data.p;
  if (pState) 
  {
    _HandlePID(pState->x, pState->y, pState->Pressed);
  }
}

/**
  * @brief  Paints callback 
  * @param  hWin: pointer to the parent handle
  * @retval None
  */
static void _OnPaint(WM_HWIN hWin)
{
  GUI_COLOR Color;
  GUI_RECT  r;
  int x, y, xPos, yPos;
  int CellSize, rStone, rMove;
  char Cell, IsValidMove;
  int xCircle, yCircle;

  #if AA_USE_HIRES
    GUI_AA_EnableHiRes();
  #endif

  WM_GetClientRectEx(hWin, &r);


  oszi_process();


}




WM_HWIN Id_Active = 0;
int menuPage = 0;

/**
  * @brief  Paints exit button
  * @param  hObj: button handle
  * @retval None
  */
static void _OnPaint_BUTTONS(WM_HWIN hBtn)
{
   GUI_SetBkColor(FRAMEWIN_GetDefaultClientColor());
   GUI_Clear();

   if(menuPage == 0)   {      if( hBtn == hButton[0] || hBtn == hButton[7] ) // [Exit] + [More]
      {
         GUI_SetColor(GUI_LIGHTGRAY);
         GUI_SetBkColor(GUI_LIGHTGRAY);
      }
      else if( hBtn == hButton[5] ) // [RUN]      {         if( Menu.trigger.state==TRIGGER_STATE_STOP )         {            GUI_SetColor(GUI_RED);            GUI_SetBkColor(GUI_RED);         }         else if(Menu.trigger.state==TRIGGER_STATE_RUN)         {            GUI_SetColor(GUI_GREEN);            GUI_SetBkColor(GUI_GREEN);         }         else if(Menu.trigger.state==TRIGGER_STATE_CONT)         {            GUI_SetColor(GUI_MAGENTA);            GUI_SetBkColor(GUI_GREEN);         }         else if(Menu.trigger.state==TRIGGER_STATE_READY)         {            GUI_SetColor(GUI_ORANGE);            GUI_SetBkColor(GUI_GREEN);         }         else         {            GUI_SetColor(GUI_YELLOW);            GUI_SetBkColor(GUI_YELLOW);         }      }      else if( hBtn == hButton[6] ) // [Rec]
      {
         if( RecorderMode == RECORDER_MODE_RECORDING )
         {
            GUI_SetColor(GUI_RED);
            GUI_SetBkColor(GUI_RED);
         }
         else
         {
            GUI_SetColor(GUI_STCOLOR_LIGHTBLUE);
            GUI_SetBkColor(GUI_STCOLOR_LIGHTBLUE);
         }
      }
      else if( hBtn == Id_Active )
      {
         GUI_SetColor(GUI_LIGHTRED);
         GUI_SetBkColor(GUI_LIGHTRED);
      }
      else
      {
         GUI_SetColor(GUI_STCOLOR_LIGHTBLUE);
         GUI_SetBkColor(GUI_STCOLOR_LIGHTBLUE);      }   }   else // Page 2   {
	  if( hBtn == hButton[0] ) // [Back]	  {		 GUI_SetColor(GUI_LIGHTGRAY);		 GUI_SetBkColor(GUI_LIGHTGRAY);	  }	  else if( hBtn == Id_Active )	  {		 GUI_SetColor(GUI_LIGHTRED);		 GUI_SetBkColor(GUI_LIGHTRED);	  }	  else	  {		 GUI_SetColor(GUI_STCOLOR_LIGHTBLUE);		 GUI_SetBkColor(GUI_STCOLOR_LIGHTBLUE);	  }   }
   // Fill in the selected botton color   GUI_FillRect(1, 1, 38, 32);

  GUI_SetColor(GUI_BLACK);
  GUI_SetFont(GUI_FONT_13B_ASCII);
  const char * str;
  if(menuPage == 0)
  {
    if( hBtn == hButton[0] )
      str = "Exit";
    else if( hBtn == hButton[1] )
      str = "Time";
    else if( hBtn == hButton[2] )
      str = "CH1";
    else if( hBtn == hButton[3] )
      str = "CH2";
    else if( hBtn == hButton[4] )
      str = "Trig";
    else if( hBtn == hButton[5] )
    {
      if( Menu.trigger.state==TRIGGER_STATE_STOP )
        str = "HOLD";
      else
   	    str = "RUN";
    }
    else if( hBtn == hButton[6] )
    {
      if( RecorderMode == RECORDER_MODE_RECORDING )
        str = "REC";
      else
   	    str = "Rec";
    }
    else if( hBtn == hButton[7] )
      str = "More";
    else
      str = "";
  }
  else
  {
    if( hBtn == hButton[0] )
      str = "Back";
    else if( hBtn == hButton[1] )
      str = "Curs";
    else if( hBtn == hButton[2] )
      str = "FFT";
    else if( hBtn == hButton[3] )
      str = "Save";
    else if( hBtn == hButton[4] )
      str = "Send";
    else if( hBtn == hButton[5] )
      str = "Vers";
    else if( hBtn == hButton[6] )      str = "Help";    else if( hBtn == hButton[7] )      str = "Conf";    else
      str = "";
  }
  GUI_DispStringAt(str, 7, 10);
}

void ExecTimeSelectDialog(void);
void ExecChannelConfigDialog(void);
void ExecArrowsWindow(void);

/**
  * @brief  List up to 25 file on the root directory with extension .BMP
  * @param  DirName: Directory name
  * @param  Files: Buffer to contain read files
  * @retval The number of the found files
  */
static void _OsciBuildFileName(void)
{
   static const char* DirName = "0:/Osci";
   int32_t maxFileNum = -1;
   uint32_t counter = 0;
   FRESULT res;
   DIR MyDirectory;
   FILINFO MyFileInfo;
   char newFileName[13] = "00000000.wav\0";

   res = f_opendir(&MyDirectory, DirName);

   if(res == FR_OK)
   {
      for (;;)
      {
         res = f_readdir(&MyDirectory, &MyFileInfo);
         if(res != FR_OK || MyFileInfo.fname[0] == 0)
            break;
         if(MyFileInfo.fname[0] == '.')
            continue;

         if(!(MyFileInfo.fattrib & AM_DIR))
         {
        	 // Searching the "." in the name
            do
            {
               counter++;
            }
            while (MyFileInfo.fname[counter] != 0x2E);


            if((MyFileInfo.fname[counter + 1] == 'w') && (MyFileInfo.fname[counter + 2] == 'a') && (MyFileInfo.fname[counter + 3] == 'v'))
            {
            	// Found a wave file (*.wav)
            	int32_t currentFileNum = atol(MyFileInfo.fname);
            	if( maxFileNum < currentFileNum )
            	{
            		maxFileNum = currentFileNum;
            	}
            }
            counter = 0;
         }
      }
      f_closedir(&MyDirectory);
      maxFileNum++;
#ifdef USE_SD_CARD_DISK
      sprintf(FileName, "0:/Osci/%08ld.wav", maxFileNum);
#else
      sprintf(FileName, "0:/Osci/%08ld.wav", maxFileNum);
#endif
   }
}

void osciStartStopRecording(void)
{
   if( RecorderMode == RECORDER_MODE_REC_IDLE) /* Start Recored */
   {
      _OsciBuildFileName();

      //hItem = WM_GetDialogItem(hMainWin, ID_FILENAMECAPTION);
      //TEXT_SetText(hItem, "File : ");
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
      //hItem = WM_GetDialogItem(hMainWin, ID_FILENAME);
      //TEXT_SetText(hItem, FileName);
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);

      OSCI_RECORDER_SelectFile(FileName, FA_CREATE_ALWAYS | FA_WRITE);
      OSCI_RECORDER_StartRec(osciGetSampleFrequency());
      RecorderMode = RECORDER_MODE_RECORDING;

      //hItem = WM_GetDialogItem(pMsg->hWin, ID_EQUAL);
      //IMAGE_SetGIF(hItem, equal, sizeof(equal));
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
   }

   else if( RecorderMode == RECORDER_MODE_RECORDING) /* Cancel */
   {
	  OSCI_RECORDER_StopRec();
      RecorderMode = RECORDER_MODE_REC_IDLE;
//      OSCI_RECORDER_RemoveOsciFile(FileName);
   }
#if 0
   else if( RecorderMode == RECORDER_MODE_PLAY_IDLE) /* Start Play */
   {
      RecorderMode = RECORDER_MODE_PLAYING;
      OSCI_RECORDER_SelectFile(FileName, FA_OPEN_EXISTING | FA_READ);
      OSCI_RECORDER_Play(DEFAULT_AUDIO_IN_FREQ);

      //hItem = WM_GetDialogItem(pMsg->hWin, ID_EQUAL);
      //IMAGE_SetGIF(hItem, equal, sizeof(equal));
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
   }
#endif
   //hItem = WM_GetDialogItem(pMsg->hWin, ID_RECORD_REC_CANCEL_PLAY);
   //WM_InvalidateWindow(hItem);
   //WM_Update(hItem);

   //hItem = WM_GetDialogItem(pMsg->hWin, ID_RECORD_STOP);
   //WM_InvalidateWindow(hItem);
   //WM_Update(hItem);

   //hItem = WM_GetDialogItem(pMsg->hWin, ID_RECORD_PAUSE);
   //WM_InvalidateWindow(hItem);
   //WM_Update(hItem);
}

void handleMenuItemTouch(WM_HWIN hItem)
{   if(menuPage == 0) // Menu-Page 0   {	  if( hItem == hButton[5] ) // HOLD	  {		 GUI.akt_menu=MM_NONE;		 Menu.akt_setting = SETTING_TRIGGER;		 { // "normal" oder "auto"			if(Menu.trigger.state==TRIGGER_STATE_RUN)			{			   Menu.trigger.state=TRIGGER_STATE_STOP; // von "Run" auf "Stop"			}			else if(Menu.trigger.state==TRIGGER_STATE_STOP)			{			   Menu.trigger.state=TRIGGER_STATE_CONT; // von "Stop" auf "Weiter"			}		 }		 return;	  }	  else if( hItem == hButton[6] ) // RECORD	  {		 GUI.akt_menu=MM_NONE;		 Menu.akt_setting = SETTING_CH1;		 osciStartStopRecording();		 return;	  }   }
   // Reset the previous selected Item and hide a previous shown dialog   if( Id_Active != hItem )   {      if( hCurrentDialog != 0 )      {         WM_HideWindow(hCurrentDialog);         hCurrentDialog = 0;      }      Id_Active = hItem;   }   if(menuPage == 0) // Menu-Page 0
   {      if( hItem == hButton[0] ) // EXIT	  {         // Close file, if a recording is running         if( RecorderMode == RECORDER_MODE_RECORDING)         {            OSCI_RECORDER_StopRec();            RecorderMode = RECORDER_MODE_REC_IDLE;         }         // Close the whole OSCI window         GUI_EndDialog(_hFrame, 0);	  }
	  else if( hItem == hButton[1] ) // Time
	  {
		 GUI.akt_menu=MM_TIME;		 if( _TimeDialog != 0 )
		 {		    if( WM_IsVisible(_TimeDialog) )		    {			   WM_HideWindow(_TimeDialog);		    }		    else		    {
		 	   int ts = (int)(Menu.timebase.value);
			   TimeDialog_SetSelection((TIME_SETTING_e)ts+1);
			   hCurrentDialog = _TimeDialog;
			   WM_ShowWindow(_TimeDialog);		    }
		 }
	  }
	  else if( hItem == hButton[2] ) // CH1
	  {
	 	 GUI.akt_menu=MM_CH_VIS;		 Menu.akt_setting = SETTING_CH1;		 if( _ChannelDialog != 0 )		 {		    if( WM_IsVisible(_ChannelDialog) )		    {		 	   WM_HideWindow(_ChannelDialog);		    }		    else		    {			   CHANNEL_CONFIG_t chConf;
			   chConf.channel  = CHANNEL_SEL_CH1;
			   chConf.voltage  = Menu.ch1.faktor+1;
			   chConf.visible  = !Menu.ch1.visible;
			   chConf.position = Menu.ch1.position;
			   ChannelDialog_SetConfig(&chConf);
			   hCurrentDialog = _ChannelDialog;
			   WM_ShowWindow(_ChannelDialog);		    }
		 }
	  }
	  else if( hItem == hButton[3] ) // CH2
	  {
		 GUI.akt_menu=MM_CH_VIS;		 Menu.akt_setting = SETTING_CH2;		 if( _ChannelDialog != 0 )
		 {
		    if( WM_IsVisible(_ChannelDialog) )		    {			   WM_HideWindow(_ChannelDialog);		    }		    else		    {			   CHANNEL_CONFIG_t chConf;			   chConf.channel  = CHANNEL_SEL_CH2;			   chConf.voltage  = Menu.ch2.faktor+1;			   chConf.visible  = !Menu.ch2.visible;			   chConf.position = Menu.ch2.position;			   ChannelDialog_SetConfig(&chConf);			   hCurrentDialog = _ChannelDialog;			   WM_ShowWindow(_ChannelDialog);		    }		 }
	  }
	  else if( hItem == hButton[4] ) // TRIG
	  {
		 GUI.akt_menu=MM_TRG_SOURCE;		 Menu.akt_setting = SETTING_TRIGGER;		 if( _TriggerDialog != 0 )
		 {
		    if( WM_IsVisible(_TriggerDialog) )		    {			   WM_HideWindow(_TriggerDialog);		    }		    else		    {			   TRIGGER_CONFIG_t trigConf;			   trigConf.channel  = Menu.trigger.source; // aktuelle Source (CH1, CH2)			   trigConf.enabled  = true;			   trigConf.position = 0;			   trigConf.edge = Menu.trigger.edge;      // Trigger-Flanke (HI, LO)			   trigConf.mode = Menu.trigger.mode;      // Mode (Normal, Auto, Single)			   trigConf.single = Menu.trigger.state;    // Single-Status			   trigConf.value_ch1 = Menu.trigger.value_ch1; // Trigger-Value (CH1)			   trigConf.value_ch2 = Menu.trigger.value_ch2; // Trigger-Value (CH2)			   TriggerDialog_SetConfig(&trigConf);			   hCurrentDialog = _TriggerDialog;			   WM_ShowWindow(_TriggerDialog);		    }		 }
	  }
	  else if( hItem == hButton[7] ) // More	  {		 menuPage = 1;		 Id_Active = 0;	  }   }   else // Menu-Page 1
   {
	  if( hItem == hButton[0] ) // Back
	  {		 menuPage = 0;		 Id_Active = 0;	  }
	  else if( hItem == hButton[1] ) // CURSOR
	  {
		 GUI.akt_menu=MM_CUR_MODE;		 Menu.akt_setting = SETTING_CURSOR;		 if( _CursorDialog != 0 )
		 {
		    if( WM_IsVisible(_CursorDialog) )		    {		 	   WM_HideWindow(_CursorDialog);		    }		    else		    {		 	   CURSOR_CONFIG_t cursConf;			   cursConf.channel  = CHANNEL_SEL_CH1;			   TriggerDialog_SetConfig(&cursConf);			   hCurrentDialog = _CursorDialog;			   WM_ShowWindow(_CursorDialog);		    }		 }
	  }
	  else if( hItem == hButton[1] ) // FFT	  {		 GUI.akt_menu=MM_FFT_MODE;		 Menu.akt_setting = SETTING_FFT;	  }	  else if( hItem == hButton[3] ) // Save	  {		 GUI.akt_menu=MM_NONE;		 //Menu.akt_setting = SETTING_SEND;	  }	  else if( hItem == hButton[4] ) // Send	  {		 GUI.akt_menu=MM_SEND_MODE;		 Menu.akt_setting = SETTING_SEND;	  }	  else if( hItem == hButton[5] ) // Vers	  {		 GUI.akt_menu=MM_NONE;		 Menu.akt_setting = SETTING_VERSION;	  }	  else if( hItem == hButton[6] ) // Help	  {		 GUI.akt_menu=MM_NONE;		 Menu.akt_setting = SETTING_HELP;	  }	  else if( hItem == hButton[7] ) // Conf	  {		 GUI.akt_menu=MM_NONE;		 //Menu.akt_setting = SETTING_SEND;	  }   }}

int getActiveMenuItem(void)
{
   if(menuPage == 0)
   {
      if( Id_Active == hButton[1] ) // Time
      {
	      return 1;
      }
      else if( Id_Active == hButton[2] ) // CH1
      {
	      return 2;
      }
      else if( Id_Active == hButton[3] ) // CH2
      {
	      return 3;
      }
      else if( Id_Active == hButton[4] ) // Trig
      {
	      return 4;
      }
      else if( Id_Active == hButton[5] ) // Hold
      {
	      return 5;
      }
      else if( Id_Active == hButton[6] ) // Rec
      {
	      return 6;
      }
    }
    else
    {
      if( Id_Active == hButton[1] ) // Cursor
      {
	      return 11;
      }
      else if( Id_Active == hButton[2] ) // FFT
      {
	      return 12;
      }
      else if( Id_Active == hButton[3] ) // Save
      {
	      return 13;
      }
      else if( Id_Active == hButton[4] ) // Send
      {
	      return 14;
      }
      else if( Id_Active == hButton[5] ) // Vers
      {
	      return 15;
      }
      else if( Id_Active == hButton[6] ) // Help      {	      return 15;      }      else if( Id_Active == hButton[7] ) // Conf      {	      return 15;      }   }
   return 0;
}


/**
  * @brief  callback for all menu buttons
  * @param  pMsg: pointer to data structure of type WM_MESSAGE
  * @retval None
  */
static void _cbButton_menu(WM_MESSAGE * pMsg)
{
  const GUI_PID_STATE* pState = 0;

  switch (pMsg->MsgId)
  {
    case WM_PAINT:
      _OnPaint_BUTTONS(pMsg->hWin);
      break;
    case WM_TOUCH:
   	  pState = (const GUI_PID_STATE*)pMsg->Data.p;
   	  if (pState && pState->Pressed)
   	  {
         handleMenuItemTouch(pMsg->hWin);
   	  }
      BUTTON_Callback(pMsg); // WW: Continue the notification chain...
	  break;
    default:
      /* The original callback */
      BUTTON_Callback(pMsg);
      break;
  }
}



/**
  * @brief  callback Reversi Win 
  * @param pMsg: pointer to data structure 
  * @retval None
  */
static void _cbOsciWin(WM_MESSAGE* pMsg)
{
  WM_HWIN hWin = pMsg->hWin;
  int Id, NCode;
  
  switch (pMsg->MsgId)
  {
#if 0
  case WM_CREATE:
    hProcessTimer = WM_CreateTimer(pMsg->hWin, ID_PROCESS_TIMER, 100, 0);
    break;
#endif
  case WM_TIMER:
	_InvalidateOsci();
    WM_RestartTimer(pMsg->Data.v, 50);
    break;

  case WM_PAINT:
    _OnPaint(hWin);
    break;
  case WM_TOUCH:
    _OnTouch(pMsg);
    break;
#if 0 //WW: Not longer needed here
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);    /* Id of widget */
    NCode = pMsg->Data.v;               /* Notification code */
    
    switch(Id)
    {
    case ID_BUTTON_EXIT: 
      switch(NCode)
      {
      case WM_NOTIFICATION_RELEASED:
        if(menuPage == 0)
        {
          //GUI_EndDialog(pMsg->hWin, 0);
        }
        break;
      }
      break;
    }
    break;
#endif    
  case WM_DELETE:
    if(hProcessTimer!=0)
    {
       WM_DeleteTimer(hProcessTimer);
       hProcessTimer = 0;
    }
    _hFrame = 0;

    if(_TimeDialog!=0)
    {
      GUI_EndDialog(_TimeDialog, 0);
	  _TimeDialog = 0;
    }
    if(_ChannelDialog!=0)
    {
      GUI_EndDialog(_ChannelDialog, 0);
	  _ChannelDialog = 0;
    }
    if(_TriggerDialog!=0)
    {
      GUI_EndDialog(_TriggerDialog, 0);
	  _TriggerDialog = 0;
    }
    if(_CursorDialog!=0)
    {
      GUI_EndDialog(_CursorDialog, 0);
	  _CursorDialog = 0;
    }
    break;

  default:
    WM_DefaultProc(pMsg);
  }
}

/**
  * @brief  Game window Startup
  * @param  hWin: pointer to the parent handle.
  * @param  xpos: X position 
  * @param  ypos: Y position
  * @retval None
  */
static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos)
{
  WM_HWIN hItem;
  _hFrame = WINDOW_CreateEx(xpos, ypos, 480, 272, hWin, WM_CF_SHOW, 0, 0x500, &_cbOsciWin);

  hButton[0] = BUTTON_CreateAsChild(440,  0, 40, 34, _hFrame, ID_BUTTON_EXIT, WM_CF_SHOW);
  WM_SetCallback(hButton[0], _cbButton_menu);
  hButton[1] = BUTTON_CreateAsChild(440, 34, 40, 34, _hFrame, ID_BUTTON_TIME, WM_CF_SHOW);
  WM_SetCallback(hButton[1], _cbButton_menu);
  hButton[2] = BUTTON_CreateAsChild(440, 68, 40, 34, _hFrame, ID_BUTTON_CHANNEL1, WM_CF_SHOW);
  WM_SetCallback(hButton[2], _cbButton_menu);
  hButton[3] = BUTTON_CreateAsChild(440,102, 40, 34, _hFrame, ID_BUTTON_CHANNEL2, WM_CF_SHOW);
  WM_SetCallback(hButton[3], _cbButton_menu);
  hButton[4] = BUTTON_CreateAsChild(440,136, 40, 34, _hFrame, ID_BUTTON_TRIGGER, WM_CF_SHOW);
  WM_SetCallback(hButton[4], _cbButton_menu);
  hButton[5] = BUTTON_CreateAsChild(440,170, 40, 34, _hFrame, ID_BUTTON_RUN, WM_CF_SHOW);
  WM_SetCallback(hButton[5], _cbButton_menu);
  hButton[6] = BUTTON_CreateAsChild(440,204, 40, 34, _hFrame, ID_BUTTON_RECORD, WM_CF_SHOW);
  WM_SetCallback(hButton[6], _cbButton_menu);
  hButton[7] = BUTTON_CreateAsChild(440,238, 40, 34, _hFrame, ID_BUTTON_MORE, WM_CF_SHOW);
  WM_SetCallback(hButton[7], _cbButton_menu);

  WINDOW_SetBkColor(_hFrame, GUI_BLACK);

  _TimeDialog = CreateTimeDialog(_hFrame);
  TimeDialog_SetSelection(TIME_SET_5ms);

  _ChannelDialog = CreateChannelDialog(_hFrame);


  _TriggerDialog = CreateTriggerDialog(_hFrame);
  _CursorDialog = CreateCursorDialog(_hFrame);

  //TODO:WW: Check if the Recording drive and folder is available
  OSCI_RECORDER_Init(50);
  RecorderMode = RECORDER_MODE_REC_IDLE;

  oszi_init();
  oszi_start();
  hProcessTimer = WM_CreateTimer(_hFrame, ID_PROCESS_TIMER, 50, 0);
}

/**
  * @brief  Notify State Change
  * @param  pMsg: pointer to data structure of type WM_MESSAGE
  * @retval None
  */
void _cbOsciNotifyStateChange (void)
{
  WM_HWIN hItem;
  if(OSCI_RECORDER_GetState() == OSCI_RECORDER_SUSPENDED)
  {
    if(RecorderMode == RECORDER_MODE_PLAYING)
    {
      RecorderMode = RECORDER_MODE_PLAY_IDLE;
      //hItem = WM_GetDialogItem(hMainWin, ID_RECORD_REC_CANCEL_PLAY);
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
    }

    if(RecorderMode == RECORDER_MODE_RECORDING)
    {
      RecorderMode = RECORDER_MODE_REC_IDLE;
    }

    //hItem = WM_GetDialogItem(hMainWin, ID_EQUAL);
    //IMAGE_SetBitmap(hItem, &bmframe0);

    //WM_InvalidateWindow(hItem);
    //WM_Update(hItem);

    //if(hMainWin != 0)
    //{
    //  hItem = WM_GetDialogItem(hMainWin, ID_ELAPSED_TIME);
    //  TEXT_SetText(hItem, "00:00");
    //  WM_Update(hItem);
    //}
  }
}
#if 0 // Earlier button callback realization ==> To Deletestatic void _cbButton(WM_MESSAGE *pMsg){  int Id;  BUTTON_SKINFLEX_PROPS Props;  GUI_PID_STATE current_PIDState;  switch(pMsg->MsgId)  {  case WM_TOUCH:    GUI_PID_GetCurrentState(&current_PIDState);    if(UnPressed_Flag == true)    {      if(current_PIDState.Pressed == 1)      {        Id = WM_GetId(pMsg->hWin);        switch(Id)        {        case ID_BUTTON_0:          _BringToBottom(_hWindow4);          _BringToTop(_hWindow5);          break;        case ID_BUTTON_3:          _BringToBottom(_hWindow4);          _BringToTop(_hWindow6);          break;        case ID_BUTTON_6:          PROGBAR_SetValue(_hProgBar_1, 0);          _BringToBottom(_hWindow5);          _BringToTop(_hWindow4);          break;        case ID_BUTTON_7:          _ProgBarStat(_hProgBar_1);          break;        case ID_BUTTON_8:          _BringToBottom(_hWindow6);          _BringToTop(_hWindow4);          break;        default:          break;        }      }    }    break;  default:    BUTTON_Callback(pMsg);    break;  }}#endif#if 0 // Earlier button callback realization ==> To Delete/**  * @brief  callback for Exit button  * @param  pMsg: pointer to data structure of type WM_MESSAGE  * @retval None  */static void _cbButton_exit(WM_MESSAGE * pMsg){  switch (pMsg->MsgId)  {    case WM_PAINT:      _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_EXIT);      break;    default:      /* The original callback */      BUTTON_Callback(pMsg);      break;  }}static void _cbButton_trigger(WM_MESSAGE * pMsg){  switch (pMsg->MsgId)  {	case WM_PAINT:	  _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_TRIGGER);	  break;    case WM_TOUCH:      _ShowMessageBox(_hFrame, "Trigger", "WM_TOUCH!", 0);      Menu.akt_setting = SETTING_TRIGGER;      break;	default:	  /* The original callback */	  BUTTON_Callback(pMsg);	  break;  }}static void _cbButton_channels(WM_MESSAGE * pMsg){  switch (pMsg->MsgId)  {	case WM_PAINT:	  _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_CHANNEL1);	  break;    case WM_TOUCH:      setActiveMenuItem(ID_BUTTON_CHANNEL1);      GUI.akt_menu=MM_CH_VIS;      Menu.akt_setting = SETTING_CH1;	  break;	default:	  /* The original callback */	  BUTTON_Callback(pMsg);	  break;  }}static void _cbButton_cursor(WM_MESSAGE * pMsg){  switch (pMsg->MsgId)  {	case WM_PAINT:	  _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_CURSOR);	  break;    case WM_TOUCH:      setActiveMenuItem(ID_BUTTON_CURSOR);      GUI.akt_menu=MM_CUR_MODE;      Menu.akt_setting = SETTING_CURSOR;	  break;	default:	  /* The original callback */	  BUTTON_Callback(pMsg);	  break;  }}static void _cbButton_fft(WM_MESSAGE * pMsg){  switch (pMsg->MsgId)  {	case WM_PAINT:	  _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_FFT);	  break;    case WM_TOUCH:      setActiveMenuItem(ID_BUTTON_FFT);      GUI.akt_menu=MM_FFT_MODE;      Menu.akt_setting = SETTING_FFT;	  break;	default:	  /* The original callback */	  BUTTON_Callback(pMsg);	  break;	  }}static void _cbButton_send(WM_MESSAGE * pMsg){  switch (pMsg->MsgId)  {	case WM_PAINT:	  _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_SEND);	  break;    case WM_TOUCH:      setActiveMenuItem(ID_BUTTON_SEND);      GUI.akt_menu=MM_SEND_MODE;      Menu.akt_setting = SETTING_SEND;	  break;	default:	  /* The original callback */	  BUTTON_Callback(pMsg);	  break;  }}static void _cbButton_version(WM_MESSAGE * pMsg){  switch (pMsg->MsgId)  {	case WM_PAINT:	  _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_VERSION);	  break;    case WM_TOUCH:      setActiveMenuItem(ID_BUTTON_VERSION);      Menu.akt_setting = SETTING_VERSION;	  break;	default:	  /* The original callback */	  BUTTON_Callback(pMsg);	  break;  }}static void _cbButton_help(WM_MESSAGE * pMsg){  //WM_HWIN hWin = pMsg->hWin;  //int Id = WM_GetId(pMsg->hWin);  switch (pMsg->MsgId)  {	case WM_PAINT:	  _OnPaint_BUTTONS(pMsg->hWin, ID_BUTTON_HELP);	  break;    case WM_TOUCH:      setActiveMenuItem(ID_BUTTON_HELP);      Menu.akt_setting = SETTING_HELP;	  break;	default:	  /* The original callback */	  BUTTON_Callback(pMsg);	  break;  }}#endif
#if 0 // MessageBox Example/**  * @brief callback Message Box  * @param  pMsg : pointer to data structure  * @retval None  */static void _cbMessageBox(WM_MESSAGE* pMsg){  WM_HWIN hWin;  int Id;  hWin = pMsg->hWin;  switch (pMsg->MsgId) {  case WM_NOTIFY_PARENT:    if (pMsg->Data.v == WM_NOTIFICATION_RELEASED) {      Id = WM_GetId(pMsg->hWinSrc);       GUI_EndDialog(hWin, (Id == GUI_ID_OK) ? 1 : 0);    }    break;  default:    WM_DefaultProc(pMsg);  }}/**  * @brief  Wait for a dialog to be completely created  * @param  hDialog: dialog handle  * @retval status  */static int GUI_WaitForDialogExec (WM_HWIN hDialog){  WM_DIALOG_STATUS DialogStatus = {0};  GUI_SetDialogStatusPtr(hDialog, &DialogStatus);  while (!DialogStatus.Done)  {    if (!GUI_Exec()) {      GUI_Delay(100);    }  }  return DialogStatus.ReturnValue;}/**  * @brief  Show Message Box  * @param  pTitle: pointer to the title  * @param  pText: pointer to the text  * @param  YesNo player response  * @retval int  */static int _ShowMessageBox(WM_HWIN hWin, const char* pTitle, const char* pText, int YesNo) {  WM_HWIN hFrame, hClient, hBut, hItem;  int r = 0;  /* Create frame win */  hFrame = FRAMEWIN_CreateEx(105, 76, 270, 120, hWin, WM_CF_SHOW, FRAMEWIN_CF_MOVEABLE, 0, pTitle, &_cbMessageBox);  FRAMEWIN_SetClientColor   (hFrame, GUI_WHITE);  FRAMEWIN_SetFont          (hFrame, &GUI_Font16B_ASCII);  FRAMEWIN_SetTextAlign     (hFrame, GUI_TA_HCENTER);  /* Create dialog items */  hClient = WM_GetClientWindow(hFrame);  hItem = TEXT_CreateEx(10, 17, 250, 60, hClient, WM_CF_SHOW, GUI_TA_HCENTER, 0, pText);  TEXT_SetFont(hItem, &GUI_Font13_ASCII);  if (YesNo) {    hBut = BUTTON_CreateEx(137, 60, 55, 24, hClient, WM_CF_SHOW, 0, GUI_ID_CANCEL);    BUTTON_SetText        (hBut, "No");    hBut = BUTTON_CreateEx(72, 60, 55, 24, hClient, WM_CF_SHOW, 0, GUI_ID_OK);    BUTTON_SetText        (hBut, "Yes");  } else {    hBut = BUTTON_CreateEx(104, 60, 55, 24, hClient, WM_CF_SHOW, 0, GUI_ID_OK);    BUTTON_SetText        (hBut, "Ok");  }  WM_SetFocus(_hFrame);  WM_MakeModal(hFrame);  r = GUI_WaitForDialogExec(hFrame);  return r;}#endif#if 0 // Test Dialog Examplestatic const GUI_WIDGET_CREATE_INFO _aTestDialogCreate[] ={   { FRAMEWIN_CreateIndirect, "Dialog", 0,               10,  10, 180, 230, 0,             0, 0 },   { BUTTON_CreateIndirect,   "OK",     GUI_ID_OK,      100,   5,  60,  20, 0,             0, 0 },   { BUTTON_CreateIndirect,   "Cancel", GUI_ID_CANCEL,  100,  30,  60,  20, 0,             0, 0 },   { TEXT_CreateIndirect,     "LText",  0,               10,  55,  48,  15, TEXT_CF_LEFT,  0, 0 },   { TEXT_CreateIndirect,     "RText",  0,               10,  80,  48,  15, TEXT_CF_RIGHT, 0, 0 },   { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT0,    60,  55, 100,  15, 0,            50, 0 },   { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT1,    60,  80, 100,  15, 0,            50, 0 },   { TEXT_CreateIndirect,     "Hex",    0,               10, 100,  48,  15, TEXT_CF_RIGHT, 0, 0 },   { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT2,    60, 100, 100,  15, 0,             6, 0 },   { TEXT_CreateIndirect,     "Bin",    0,               10, 120,  48,  15, TEXT_CF_RIGHT, 0, 0 },   { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT3,    60, 120, 100,  15, 0,             0, 0 },   { LISTBOX_CreateIndirect,  NULL,     GUI_ID_LISTBOX0, 10,  10,  48,  40, 0,             0, 0 },   { CHECKBOX_CreateIndirect, NULL,     GUI_ID_CHECK0,   10, 140,   0,   0, 0,             0, 0 },   { CHECKBOX_CreateIndirect, NULL,     GUI_ID_CHECK1,   30, 140,   0,   0, 0,             0, 0 },   { SLIDER_CreateIndirect,   NULL,     GUI_ID_SLIDER0,  60, 140, 100,  20, 0,             0, 0 },   { SLIDER_CreateIndirect,   NULL,     GUI_ID_SLIDER1,  10, 170, 150,  30, 0,             0, 0 }};const char* ppText1[] ={  "5.0V",  "2.0V",  "1.0V",  "0.5V",  "0.2V",  "0.1V",};/***********************************************************************Dialog procedure*/static void _cbTestDialogCallback(WM_MESSAGE * pMsg){   WM_HWIN hEdit0, hEdit1, hEdit2, hEdit3;   WM_HWIN hListBox;   WM_HWIN hWin;   int NCode;   int Id;   hWin = pMsg->hWin;   switch (pMsg->MsgId)   {   case WM_INIT_DIALOG:      /* Get window handles for all widgets */      hEdit0 = WM_GetDialogItem(hWin, GUI_ID_EDIT0);      hEdit1 = WM_GetDialogItem(hWin, GUI_ID_EDIT1);      hEdit2 = WM_GetDialogItem(hWin, GUI_ID_EDIT2);      hEdit3 = WM_GetDialogItem(hWin, GUI_ID_EDIT3);      hListBox = WM_GetDialogItem(hWin, GUI_ID_LISTBOX0);      /* Initialize all widgets */      EDIT_SetText(hEdit0, "EDIT widget 0");      EDIT_SetText(hEdit1, "EDIT widget 1");      EDIT_SetTextAlign(hEdit1, GUI_TA_LEFT);      EDIT_SetHexMode(hEdit2, 0x1234, 0, 0xffff);      EDIT_SetBinMode(hEdit3, 0x1234, 0, 0xffff);      LISTBOX_SetText(hListBox, ppText1);      WM_DisableWindow(WM_GetDialogItem(hWin, GUI_ID_CHECK1));      CHECKBOX_Check( WM_GetDialogItem(hWin, GUI_ID_CHECK0));      CHECKBOX_Check( WM_GetDialogItem(hWin, GUI_ID_CHECK1));      SLIDER_SetWidth( WM_GetDialogItem(hWin, GUI_ID_SLIDER0), 5);      SLIDER_SetValue( WM_GetDialogItem(hWin, GUI_ID_SLIDER1), 50);      break;   case WM_KEY:#if 0      switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key)      {      case GUI_ID_ESCAPE:         GUI_EndDialog(hWin, 1);         break;      case GUI_ID_ENTER:         GUI_EndDialog(hWin, 0);         break;      }#endif      break;   case WM_NOTIFY_PARENT:      Id = WM_GetId(pMsg->hWinSrc);      /* Id of widget */      NCode = pMsg->Data.v;      /* Notification code */      switch (NCode)      {      case WM_NOTIFICATION_RELEASED:         /* React only if released */         if (Id == GUI_ID_OK)         {            /* OK Button */            GUI_EndDialog(hWin, 0);         }         if (Id == GUI_ID_CANCEL)         {            /* Cancel Button */            GUI_EndDialog(hWin, 1);         }         break;      case WM_NOTIFICATION_SEL_CHANGED:         /* Selection changed */         FRAMEWIN_SetText(hWin, "Dialog - sel changed");         break;      default:         FRAMEWIN_SetText(hWin, "Dialog - notification received");      }      break;   default:      WM_DefaultProc(pMsg);   }}#endif  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
