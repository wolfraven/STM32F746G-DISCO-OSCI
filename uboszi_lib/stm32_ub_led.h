/**
  ******************************************************************************
  * @file         stm32_ub_led.h
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : stm32_ub_led.h
  * Datum    : 24.10.2013
  * Version  : 1.0
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
#ifndef __STM32F7_UB_LED_H
#define __STM32F7_UB_LED_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f7xx.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_rcc.h"



//--------------------------------------------------------------
// Liste aller LEDs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum 
{
  LED_GREENx = 0,  // LED3 auf dem STM32F429-Discovery
}LED_NAME_t;

#define  LED_ANZ   1 // Anzahl von LED_NAME_t


//--------------------------------------------------------------
// Status einer LED
//--------------------------------------------------------------
typedef enum {
  LED_OFF = 0,  // LED AUS
  LED_ON        // LED EIN
}LED_STATUS_t;


//--------------------------------------------------------------
// Struktur einer LED
//--------------------------------------------------------------
typedef struct {
  LED_NAME_t LED_NAME;    // Name
  GPIO_TypeDef* LED_PORT; // Port
  const uint16_t LED_PIN; // Pin
  const uint32_t LED_CLK; // Clock
  LED_STATUS_t LED_INIT;  // Init
}LED_t;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_Led_Init(void);
void UB_Led_Off(LED_NAME_t led_name);
void UB_Led_On(LED_NAME_t led_name);
void UB_Led_Toggle(LED_NAME_t led_name);
void UB_Led_Switch(LED_NAME_t led_name, LED_STATUS_t wert);



//--------------------------------------------------------------
#endif // __STM32F7_UB_LED_H
