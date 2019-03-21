/**
  ******************************************************************************
  * @file         TimeDLG.c
  * @created by   Werner Wolfrum
  * @date         10.03.2019
  * @brief        A timebase selection dialog for use in STM32F746G-DISCO-OSCI
  ******************************************************************************
  */

#include "DIALOG.h"
#include "oszi.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0  (GUI_ID_USER + 0x00)
#define ID_TEXT_0  (GUI_ID_USER + 0x01)
#define ID_TEXT_1  (GUI_ID_USER + 0x02)
#define ID_TEXT_2  (GUI_ID_USER + 0x03)
#define ID_TEXT_3  (GUI_ID_USER + 0x04)
#define ID_TEXT_4  (GUI_ID_USER + 0x05)
#define ID_TEXT_5  (GUI_ID_USER + 0x06)
#define ID_TEXT_6  (GUI_ID_USER + 0x07)
#define ID_TEXT_7  (GUI_ID_USER + 0x08)
#define ID_TEXT_8  (GUI_ID_USER + 0x09)
#define ID_TEXT_9  (GUI_ID_USER + 0x0A)
#define ID_TEXT_10  (GUI_ID_USER + 0x0B)
#define ID_TEXT_11  (GUI_ID_USER + 0x0C)
#define ID_TEXT_12  (GUI_ID_USER + 0x0D)
#define ID_TEXT_13  (GUI_ID_USER + 0x0E)
#define ID_TEXT_14  (GUI_ID_USER + 0x0F)
#define ID_TEXT_15  (GUI_ID_USER + 0x10)
#define ID_TEXT_16  (GUI_ID_USER + 0x11)


// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static WM_HWIN hOwnHandle = 0;

static TIME_SETTING_e currentTimeSetting = TIME_SET_NONE;
// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _TimeDialogCreate[] = {
  { WINDOW_CreateIndirect, "Time", ID_WINDOW_0, 170, 15, 260, 221, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Text5s", ID_TEXT_0, 5, 5, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text2s", ID_TEXT_1, 5, 41, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text1s", ID_TEXT_2, 5, 76, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text05s", ID_TEXT_3, 5, 111, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text02s", ID_TEXT_4, 5, 146, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text01s", ID_TEXT_5, 5, 181, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text50ms", ID_TEXT_6, 90, 5, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text20ms", ID_TEXT_7, 90, 41, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text10ms", ID_TEXT_8, 90, 76, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text5ms", ID_TEXT_9, 90, 111, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text2ms", ID_TEXT_10, 90, 146, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text1ms", ID_TEXT_11, 90, 181, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text500us", ID_TEXT_12, 175, 5, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text200us", ID_TEXT_13, 175, 41, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text100us", ID_TEXT_14, 175, 76, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text50us", ID_TEXT_15, 175, 111, 80, 35, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text25us", ID_TEXT_16, 175, 146, 80, 35, 0, 0x64, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbTimeDialog(WM_MESSAGE * pMsg)
{
  WM_HWIN hItem;
  int     NCode;
  int     Id;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Text5s'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "5s");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text2s'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "2s");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text1s'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "1s");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text05s'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "0.5s");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text02s'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_4);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "0.2s");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
   //
    // Initialization of 'Text01s'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "0.1s");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text50ms'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_6);
    TEXT_SetText(hItem, "50ms");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text20ms'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_7);
    TEXT_SetText(hItem, "20ms");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text10ms'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_8);
    TEXT_SetText(hItem, "10ms");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text5ms'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_9);
    TEXT_SetText(hItem, "5ms");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text2ms'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_10);
    TEXT_SetText(hItem, "2ms");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text1ms'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_11);
    TEXT_SetText(hItem, "1ms");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text500us'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_12);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "500us");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text200us'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_13);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "200us");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text100us'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_14);
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetText(hItem, "100us");
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text50us'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_15);
    TEXT_SetText(hItem, "50us");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    //
    // Initialization of 'Text25us'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_16);
    TEXT_SetText(hItem, "25us");
    TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_BLACK);
    break;

  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc) - ID_TEXT_0 + TIME_SET_5s;
    NCode = pMsg->Data.v;
    switch(NCode)
    {
    case WM_NOTIFICATION_CLICKED:
      if( currentTimeSetting != TIME_SET_NONE )
      {
        hItem = WM_GetDialogItem(pMsg->hWin, currentTimeSetting+ID_TEXT_0-1);
        TEXT_SetBkColor(hItem, GUI_WHITE);
        TEXT_SetTextColor(hItem, GUI_BLACK);
      }
      hItem = WM_GetDialogItem(pMsg->hWin, Id+ID_TEXT_0-1);
      TEXT_SetBkColor(hItem, GUI_BLUE);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      currentTimeSetting = Id;
      break;
    case WM_NOTIFICATION_RELEASED:
      //TODO:WW: Implement callback or user message for time selection handling
      setTimeSelection(currentTimeSetting);
      WM_HideWindow(pMsg->hWin);
      break;
    }
    break;

  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreateTime
*/
WM_HWIN CreateTimeDialog(WM_HWIN hParent)
{

   hOwnHandle = GUI_CreateDialogBox(_TimeDialogCreate, GUI_COUNTOF(_TimeDialogCreate), _cbTimeDialog, hParent, 0, 0);
   WM_HideWindow(hOwnHandle);

  return hOwnHandle;
}

void TimeDialog_SetSelection(TIME_SETTING_e timeSel)
{
   WM_HWIN hItem;
   int     Id = timeSel;

   if( hOwnHandle != 0 )
   {
      if( currentTimeSetting != TIME_SET_NONE )
      {
         hItem = WM_GetDialogItem(hOwnHandle, currentTimeSetting+ID_TEXT_0-1);
         TEXT_SetBkColor(hItem, GUI_WHITE);
         TEXT_SetTextColor(hItem, GUI_BLACK);
      }
      hItem = WM_GetDialogItem(hOwnHandle, Id+ID_TEXT_0-1);
      TEXT_SetBkColor(hItem, GUI_BLUE);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      currentTimeSetting = Id;
   }
}

/*************************** End of file ****************************/
