/**
  ******************************************************************************
  * @file         oszi.h
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : oszi.h
  * Datum    : 24.03.2014
  * Version  : 1.6
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
#ifndef __STM32F7_UB_OSZI_H
#define __STM32F7_UB_OSZI_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f7xx.h"
#include "stm32_ub_led.h"
//#include "stm32_ub_lcd_ili9341.h"
//#include "stm32_ub_graphic2d.h"
//#include "stm32_ub_font.h"
//#include "stm32_ub_touch_stmpe811.h"
//#include "stm32_ub_systick.h"
#include "stm32_ub_uart.h"
//#include "stm32_ub_button.h"
#include "adc.h"
#include "menu.h"
//#include "fft.h"
#include <stdio.h>





//--------------------------------------------------------------
// Definitionen der Oszi-Scala
// (nicht ab�ndern !!)
//--------------------------------------------------------------
#define  SCALE_START_X      15   // dont change
#define  SCALE_START_Y      24   // dont change
#define  FFT_START_Y        31   // dont change
#define  FFT_START_X        20   // dont change

#define  SCALE_SPACE        25   // dont change
#define  SCALE_ANZ_Y        9    // dont change
#define  SCALE_ANZ_X        17   // dont change

#define  SCALE_W            SCALE_SPACE*(SCALE_ANZ_X-1)
#define  SCALE_H            SCALE_SPACE*(SCALE_ANZ_Y-1)

#define  SCALE_MX_PIXEL     SCALE_START_X+SCALE_H
#define  SCALE_MY_PIXEL     SCALE_START_Y+SCALE_W

#define  SCALE_X_MITTE      SCALE_W/2
#define  SCALE_Y_MITTE      SCALE_H/2




//--------------------------------------------------------------
// Defines zum Umrechnen : ADC-Wert -> Pixel-Wert
//--------------------------------------------------------------
#define  FAKTOR_5V          SCALE_SPACE/6825
#define  FAKTOR_2V          SCALE_SPACE/2730
#define  FAKTOR_1V          SCALE_SPACE/1365
#define  FAKTOR_0V5         SCALE_SPACE/682.5
#define  FAKTOR_0V2         SCALE_SPACE/273
#define  FAKTOR_0V1         SCALE_SPACE/136.5


//--------------------------------------------------------------
// Defines zum Umrechnen : Zeit-Wert -> Pixel-Wert
//--------------------------------------------------------------
#define  FAKTOR_T           SCALE_W/4095



//--------------------------------------------------------------
// Defines zum Umrechnen : FFT-Wert -> Pixel-Wert
//--------------------------------------------------------------
#define  FAKTOR_F           FFT_VISIBLE_LENGTH/4095


//--------------------------------------------------------------
// Aktuallisierungs-Intervall der GUI
//--------------------------------------------------------------
#define  GUI_INTERVALL_MS     50   // interall der GUI (in ms)

//--------------------------------------------------------------
// color defines
//--------------------------------------------------------------
#define  BACKGROUND_COL     GUI_BLACK  // Farbe vom Hintergrund
#define  FONT_COL           GUI_BLUE   // (debug Schriftfarbe)
#define  ADC_CH1_COL        GUI_CYAN   // Farbe von Kanal-1
#define  ADC_CH2_COL        GUI_YELLOW // Faebe von Kanal-2
#define  SCALE_COL          GUI_GRAY   // Farbe vom Oszi-Gitter
#define  CURSOR_COL         GUI_GREEN  // Farbe vom Cursor
#define  FFT_COL            GUI_MAGENTA  // Farbe der FFT


//--------------------------------------------------------------
#define  BMP_HEADER_LEN   54 // Header = 54 Bytes (dont change)


typedef struct _CHANNEL_CONFIG
{
   CHANNEL_SELECTION_e channel;
   VOLTAGE_SETTING_e   voltage;
   bool                visible;
   int16_t             position;
} CHANNEL_CONFIG_t;

typedef struct _TRIGGER_CONFIG
{
   CHANNEL_SELECTION_e channel;
   bool                enabled;
   int16_t             position;
   TRIGGER_EDGE_e      edge;      // Trigger-Flanke (HI, LO)
   TRIGGER_MODE_e      mode;      // Mode (Normal, Auto, Single)
   TRIGGER_STATE_e     single;    // Single-Status
   uint16_t            value_ch1; // Trigger-Value (CH1)
   uint16_t            value_ch2; // Trigger-Value (CH2)
} TRIGGER_CONFIG_t;

typedef struct _CURSOR_CONFIG
{
   CHANNEL_SELECTION_e channel;
   VOLTAGE_SETTING_e   voltage;
   bool                enabled;
   int16_t             position;
} CURSOR_CONFIG_t;

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void oszi_init(void);
void oszi_start(void);
void oszi_process(void);
void osziHandleArrowKey(ARROW_KEY_t key);
uint32_t osciGetSampleFrequency(void);

int getActiveMenuItem(void);

uint8_t OSZI_RECORD_IN_Record(/*uint16_t* pbuf, uint32_t size*/);
uint8_t OSZI_RECORD_IN_Stop();

void osziSetTriggerChannel(CHANNEL_SELECTION_e ch);
void osziSetTriggerEdge(TRIGGER_EDGE_e te);
void osziSetTriggerMode(TRIGGER_MODE_e tm);
void osziSetTriggerValue(CHANNEL_SELECTION_e ch, uint16_t value);


//--------------------------------------------------------------
#endif // __STM32F7_UB_OSZI_H
