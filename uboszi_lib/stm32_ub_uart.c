/**
  ******************************************************************************
  * @file         stm32_ub_uart.c
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : stm32_ub_led.c
  * Datum    : 28.11.2013
  * Version  : 1.1
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include <stm32_ub_uart.h>

//--------------------------------------------------------------
// Definition aller UARTs
// Reihenfolge wie bei UART_NAME_t
//--------------------------------------------------------------
UART_t UART[] = {
// Name, Clock               , AF-UART      ,UART  , Baud , Interrupt
  {COM1x,LL_APB2_GRP1_PERIPH_USART1,USART1,115200,USART1_IRQn, // UART1 mit 115200 Baud
// PORT , PIN      , Clock              , Source
  {GPIOA,LL_GPIO_PIN_9,LL_AHB1_GRP1_PERIPH_GPIOA,LL_GPIO_AF_7},  // TX an PA9
  {GPIOB,LL_GPIO_PIN_7,LL_AHB1_GRP1_PERIPH_GPIOB,LL_GPIO_AF_7}}, // RX an PB7
};



//--------------------------------------------------------------
// init aller UARTs
//--------------------------------------------------------------
void UB_Uart_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  UART_NAME_t nr;

  for(nr=0;nr<UART_ANZ;nr++) {

    // Clock enable der TX und RX Pins
    LL_AHB1_GRP1_EnableClock(UART[nr].TX.CLK);
    LL_AHB1_GRP1_EnableClock(UART[nr].RX.CLK);

    // Clock enable der UART
    if((UART[nr].UART==USART1) || (UART[nr].UART==USART6)) {
      LL_APB2_GRP1_EnableClock(UART[nr].CLK);
    }
    else {
    	LL_APB1_GRP1_EnableClock(UART[nr].CLK);
    }

    // UART als Alternative-Funktion mit PushPull
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;

    // TX-Pin
    GPIO_InitStruct.Pin = UART[nr].TX.PIN;
    GPIO_InitStruct.Alternate = UART[nr].TX.AF;
    LL_GPIO_Init(UART[nr].TX.PORT, &GPIO_InitStruct);

    // RX-Pin
    GPIO_InitStruct.Pin = UART[nr].RX.PIN;
    GPIO_InitStruct.Alternate = UART[nr].RX.AF;
    LL_GPIO_Init(UART[nr].TX.PORT, &GPIO_InitStruct);

    USART_InitStruct.BaudRate = UART[nr].BAUD;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USART1);
    LL_USART_Enable(USART1);

    // RX-Interrupt enable
    LL_USART_EnableIT_RXNE(UART[nr].UART);

    // enable UART Interrupt-Vector
    NVIC_SetPriority(UART[nr].INT, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(UART[nr].INT);

    // RX-Puffer vorbereiten
    UART_RX[nr].rx_buffer[0]=RX_END_CHR;
    UART_RX[nr].wr_ptr=0;
    UART_RX[nr].status=RX_EMPTY;
  }

}

//--------------------------------------------------------------
// ein Byte per UART senden
//--------------------------------------------------------------
void UB_Uart_SendByte(UART_NAME_t uart, uint16_t wert)
{
  // warten bis altes Byte gesendet wurde
  while (LL_USART_IsActiveFlag_TXE(UART[uart].UART) == RESET);
  LL_USART_TransmitData8(UART[uart].UART, wert);
}

//--------------------------------------------------------------
// einen String per UART senden
//--------------------------------------------------------------
void UB_Uart_SendString(UART_NAME_t uart, char *ptr, UART_LASTBYTE_t end_cmd)
{
  // sende kompletten String
  while (*ptr != 0) {
    UB_Uart_SendByte(uart,*ptr);
    ptr++;
  }
  // eventuell Endekennung senden
  if(end_cmd==LFCR) {
    UB_Uart_SendByte(uart,0x0A); // LineFeed senden
    UB_Uart_SendByte(uart,0x0D); // CariageReturn senden
  }
  else if(end_cmd==CRLF) {    
    UB_Uart_SendByte(uart,0x0D); // CariageReturn senden
    UB_Uart_SendByte(uart,0x0A); // LineFeed senden
  }
  else if(end_cmd==LF) {    
    UB_Uart_SendByte(uart,0x0A); // LineFeed senden
  }
  else if(end_cmd==CR) {    
    UB_Uart_SendByte(uart,0x0D); // CariageReturn senden    
  }
}

//--------------------------------------------------------------
// einen String per UART empfangen
// (der Empfang wird per Interrupt abgehandelt)
// diese Funktion muss zyklisch gepollt werden
// Return Wert :
//  -> wenn nichts empfangen = RX_EMPTY
//  -> wenn String empfangen = RX_READY -> String steht in *ptr
//  -> wenn Puffer voll      = RX_FULL
//--------------------------------------------------------------
UART_RXSTATUS_t UB_Uart_ReceiveString(UART_NAME_t uart, char *ptr)
{
  UART_RXSTATUS_t ret_wert=RX_EMPTY;
  uint8_t n,wert;

  if(UART_RX[uart].status==RX_READY) {
    ret_wert=RX_READY;
    // Puffer kopieren
    n=0;
    do {
      wert=UART_RX[uart].rx_buffer[n];
      if(wert!=RX_END_CHR) {
        ptr[n]=wert;
        n++;
      }
    }while(wert!=RX_END_CHR);
    // Stringendekennung
    ptr[n]=0x00;
    // RX-Puffer l�schen
    UART_RX[uart].rx_buffer[0]=RX_END_CHR;
    UART_RX[uart].wr_ptr=0;
    UART_RX[uart].status=RX_EMPTY;
  }
  else if(UART_RX[uart].status==RX_FULL) {
    ret_wert=RX_FULL;
    // RX-Puffer l�schen
    UART_RX[uart].rx_buffer[0]=RX_END_CHR;
    UART_RX[uart].wr_ptr=0;
    UART_RX[uart].status=RX_EMPTY;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// speichern des empfangenen Zeichens im Puffer
//--------------------------------------------------------------
void P_UART_Receive(UART_NAME_t uart, uint16_t wert)
{
  if(UART_RX[uart].wr_ptr<RX_BUF_SIZE) {
    // wenn noch Platz im Puffer
    if(UART_RX[uart].status==RX_EMPTY) {
      // wenn noch keine Endekennung empfangen wurde
      if((wert>=RX_FIRST_CHR) && (wert<=RX_LAST_CHR)) {
        // Byte im Puffer speichern
        UART_RX[uart].rx_buffer[UART_RX[uart].wr_ptr]=wert;
        UART_RX[uart].wr_ptr++;
      }
      if(wert==RX_END_CHR) {
        // wenn Endekennung empfangen
        UART_RX[uart].rx_buffer[UART_RX[uart].wr_ptr]=wert;
        UART_RX[uart].status=RX_READY;
      }
    }
  }
  else {
    // wenn Puffer voll ist
    UART_RX[uart].status=RX_FULL;
  }
}


//--------------------------------------------------------------
// interne Funktion
// UART-Interrupt-Funktion
// Interrupt-Nr muss �bergeben werden
//--------------------------------------------------------------
void P_UART_RX_INT(uint8_t int_nr, uint16_t wert)
{
  UART_NAME_t nr;

  // den passenden Eintrag suchen
  for(nr=0;nr<UART_ANZ;nr++) {
    if(UART[nr].INT==int_nr) {
      // eintrag gefunden, Byte speichern
      P_UART_Receive(nr,wert);
      break;
    }
  }
}


//--------------------------------------------------------------
// UART1-Interrupt
//--------------------------------------------------------------
void USART1_IRQHandler(void) {
  uint16_t wert;

  if (LL_USART_IsActiveFlag_RXNE(USART1) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=LL_USART_ReceiveData8(USART1);
    // Byte speichern
    P_UART_RX_INT(USART1_IRQn,wert);
  }
}

#if 0
//--------------------------------------------------------------
// UART2-Interrupt
//--------------------------------------------------------------
void USART2_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(USART2);
    // Byte speichern
    P_UART_RX_INT(USART2_IRQn,wert);
  }
}


//--------------------------------------------------------------
// UART3-Interrupt
//--------------------------------------------------------------
void USART3_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(USART3);
    // Byte speichern
    P_UART_RX_INT(USART3_IRQn,wert);
  }
}

//--------------------------------------------------------------
// UART4-Interrupt
//--------------------------------------------------------------
void UART4_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(UART4);
    // Byte speichern
    P_UART_RX_INT(UART4_IRQn,wert);
  }
}

//--------------------------------------------------------------
// UART5-Interrupt
//--------------------------------------------------------------
void UART5_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(UART5);
    // Byte speichern
    P_UART_RX_INT(UART5_IRQn,wert);
  }
}

//--------------------------------------------------------------
// UART6-Interrupt
//--------------------------------------------------------------
void USART6_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(USART6, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(USART6);
    // Byte speichern
    P_UART_RX_INT(USART6_IRQn,wert);
  }
}

//--------------------------------------------------------------
// UART7-Interrupt
//--------------------------------------------------------------
void UART7_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(UART7, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(UART7);
    // Byte speichern
    P_UART_RX_INT(UART7_IRQn,wert);
  }
}

//--------------------------------------------------------------
// UART8-Interrupt
//--------------------------------------------------------------
void UART8_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(UART8, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(UART8);
    // Byte speichern
    P_UART_RX_INT(UART8_IRQn,wert);
  }
}
#endif
