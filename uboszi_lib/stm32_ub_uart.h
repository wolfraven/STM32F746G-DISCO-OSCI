/**
  ******************************************************************************
  * @file         stm32_ub_uart.h
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : stm32_ub_led.h
  * Datum    : 28.11.2013
  * Version  : 1.1
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
#ifndef __STM32F7_UB_UART_H
#define __STM32F7_UB_UART_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f7xx.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_rcc.h"
#include "stm32f7xx_ll_usart.h"
//#include "misc.h"


//--------------------------------------------------------------
// Liste aller UARTs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum
{
  COM1x = 0   // COM1 (TX=PA9, RX=PA10)
}UART_NAME_t;

#define  UART_ANZ   1 // Anzahl von UART_NAME_t


//--------------------------------------------------------------
// Endekennung beim Senden
//--------------------------------------------------------------
typedef enum {
  NONE = 0,  // keine Endekennung
  LFCR,      // LineFeed + CarriageReturn (0x0A,0x0D)
  CRLF,      // CarriageReturn + LineFeed (0x0D,0x0A)
  LF,        // nur LineFeed (0x0A)
  CR         // nur CarriageReturn (0x0D)
}UART_LASTBYTE_t;


//--------------------------------------------------------------
// Status beim Empfangen
//--------------------------------------------------------------
typedef enum {
  RX_EMPTY = 0,  // nichts empfangen
  RX_READY,      // es steht was im Empfangspuffer
  RX_FULL        // RX-Puffer ist voll
}UART_RXSTATUS_t;


//--------------------------------------------------------------
// Struktur eines UART-Pins
//--------------------------------------------------------------
typedef struct {
  GPIO_TypeDef* PORT;     // Port
  const uint16_t PIN;     // Pin
  const uint32_t CLK;     // Clock
  const uint8_t AF;         // AF
}UART_PIN_t;

//--------------------------------------------------------------
// Struktur eines UARTs
//--------------------------------------------------------------
typedef struct {
  UART_NAME_t UART_NAME;    // Name
  const uint32_t CLK;       // Clock
  USART_TypeDef* UART;      // UART
  const uint32_t BAUD;      // Baudrate
  const uint8_t INT;        // Interrupt
  UART_PIN_t TX;            // TX-Pin
  UART_PIN_t RX;            // RX-Pin
}UART_t;


//--------------------------------------------------------------
// Defines fuer das Empfangen
//--------------------------------------------------------------
#define  RX_BUF_SIZE   50    // Gr�sse vom RX-Puffer in Bytes
#define  RX_FIRST_CHR  0x20  // erstes erlaubte Zeichen (Ascii-Wert)
#define  RX_LAST_CHR   0x7E  // letztes erlaubt Zeichen (Ascii-Wert)
#define  RX_END_CHR    0x0D  // Endekennung (Ascii-Wert)


//--------------------------------------------------------------
// Struktur f�r UART_RX
//--------------------------------------------------------------
typedef struct {
  char rx_buffer[RX_BUF_SIZE]; // RX-Puffer
  uint8_t wr_ptr;              // Schreib Pointer
  UART_RXSTATUS_t status;      // RX-Status
}UART_RX_t;
UART_RX_t UART_RX[UART_ANZ];


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_Uart_Init(void);
void UB_Uart_SendByte(UART_NAME_t uart, uint16_t wert);
void UB_Uart_SendString(UART_NAME_t uart, char *ptr, UART_LASTBYTE_t end_cmd);
UART_RXSTATUS_t UB_Uart_ReceiveString(UART_NAME_t uart, char *ptr);



//--------------------------------------------------------------
#endif // __STM32F7_UB_UART_H
