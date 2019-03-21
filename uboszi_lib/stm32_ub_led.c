/**
  ******************************************************************************
  * @file         stm32_ub_led.c
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : stm32_ub_led.c
  * Datum    : 24.10.2013
  * Version  : 1.0
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include <stm32_ub_led.h>


//--------------------------------------------------------------
// Definition aller LEDs
// Reihenfolge wie bei LED_NAME_t
//
// Init : [LED_OFF,LED_ON]
//--------------------------------------------------------------
LED_t LED[] = {
  // Name    ,PORT , PIN       , CLOCK              , Init
  {LED_GREENx ,GPIOI,LL_GPIO_PIN_1,LL_AHB1_GRP1_PERIPH_GPIOA,LED_OFF},   // PG13=Gruene LED auf dem Discovery-Board
};



//--------------------------------------------------------------
// Init aller LEDs
//--------------------------------------------------------------
void UB_Led_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LED_NAME_t led_name;
  
  for(led_name=0;led_name<LED_ANZ;led_name++) {
    // Clock Enable
    LL_AHB1_GRP1_EnableClock(LED[led_name].LED_CLK);

    // Config als Digital-Ausgang
    GPIO_InitStruct.Pin = LED[led_name].LED_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(LED[led_name].LED_PORT, &GPIO_InitStruct);

    // Default Wert einstellen
    if(LED[led_name].LED_INIT==LED_OFF) {
      UB_Led_Off(led_name);
    }
    else {
      UB_Led_On(led_name);
    }
  }
}


//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void UB_Led_Off(LED_NAME_t led_name)
{
  LED[led_name].LED_PORT->BSRR = (LED[led_name].LED_PIN)<<16;
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void UB_Led_On(LED_NAME_t led_name)
{
  LED[led_name].LED_PORT->BSRR = LED[led_name].LED_PIN;
} 

//--------------------------------------------------------------
// LED toggeln
//--------------------------------------------------------------
void UB_Led_Toggle(LED_NAME_t led_name)
{
  LED[led_name].LED_PORT->ODR ^= LED[led_name].LED_PIN;
}

//--------------------------------------------------------------
// LED ein- oder ausschalten
//--------------------------------------------------------------
void UB_Led_Switch(LED_NAME_t led_name, LED_STATUS_t wert)
{
  if(wert==LED_OFF) {
    UB_Led_Off(led_name);
  }
  else {
    UB_Led_On(led_name);
  }
}
