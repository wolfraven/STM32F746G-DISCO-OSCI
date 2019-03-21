/**
  ******************************************************************************
  * @file         osci_if.c
  * @created by   Werner Wolfrum
  * @date         10.03.2019
  * @brief        Implementation of osci recorder interface

  * @template     audio_if.c
                  Original file name, author, description, copyright for template below.
  ******************************************************************************
  */
/**
  ******************************************************************************
  * @file    audio_if.c
  * @author  MCD Application Team
  * @brief   Audio common interface
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
#include "../osci_recorder/osci_if.h"

/** @addtogroup OSCI_PLAYER_MODULE
  * @{
  */

/** @defgroup OSCI_APPLICATION
  * @brief audio application routines
  * @{
  */


/* External variables --------------------------------------------------------*/
 static OSCI_IFTypeDef  OsciIf;
 
OSCI_ProcessTypdef hosci;

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


 
 /**
  * @brief  Register OSCI callbacks
  * @param  callbacks
  * @retval None
  */
void OSCI_IF_RegisterCallbacks(pFunc  tc_cb,
                                pFunc  ht_cb, 
                                pFunc  err_cb)
{
	OsciIf.TransferComplete_CallBack = tc_cb;
	OsciIf.HalfTransfer_CallBack = ht_cb;
	OsciIf.Error_CallBack = err_cb;
}
/**
  * @brief  Manages the DMA Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_OSCI_OUT_TransferComplete_CallBack(void)
{
  if(OsciIf.TransferComplete_CallBack)
  {
	  OsciIf.TransferComplete_CallBack();
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_OSCI_OUT_HalfTransfer_CallBack(void)
{ 
  if (OsciIf.HalfTransfer_CallBack )
  {
	  OsciIf.HalfTransfer_CallBack();
  }
}

/**
  * @brief  Manages the DMA FIFO error interrupt.
  * @param  None
  * @retval None
  */
void BSP_OSCI_OUT_Error_CallBack(void)
{
  if(OsciIf.Error_CallBack)
  {
	  OsciIf.Error_CallBack();
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
