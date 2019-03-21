/**
  ******************************************************************************
  * @file         menu.c
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : menu.h
  * Datum    : 24.03.2014
  * Version  : 1.6
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
#ifndef __STM32F7_UB_MENU_H
#define __STM32F7_UB_MENU_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f7xx.h"
//#include "stm32_ub_lcd_ili9341.h"
//#include "stm32_ub_touch_stmpe811.h"
//#include "stm32_ub_font.h"
//#include "stm32_ub_graphic2d.h"
#include <stdio.h>
#include <string.h>

#include "GUI.h"
#include "adc.h"





//WW>>>

#define  LCD_MAXX           ((uint16_t)440)      // Pixel in X-Richtung (minus Button with)
#define  LCD_MAXY           ((uint16_t)272)      // Pixel in Y-Richtung
#define  LCD_PIXEL  LCD_MAXX*LCD_MAXY

typedef struct UB_Image_t
{
  const uint16_t *table; // Tabelle mit den Daten
  uint16_t width;        // Breite des Bildes (in Pixel)
  uint16_t height;       // Hoehe des Bildes  (in Pixel)
}UB_Image;

extern UB_Image GUI1;
extern UB_Image GUI2;



#define UB_Graphic2D_DrawLineNormal(x0,y0,x1,y1,col) \
		GUI_SetColor(col);                           \
		GUI_DrawLine(x0, y0, x1, y1)

#define UB_Font_DrawString(x,y,s,font,vg,bg) \
		GUI_SetFont(font);	\
	    GUI_SetBkColor(bg); /* Set background color */ \
        GUI_SetColor(vg); /* Set foreground color */ \
        GUI_DispStringAt( s, x, y)

#define UB_Graphic2D_DrawPixelNormal(x,y,col) \
		GUI_SetColor(col);                    \
		GUI_DrawPixel(x, y)


#define UB_LCD_FillLayer(x)
#define UB_LCD_SetLayer_2(x)
#define UB_LCD_SetTransparency(x)
#define UB_LCD_Copy_Layer2_to_Layer1()
#define UB_LCD_SetLayer_Menu()
#define UB_LCD_SetLayer_ADC()
#define UB_LCD_SetLayer_Back()

#define UB_LCD_Refresh()
#define UB_Graphic2D_Copy1DMA()
#define UB_Graphic2D_Copy2DMA(x)


#define UB_Button_OnClick(BTN_USER) false



//WW<<<













//--------------------------------------------------------------
// Defines der Farben
//--------------------------------------------------------------
#define  MENU_VG_COL        GUI_WHITE     // Menu Schrift-Farbe
#define  MENU_BG_COL        GUI_BLUE      // Menu Hintergrund
#define  MENU_AK_COL        GUI_RED       // Aktiver Hintergrund


//--------------------------------------------------------------
// Defines der Schrift (nicht �ndern !!)
//--------------------------------------------------------------
#define  FONT_H             10 // hoehe der Schriftart
#define  FONT_W             7  // breite der Schriftart




//--------------------------------------------------------------
// Defines zum Umrechnen von Spannungen
//--------------------------------------------------------------
#define  FAKTOR_ADC            3.0/4095


//--------------------------------------------------------------
// Defines zum Umrechnen von Spannungen
//--------------------------------------------------------------
#define  VFAKTOR_5V             5.0/25
#define  VFAKTOR_2V             2.0/25
#define  VFAKTOR_1V             1.0/25
#define  VFAKTOR_0V5            0.5/25
#define  VFAKTOR_0V2            0.2/25
#define  VFAKTOR_0V1            0.1/25


//--------------------------------------------------------------
// Defines zum umrechnen von Zeiten
//--------------------------------------------------------------
#define  TFAKTOR_5              5.0/(4095/12)
#define  TFAKTOR_2              2.0/(4095/12)
#define  TFAKTOR_1              1.0/(4095/12)
#define  TFAKTOR_500          500.0/(4095/12)
#define  TFAKTOR_200          200.0/(4095/12)
#define  TFAKTOR_100          100.0/(4095/12)
#define  TFAKTOR_50            50.0/(4095/12)
#define  TFAKTOR_20            20.0/(4095/12)
#define  TFAKTOR_10            10.0/(4095/12)
#define  TFAKTOR_25            25.0/(4095/12)


//--------------------------------------------------------------
// Defines zum umrechnen von FFT-Werte
//--------------------------------------------------------------
#define  FFAKTOR_5s              5.0/4095/2
#define  FFAKTOR_2s             12.5/4095/2
#define  FFAKTOR_1s             25.0/4095/2
#define  FFAKTOR_500m           50.0/4095/2
#define  FFAKTOR_200m          125.0/4095/2
#define  FFAKTOR_100m          250.0/4095/2
#define  FFAKTOR_50m           500.0/4095/2
#define  FFAKTOR_20m          1250.0/4095/2
#define  FFAKTOR_10m          2500.0/4095/2
#define  FFAKTOR_5m           5000.0/4095/2
#define  FFAKTOR_2m          12500.0/4095/2
#define  FFAKTOR_1m          25000.0/4095/2
#define  FFAKTOR_500u        50000.0/4095/2
#define  FFAKTOR_200u          125.0/4095/2
#define  FFAKTOR_100u          250.0/4095/2
#define  FFAKTOR_50u           500.0/4095/2
#define  FFAKTOR_25u          1000.0/4095/2



//--------------------------------------------------------------
// Anzahl der Nachkommastellen bei Float
//  Faktor ->  100 = 2 Nachkommastellen,  Formatierung -> "%d.%02d"
//  Faktor -> 1000 = 3 Nachkommastellen,  Formatierung -> "%d.%03d"
//  usw
//--------------------------------------------------------------
#define  STRING_FLOAT_FAKTOR     100    // 100 = 2 Nachkommastellen
#define  STRING_FLOAT_FORMAT "%d.%02d"  // Formatierung
#define  STRING_FLOAT_FORMAT2 "-%d.%02d"  // Formatierung


//--------------------------------------------------------------
// position der GUI
//--------------------------------------------------------------
#define  GUI_YPOS       15
#define  GUI_XPOS_OFF   0
#define  GUI_XPOS_LEFT  10
#define  GUI_XPOS_MID   108
#define  GUI_XPOS_RIGHT 204

//--------------------------------------------------------------
// Button der GUI
//--------------------------------------------------------------
typedef enum {
  GUI_BTN_NONE =0,
  GUI_BTN_UP,
  GUI_BTN_DOWN,
  GUI_BTN_LEFT,
  GUI_BTN_RIGHT
}GUI_Button_t;



//--------------------------------------------------------------
// returnwerte der GUI
//--------------------------------------------------------------
typedef enum {
  MENU_NO_CHANGE =0,
  MENU_CHANGE_GUI,
  MENU_CHANGE_NORMAL,
  MENU_CHANGE_FRQ,
  MENU_CHANGE_MODE,
  MENU_CHANGE_VALUE,
  MENU_SEND_DATA
}MENU_Status_t;



//--------------------------------------------------------------
// Main-Men�-Punkte
//--------------------------------------------------------------
typedef enum {
  MM_NONE =0,
  MM_CH1,
  MM_CH2,
  MM_TIME,
  MM_SETTING,
  MM_TRG_SOURCE,
  MM_TRG_EDGE,
  MM_TRG_MODE,
  MM_TRG_VAL,
  MM_TRG_RESET,
  MM_CH_VIS,
  MM_CH_POS,
  MM_CUR_MODE,
  MM_CUR_P1,
  MM_CUR_P2,
  MM_SEND_MODE,
  MM_SEND_SCREEN,
  MM_SEND_DATA,
  MM_FFT_MODE
}MM_Akt_Item_t;

//--------------------------------------------------------------
typedef enum _MENU_SETTINGS
{
   SETTING_TRIGGER = 0,
   SETTING_CH1,
   SETTING_CH2,
   SETTING_CURSOR,
   SETTING_FFT,
   SETTING_SEND,
   SETTING_VERSION,
   SETTING_HELP,
} MENU_SETTINGS;


typedef enum _ARROW_KEY
{
   ARROW_KEY_NONE  = 0,
   ARROW_KEY_LEFT  = 1,
   ARROW_KEY_RIGHT = 2,
   ARROW_KEY_UP    = 3,
   ARROW_KEY_DOWN  = 4
} ARROW_KEY_t;

typedef enum _TIME_SETTING
{
   TIME_SET_NONE = 0,
   TIME_SET_5s,
   TIME_SET_2s,
   TIME_SET_1s,
   TIME_SET_05s,
   TIME_SET_02s,
   TIME_SET_01s,
   TIME_SET_50ms,
   TIME_SET_20ms,
   TIME_SET_10ms,
   TIME_SET_5ms,
   TIME_SET_2ms,
   TIME_SET_1ms,
   TIME_SET_500us,
   TIME_SET_200us,
   TIME_SET_100us,
   TIME_SET_50us,
   TIME_SET_25us,
} TIME_SETTING_e;



typedef enum _CHANNEL_SELECTION
{
   CHANNEL_SEL_NONE = 0,
   CHANNEL_SEL_CH1,
   CHANNEL_SEL_CH2,
} CHANNEL_SELECTION_e;

typedef enum _VOLTAGE_SETTING
{
   VOLTAGE_SET_NONE = 0,
   VOLTAGE_SET_5V,
   VOLTAGE_SET_2V,
   VOLTAGE_SET_1V,
   VOLTAGE_SET_05V,
   VOLTAGE_SET_02V,
   VOLTAGE_SET_01V,
} VOLTAGE_SETTING_e;

typedef enum _TRIGGER_MODE
{
   TRIGGER_MODE_NORMAL = 0,
   TRIGGER_MODE_AUTO,
   TRIGGER_MODE_SINGLE
} TRIGGER_MODE_e;

typedef enum _TRIGGER_EDGE
{
   TRIGGER_EDGE_RISING,
   TRIGGER_EDGE_FALLING,
   TRIGGER_EDGE_BOUTH
} TRIGGER_EDGE_e;

typedef enum _TRIGGER_STATE_e
{
   TRIGGER_STATE_RUN = 0,
   TRIGGER_STATE_STOP,
   TRIGGER_STATE_CONT,
   TRIGGER_STATE_WAIT,
   TRIGGER_STATE_READY,
   TRIGGER_STATE_WAIT2,
} TRIGGER_STATE_e;

//--------------------------------------------------------------
// Struktur f�r die Main-Men�-Punkte
//--------------------------------------------------------------
typedef struct {
  char *txt;          // linke Seite vom Menu-Text
  uint16_t xp;        // Ypos an die das Menu gezeichnet wird
  uint16_t um_cnt;    // Anzahl der Menu-Unterpunkte
}MM_Item_t;

//--------------------------------------------------------------
// Struktur von einem Untermenu
//--------------------------------------------------------------
typedef struct {
  char *stxt;     // Text
}SM_Item_t;


//--------------------------------------------------------------
// Struktur von "Channel"
//--------------------------------------------------------------
typedef struct {
  uint32_t faktor;  // aktueller Vorteiler (5V,2V,1V usw)
  uint32_t visible; // sichtbar (On, Off)
  int16_t position; // Y-Position
}Channel_t;


//--------------------------------------------------------------
// Struktur von "Timebase"
//--------------------------------------------------------------
typedef struct {
  uint32_t value;    // Wert Timebase (5s,2s,1s,500ms usw)
}Timebase_t;


//--------------------------------------------------------------
// Struktur von "Trigger"
//--------------------------------------------------------------
typedef struct
{
  CHANNEL_SELECTION_e source;    // aktuelle Source (CH1, CH2)
  TRIGGER_EDGE_e  edge;      // Trigger-Flanke (HI, LO)
  TRIGGER_MODE_e  mode;      // Mode (Normal, Auto, Single)
  TRIGGER_STATE_e state;    // Single-Status
  uint16_t value_ch1; // Trigger-Value (CH1)
  uint16_t value_ch2; // Trigger-Value (CH2)
}Trigger_t;


//--------------------------------------------------------------
// Struktur von "Cursor"
//--------------------------------------------------------------
typedef struct {
  uint32_t mode;  // aktueller Mode (Off, CH1, CH2, Time)
  uint16_t p1;    // Wert Cursor-A (bei CH1 oder CH2)
  uint16_t p2;    // Wert Cursor-B (bei CH1 oder CH2)
  uint16_t t1;    // Wert Cursor-A (bei Time)
  uint16_t t2;    // Wert Cursor-B (bei Time)
  uint16_t f1;    // Wert Cursor-A (bei FFT)
}Cursor_t;


//--------------------------------------------------------------
// Struktur von "Send"
//--------------------------------------------------------------
typedef struct {
  uint32_t mode;  	// Mode (CH1,CH2,CH1+CH2)
  uint32_t screen;  // Screen
  uint32_t data;  	// Datamode (Offline, Start)
}Send_t;


//--------------------------------------------------------------
// Struktur von "FFT"
//--------------------------------------------------------------
typedef struct {
  uint32_t mode;  	// Mode (Off,CH1,CH2)
}FFT_t;


//--------------------------------------------------------------
// Struktur von "Menu"
//--------------------------------------------------------------
typedef struct {
  uint32_t akt_transparenz;  // aktuelle Transparenz
  uint32_t akt_setting;      // aktives Setting
  Channel_t ch1;             // Daten "Channel-1"
  Channel_t ch2;             // Daten "Channel-2"
  Timebase_t timebase;       // Daten "Timebase"
  Trigger_t trigger;         // Daten "Trigger"
  Cursor_t cursor;           // Daten "Cursor"
  Send_t send;               // Daten "Send"
  FFT_t fft;                 // Daten "FFT"
}Menu_t;
Menu_t Menu;



//--------------------------------------------------------------
// Struktur der "GUI"
//--------------------------------------------------------------
typedef struct {
  uint32_t gui_xpos;
  MM_Akt_Item_t akt_menu;
  MM_Akt_Item_t old_menu;
  GUI_Button_t akt_button;
  GUI_Button_t old_button;
}GUI_t;
GUI_t GUI;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void menu_draw_all(void);
MENU_Status_t menu_check_touch(void);


//--------------------------------------------------------------
#endif // __STM32F7_UB_MENU_H
