/**
  ******************************************************************************
  * @file         adc.h
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *               Workaround for ADC trigger problem
  *               Adaption to 100MHz timer clock (instead of 84MHz).
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : adc.h
  * Datum    : 05.01.2014
  * Version  : 1.1
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

#ifndef __STM32F7_UB_ADC_H
#define __STM32F7_UB_ADC_H

#define WW_USE_AUDIO_BUFFER_FOR_ADC
#define ENABLE_ADC_DMA_CACHE_MAINTENANCE  1
//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f7xx.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_rcc.h"
#include "stm32f7xx_ll_adc.h"
#include "stm32f7xx_ll_dma.h"
#include "stm32f7xx_ll_tim.h"
//#include "misc.h"
//#include "menu.h"
#include "stm32_ub_led.h"

#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
#include "audio_if.h"
#endif
//--------------------------------------------------------------
// Liste aller ADC-Kan�le
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum {
  ADC_PA0  = 0,  // PA0
  ADC_PF10 = 1,  // PF10
  ADC_PF9  = 2,  // PF9
  ADC_PF8  = 3,  // PF8
}ADC1d_NAME_t;

#define  ADC1d_ANZ   3 // Anzahl von ADC1d_NAME_t (maximum = 16)


//--------------------------------------------------------------
// Defines vom ADC (nicht �ndern)
//--------------------------------------------------------------
#define  ADC_ARRAY_LEN         400 // Anzahl der Messwerte (pro Kanal)
#define  ADC_HALF_ARRAY_LEN    (ADC_ARRAY_LEN/2)



//--------------------------------------------------------------
// Globale ADC-Puffer
// Jeder Buffer ist so gross wie ein kompletter Oszi-Screen
// (400 Pixel x 3 Kanäle x 16bit)
//--------------------------------------------------------------
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC

extern AUDIO_ProcessTypdef haudio;
                               //       3     *2*        200          =  1200 Byte
#define ADC_HALF_BUFFER_BYTE_SIZE  ( ADC1d_ANZ*2*ADC_HALF_ARRAY_LEN)
#define ADC_DMA_Buffer_A_1  (haudio.buff)                               // 1200
#define ADC_DMA_Buffer_A_2  (haudio.buff+   ADC_HALF_BUFFER_BYTE_SIZE ) // 2400
#define ADC_DMA_Buffer_B_1  (haudio.buff+(2*ADC_HALF_BUFFER_BYTE_SIZE)) // 3600
#define ADC_DMA_Buffer_B_2  (haudio.buff+(3*ADC_HALF_BUFFER_BYTE_SIZE)) // 4800
#define ADC_DMA_Buffer_C_1  (haudio.buff+(4*ADC_HALF_BUFFER_BYTE_SIZE)) // 6000
#define ADC_DMA_Buffer_C_2  (haudio.buff+(5*ADC_HALF_BUFFER_BYTE_SIZE)) // 7200 <== fits pretty well into 18k ;-)

#else

extern volatile uint16_t ADC_DMA_Buffer_A[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (A) Roh-Daten per DMA
extern volatile uint16_t ADC_DMA_Buffer_B[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (B) Roh-Daten per DMA
extern volatile uint16_t ADC_DMA_Buffer_C[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (C) sortierte Daten
/*
  WW: ATTENTION!!!
   COMMON        0x0000000020017114     0x1c94 ./uboszi_lib/adc.o
                 0x0000000020017114                ADC_DMA_Buffer_A
                 0x0000000020017a74                ADC_DMA_Buffer_C
                 0x00000000200183d4                Menu
                 0x000000002001842c                ADC_UB
                 0x0000000020018440                ADC_DMA_Buffer_B
                 0x0000000020018da0                GUI
  So that was placed in
                 SRAM1 (240KB): 0x20010000 - 0x2004BFFF  (0x3BFFF = 240k)
  But that memory is to slow - especially ADC and SDIO share both DMA2 access!!!
  For SDIO this issue causes "HAL_SD_ERROR_TX_UNDERRUN" and write operations will fail!!!
  We need a placement in:
                 DTCM  ( 64KB): 0x20000000 - 0x2000FFFF  ( 0xFFFF =  64k)  0-wait states!!!

  But here we can use:
                 AUDIO_ProcessTypdef haudio __attribute__((section(".RamData1")));
  that goes to DTCM!!!
  It contains a 18KB "buff" that can be shared and reused (AUDIO is not active while OSCI is running):
                 #define AUDIO_OUT_BUFFER_SIZE (18 * 1024)
                 typedef struct
                 {
                    uint8_t          buff[AUDIO_OUT_BUFFER_SIZE];
                    OUT_Process_t    out;
                    IN_Process_t     in;
                 } AUDIO_ProcessTypdef ;
 */

#endif


//--------------------------------------------------------------
// Adressen der ADCs
// (siehe Seite 66+427 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_BASE_ADR        ((uint32_t)0x40012000)
#define ADC_COM_OFFSET      ((uint32_t)0x00000300)


//--------------------------------------------------------------
// Adressen der Register
// (siehe Seite 427+428 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_REG_CDR_OFFSET         0x08

#define ADC1_CDR_ADDRESS    (ADC_BASE_ADR | ADC_COM_OFFSET | ADC_REG_CDR_OFFSET)




//--------------------------------------------------------------
// ADC-Clock
// Max-ADC-Frq = 36MHz
// Grundfrequenz = APB2 (APB2=84MHz @ Sysclock=168MHz)
// M�gliche Vorteiler = 2,4,6,8
//
// Max-Conversion-Time @ 21MHz : (with minimum SampleTime)
//   12bit Resolution : 3+12=15 TCycles => 714ns
//   10bit Resolution : 3+10=13 TCycles => 619ns
//    8bit Resolution : 3+8 =11 TCycles => 523ns
//    6bit Resolution : 3+6 =9  TCycles => 428ns
//--------------------------------------------------------------

//#define ADC1d_VORTEILER     ADC_Prescaler_Div2 // Frq = 42 MHz
#define ADC1d_VORTEILER     ADC_Prescaler_Div4   // Frq = 21 MHz
//#define ADC1d_VORTEILER     ADC_Prescaler_Div6 // Frq = 14 MHz
//#define ADC1d_VORTEILER     ADC_Prescaler_Div8 // Frq = 10.5 MHz


//--------------------------------------------------------------
// DMA Einstellung
// (siehe Seite 304+305 vom Referenz Manual)
// Moegliche DMAs fuer ADC1 :
//   DMA2_STREAM0_CHANNEL0
//   DMA2_STREAM4_CHANNEL0
//--------------------------------------------------------------


#define ADC_DMA                   DMA2
#define ADC_DMA_STREAM            LL_DMA_STREAM_0
#define ADC_DMA_CHANNEL           LL_DMA_CHANNEL_0



//--------------------------------------------------------------
// Struktur eines ADC Kanals
//--------------------------------------------------------------
typedef struct {
  ADC1d_NAME_t ADC_NAME;  // Name
  GPIO_TypeDef* ADC_PORT; // Port
  const uint16_t ADC_PIN; // Pin
  const uint32_t ADC_CLK; // Clock
  const uint8_t ADC_CH;   // ADC-Kanal
}ADC1d_t;



//--------------------------------------------------------------
// Timer-4
//
// Grundfreq = 2*APB1 (APB1=50MHz @ Sysclock=200MHz) => TIM_CLK=100MHz
//
// ADC-FRQ = 100MHz / (PRESCALE+1) / (PERIODE+1)
//
//--------------------------------------------------------------
#define OSZI_TIM2_PERIODE      299
#define OSZI_TIM2_PRESCALE     99


//--------------------------------------------------------------
typedef enum {
  ADC_VORLAUF =0,
  ADC_RUNNING,
  ADC_PRE_TRIGGER,
  ADC_TRIGGER_OK,
  ADC_READY
}ADC_Status_t;


typedef void  (*OsciBufferTransfer_CallBack)(uint8_t* buff, uint32_t len);

//--------------------------------------------------------------
typedef struct {
  ADC_Status_t status;
  uint32_t trigger_pos;
  uint32_t trigger_quarter;
  uint32_t dma_status;
  OsciBufferTransfer_CallBack recordCallback;
}ADC_t;
ADC_t ADC_UB;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void ADC_Init_ALL(void);
void ADC_change_Frq(uint32_t n);
void ADC_change_Mode(uint32_t n);
void WW_Trigger_Processing(bool lowerHalfBuffer, bool targetA);



//--------------------------------------------------------------
#endif // __STM32F4_UB_ADC_H
