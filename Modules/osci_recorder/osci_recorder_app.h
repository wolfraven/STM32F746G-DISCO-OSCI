/**
  ******************************************************************************
  * @file         osci_recorder_app.h
  * @created by   Werner Wolfrum
  * @date         10.03.2019
  * @brief        header of osci recorder application file

  * @template     AUDIO_RECORDER_app.h
                  Original file name, author, description, copyright for template below.
  ******************************************************************************
  */
/**
  ******************************************************************************
  * @file    AUDIO_RECORDER_app.h
  * @author  MCD Application Team
  * @brief   header of audio player application file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OSCI_RECORDER_APP_H
#define __OSCI_RECORDER_APP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "../Common/audio_if.h"
#include "../osci_recorder/osci_if.h"

/* Exported constants --------------------------------------------------------*/
#define DEFAULT_REC_OSCI_VOLUME                   98
/* Exported types ------------------------------------------------------------*/

typedef enum
{
  OSCI_RECORDER_ERROR_NONE = 0,
  OSCI_RECORDER_ERROR_IO,
  OSCI_RECORDER_ERROR_HW,
  OSCI_RECORDER_ERROR_MEM,
  OSCI_RECORDER_ERROR_FORMAT_NOTSUPPORTED,
  
}OSCI_RECORDER_ErrorTypdef;

#define OSCI_RECORDER_StateTypdef    IN_StateTypdef
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_Init(uint8_t volume);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_SelectFile(char* file, uint8_t mode);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_StartRec(uint32_t frequency);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_DeInit(void);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_StopRec(void);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_StopPlay(void);
WAV_InfoTypedef*           OSCI_RECORDER_GetFileInfo(void);
uint32_t                   OSCI_RECORDER_GetElapsedTime(void);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_Stop(void);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_PauseResume(void);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_DeInit(void);
OSCI_RECORDER_StateTypdef  OSCI_RECORDER_GetState(void);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_Play(uint32_t frequency);
uint32_t                   OSCI_RECORDER_GetPlayedTime(void);
uint32_t                   OSCI_RECORDER_GetTotalTime(void);
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_StopPlayer(void);
void                       OSCI_RECORDER_RemoveAudioFile(char const*fname);

#ifdef __cplusplus
}
#endif

#endif /*__OSCI_RECORDER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
