/**
  ******************************************************************************
  * @file         osci_if.h
  * @created by   Werner Wolfrum
  * @date         10.03.2019
  * @brief        header of osci recorder interface file

  * @template     audio_if.h
                  Original file name, author, description, copyright for template below.
  ******************************************************************************
  */
/**
  ******************************************************************************
  * @file    audio_if.h //WW: Corrected filename  
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
#ifndef __OSCI_IF_H
#define __OSCI_IF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Exported constants --------------------------------------------------------*/
#define DEFAULT_OSCI_IN_BIT_RESOLUTION  16
#define DEFAULT_OSCI_IN_CHANNEL_NBR      2
#define OSCI_OUT_BUFFER_SIZE                      (18 * 1024)
#define OSCI_IN_BUFFER_SIZE                       (8 * 2304) /* buffer size in half-word */
/* Exported types ------------------------------------------------------------*/
 /* Audio buffer control struct */

typedef enum
{
    OSCI_RECORDER_IDLE   =           0,
    OSCI_RECORDER_SUSPENDED =        1,
    OSCI_RECORDER_RECORDING =        2,
    OSCI_RECORDER_PLAYING   =        3,
    OSCI_RECORDER_RECORD_PAUSE =     4,
    OSCI_RECORDER_PLAY_PAUSE   =     5,
    OSCI_RECORDER_RECORD_EOF =       6,
    OSCI_RECORDER_PLAY_EOF =         7,
    OSCI_RECORDER_ERROR        =     0xFF,
  
}OSCI_IN_StateTypdef;

typedef enum
{
  OSCI_BUFFER_OFFSET_NONE = 0,
  OSCI_REC_BUFFER_AVAILABLE,
  //OSCI_REC_BUFFER_OFFSET_HALF,
  //OSCI_REC_BUFFER_OFFSET_FULL,
  OSCI_PLAY_BUFFER_OFFSET_HALF,
  OSCI_PLAY_BUFFER_OFFSET_FULL,
}
OSCI_BUFFER_StateTypeDef;


typedef enum
{
  OSCI_PLAYER_STOP = 0,
  OSCI_PLAYER_START,
  OSCI_PLAYER_PLAY,
  OSCI_PLAYER_PAUSE,
  OSCI_PLAYER_EOF,
  OSCI_PLAYER_ERROR,
  
}OSCI_OUT_StateTypdef;

typedef struct {
  uint32_t        volume;
  uint32_t        mute;   
  OSCI_OUT_StateTypdef state;
}OSCI_OUT_Process_t;

typedef enum {
	OSCI_BUFFER_EMPTY = 0,
	OSCI_BUFFER_FULL,
}OSCI_WR_BUFFER_StateTypeDef;

typedef struct {
  uint32_t        volume;
  uint32_t        fptr;  
  OSCI_IN_StateTypdef  state;
  uint8_t*        buffer;
  uint32_t        lenght;
}OSCI_IN_Process_t;

typedef struct 
{
  uint8_t*          buff;
  OSCI_OUT_Process_t    out;
  OSCI_IN_Process_t     in;


}OSCI_ProcessTypdef ;

#if 0
typedef struct 
{
  uint32_t   ChunkID;       /* 0 */ 
  uint32_t   FileSize;      /* 4 */
  uint32_t   FileFormat;    /* 8 */
  uint32_t   SubChunk1ID;   /* 12 */
  uint32_t   SubChunk1Size; /* 16*/  
  uint16_t   AudioFormat;   /* 20 */ 
  uint16_t   NbrChannels;   /* 22 */   
  uint32_t   SampleRate;    /* 24 */
  
  uint32_t   ByteRate;      /* 28 */
  uint16_t   BlockAlign;    /* 32 */  
  uint16_t   BitPerSample;  /* 34 */  
  uint32_t   SubChunk2ID;   /* 36 */   
  uint32_t   SubChunk2Size; /* 40 */    

}WAV_InfoTypedef ;   
#endif
   
typedef struct
{
  void  (*TransferComplete_CallBack)(void);
  void  (*HalfTransfer_CallBack)(void);  
  void  (*Error_CallBack)(void);
  
}OSCI_IFTypeDef;

typedef  void (*pFunc)(void);


/* Exported macros -----------------------------------------------------------*/
extern OSCI_ProcessTypdef hosci;
/* Exported functions ------------------------------------------------------- */
void OSCI_IF_RegisterCallbacks(pFunc  tc_cb,
                                pFunc  ht_cb, 
                                pFunc  err_cb);
#ifdef __cplusplus
}
#endif

#endif /*__OSCI_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
