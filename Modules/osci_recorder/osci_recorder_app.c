/**
  ******************************************************************************
  * @file         osci_recorder_app.c
  * @created by   Werner Wolfrum
  * @date         12.03.2019
  * @brief        Osci recorder application file

  * @template     AUDIO_RECORDER_app.c
                  Original file name, author, description, copyright for template below.
  ******************************************************************************
  */
/**
  ******************************************************************************
  * @file    AUDIO_RECORDER_app.c
  * @author  MCD Application Team
  * @brief   Audio player application functions
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
  
/* Includes ----------------------------------*/
#include "../osci_recorder/osci_recorder_app.h"
#include "oszi.h"

/** @addtogroup OSCI_PLAYER_MODULE
  * @{
  */

/** @defgroup OSCI_APPLICATION
  * @brief audio application routines
  * @{
  */


/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile FIL                    wav_file;
static osMessageQId           OsciEvent = 0;
static osThreadId             OsciThreadId = 0;
static WAV_InfoTypedef        OsciInfo;
static uint8_t                pHeaderBuff[44];
/* Private function prototypes -----------------------------------------------*/
static void Osci_Thread(void const * argument);
static uint32_t WavProcess_EncInit(uint32_t Freq, uint8_t* pHeader);
static uint32_t WavProcess_HeaderInit(uint8_t* pHeader, WAV_InfoTypedef* pAudioInfoStruct);
static uint32_t WavProcess_HeaderUpdate(uint8_t* pHeader, WAV_InfoTypedef* pAudioInfoStruct);

static void OSCI_IN_BufferTransferCallBack(uint8_t* buff, uint32_t len);

static void OSCI_TransferComplete_CallBack(void);
static void OSCI_HalfTransfer_CallBack(void);
static void OSCI_Error_CallBack(void);

extern void _cbOsciNotifyStateChange (void) ;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes audio
  * @param  None.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_Init(uint8_t volume)
{
  /* Initialize internal audio structure */

  hosci.in.volume = DEFAULT_REC_OSCI_VOLUME;
   
#if 0
  /* Register audio BSP drivers callbacks */
  OSCI_IF_RegisterCallbacks(OSCI_TransferComplete_CallBack,
                             OSCI_HalfTransfer_CallBack,
                             OSCI_Error_CallBack);
#endif
  
  /* Create Audio Queue */
  osMessageQDef(OSCI_Queue, 1, uint16_t);
  OsciEvent = osMessageCreate (osMessageQ(OSCI_Queue), NULL);
  
  /* Create Audio task */
  osThreadDef(osOsci_Thread, Osci_Thread, osPriorityNormal, 0, 256);
  OsciThreadId = osThreadCreate (osThread(osOsci_Thread), NULL);
  hosci.in.state  = OSCI_RECORDER_IDLE;
  return OSCI_RECORDER_ERROR_NONE;
}
   
/**
  * @brief  Get audio state
  * @param  None.
  * @retval Audio state.
  */
OSCI_RECORDER_StateTypdef  OSCI_RECORDER_GetState(void)
{
  return hosci.in.state;
}

/**
  * @brief  Get audio volume
  * @param  None.
  * @retval Audio volume.
  */
uint32_t  OSCI_RECORDER_GetVolume(void)
{
  return hosci.in.volume;
}

/**
  * @brief  Play audio stream
  * @param  frequency: Audio frequency used to play the audio stream.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_StartRec(uint32_t frequency)
{
  uint32_t byteswritten = 0;
  
  /* Initialize header file */
  uint32_t frequ = osciGetSampleFrequency();
  WavProcess_EncInit(frequ, pHeaderBuff);
  
  /* Write header file */
  if(f_write(&wav_file, pHeaderBuff, 44, (void*)&byteswritten) == FR_OK)
  {
    if(byteswritten != 0)
    {
      //BSP_OSCI_IN_Init(frequ, DEFAULT_OSCI_IN_BIT_RESOLUTION, DEFAULT_OSCI_IN_CHANNEL_NBR);
      //OSZI_RECORD_IN_Record(/*(uint16_t*)&hosci.buff[0], OSCI_IN_BUFFER_SIZE/2*/);
      // WW: Set recording callback
      hosci.in.buffer = 0;
      OSZI_SetRecordCallback( OSCI_IN_BufferTransferCallBack );

      if(hosci.in.state == OSCI_RECORDER_SUSPENDED)
      {
        osThreadResume(OsciThreadId);
      }
      hosci.in.state = OSCI_RECORDER_RECORDING;
      
      
      hosci.in.fptr = byteswritten;
      return OSCI_RECORDER_ERROR_NONE;
    }
  }
  return OSCI_RECORDER_ERROR_IO;
  
}

/**
  * @brief  Get the wav file information.
  * @param  file: wav file.
  * @param  info: pointer to wav file structure
  * @retval Audio state.
  */
WAV_InfoTypedef* OSCI_RECORDER_GetFileInfo(void)
{
  return &OsciInfo;
}


/**
  * @brief  Erase audio file from storage unit
  * @param  None
  * @retval None
  */
void OSCI_RECORDER_RemoveOsciFile(char const *fname)
{
  f_unlink(fname);
}

/**
  * @brief  Select wav file.
  * @param  file: wav file.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_SelectFile(char* file, uint8_t mode)
{
  int numOfReadBytes;

  OSCI_RECORDER_ErrorTypdef ret = OSCI_RECORDER_ERROR_IO;

#ifdef USE_SD_CARD_DISK
  f_mkdir("0:/Osci"); //TODO:WW: Warum hier "0" obwohl die SD-Karte als "1" eingehängt ist!?
#else
  f_mkdir("0:/Record");
#endif
  if( f_open(&wav_file, file, mode) == FR_OK) 
  {
    if (mode & FA_READ)
    {
      if(f_read(&wav_file, &OsciInfo, sizeof(WAV_InfoTypedef), (void *)&numOfReadBytes) == FR_OK)
      {
        ret = OSCI_RECORDER_ERROR_NONE;
      }
    }
  }
  return ret;  
}

#if 0
/**
  * @brief  Play audio stream
  * @param  frequency: Audio frequency used to play the audio stream.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_Play(uint32_t frequency)
{
  uint32_t numOfReadBytes;

  
  BSP_OSCI_OUT_Init(OUTPUT_DEVICE_AUTO, DEFAULT_REC_OSCI_VOLUME, DEFAULT_OSCI_IN_FREQ);
  BSP_OSCI_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
  
  /* Fill whole buffer @ first time */
  if(f_read(&wav_file, 
            &hosci.buff[0],
            OSCI_OUT_BUFFER_SIZE,
            (void *)&numOfReadBytes) == FR_OK)
  { 
    if(numOfReadBytes != 0)
    {
      if(hosci.in.state == OSCI_RECORDER_SUSPENDED)
      {
        osThreadResume(AudioThreadId);
      }
      hosci.in.state = OSCI_RECORDER_PLAYING;
      BSP_OSCI_OUT_Play((uint16_t*)&hosci.buff[0], OSCI_OUT_BUFFER_SIZE);
      return OSCI_RECORDER_ERROR_NONE;
    }
  }
  return OSCI_RECORDER_ERROR_IO;
  
}
#endif

/**
  * @brief  Audio player DeInit
  * @param  None.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_DeInit(void)
{
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  if(hosci.in.state == OSCI_RECORDER_RECORDING)
  {
    BSP_OSCI_IN_Stop(CODEC_PDWN_HW);
    BSP_OSCI_IN_DeInit();
    f_close(&wav_file); 
  }
  
  if(hosci.in.state == OSCI_RECORDER_PLAYING)
  {  
    BSP_OSCI_OUT_Stop(CODEC_PDWN_HW);
    BSP_OSCI_OUT_DeInit();
    f_close(&wav_file); 
  }
  
  hosci.in.state = OSCI_RECORDER_IDLE;
  
    if(OsciEvent != 0)
  {
    vQueueDelete(OsciEvent);
    OsciEvent = 0;
  }

#if 0
  /* Restore SAI PLL clock */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
#endif
  
  if(OsciThreadId != 0)
  {
    osThreadTerminate(OsciThreadId);
    OsciThreadId = 0;
  }
  return OSCI_RECORDER_ERROR_NONE;
}

/**
  * @brief  Stop audio stream.
  * @param  None.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_StopRec(void)
{
  uint32_t byteswritten = 0;
  OSCI_RECORDER_ErrorTypdef audio_error = OSCI_RECORDER_ERROR_IO;
  

  OSZI_RECORD_IN_Stop();
  hosci.in.state = OSCI_RECORDER_IDLE;
  if(f_lseek(&wav_file, 0) == FR_OK)
  {
    /* Update the wav file header save it into wav file */
    WavProcess_HeaderUpdate(pHeaderBuff, &OsciInfo);
    
    if(f_write(&wav_file, pHeaderBuff, sizeof(WAV_InfoTypedef), (void*)&byteswritten) == FR_OK)
    {   
      audio_error = OSCI_RECORDER_ERROR_NONE;
    }
  }

  hosci.in.state = OSCI_RECORDER_SUSPENDED;
  f_close(&wav_file);

  _cbOsciNotifyStateChange();
  
  if(OsciThreadId != 0)
  {  
    osThreadSuspend(OsciThreadId);
  }
  return audio_error;
}

#if 0
/**
  * @brief  Stop audio stream.
  * @param  None.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_StopPlayer(void)
{
  BSP_OSCI_OUT_Stop(CODEC_PDWN_HW);
  BSP_OSCI_OUT_DeInit();
  hosci.in.state = OSCI_RECORDER_SUSPENDED;
  f_close(&wav_file);
  _cbOsciNotifyStateChange();
  
  if(OsciThreadId != 0)
  {  
    osThreadSuspend(OsciThreadId);
  } 
  return OSCI_RECORDER_ERROR_NONE;
}
#endif
/**
  * @brief  Pause Audio stream
  * @param  None.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_PauseResume(void)
{
  if(hosci.in.state == OSCI_RECORDER_PLAYING)
  {  
    osThreadSuspend(OsciThreadId);
    BSP_OSCI_OUT_Pause();
    hosci.in.state = OSCI_RECORDER_PLAY_PAUSE;
  }
  else if(hosci.in.state == OSCI_RECORDER_RECORDING)
  {
    osThreadSuspend(OsciThreadId);
    BSP_OSCI_IN_Pause();
    hosci.in.state = OSCI_RECORDER_RECORD_PAUSE;
  }
  
  else if(hosci.in.state == OSCI_RECORDER_PLAY_PAUSE)
  { 
    osThreadResume(OsciThreadId);
    BSP_OSCI_OUT_Resume();
    hosci.in.state = OSCI_RECORDER_PLAYING;
  }
  else if(hosci.in.state == OSCI_RECORDER_RECORD_PAUSE)
  {
    osThreadResume(OsciThreadId);
    BSP_OSCI_IN_Resume();
    hosci.in.state = OSCI_RECORDER_RECORDING;
  }  
  return OSCI_RECORDER_ERROR_NONE;
}

/**
  * @brief  Resume Audio stream
  * @param  None.
  * @retval Audio state.
  */
OSCI_RECORDER_ErrorTypdef  OSCI_RECORDER_Resume(void)
{

  if(hosci.in.state == OSCI_RECORDER_PLAY_PAUSE)
  { 
      osThreadResume(OsciThreadId);
    BSP_OSCI_OUT_Resume();
    hosci.in.state = OSCI_RECORDER_PLAYING;
  }
  else if(hosci.in.state == OSCI_RECORDER_RECORD_PAUSE)
  {
      osThreadResume(OsciThreadId);
    BSP_OSCI_IN_Resume();
    hosci.in.state = OSCI_RECORDER_RECORDING;
  }
  return OSCI_RECORDER_ERROR_NONE;
}

#if 0
/**
  * @brief  Calculates the remaining file size and new position of the pointer.
  * @param  None
  * @retval None
  */
void BSP_OSCI_IN_TransferComplete_CallBack(void)
{
  osMessagePut ( OsciEvent, OSCI_REC_BUFFER_OFFSET_FULL, 0);
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_OSCI_IN_HalfTransfer_CallBack(void)
{ 
    osMessagePut ( OsciEvent, OSCI_REC_BUFFER_OFFSET_HALF, 0);
}
#endif
/**
  * @brief  Manages the DMA FIFO error interrupt.
  * @param  None
  * @retval None
  */
void BSP_OSCI_IN_Error_CallBack(void)
{
  hosci.in.state = OSCI_RECORDER_ERROR;
}


#define WW_NOTIFY_SKIPPED_TRANSFERS
uint16_t skippedTransfers = 0;
static void OSCI_IN_BufferTransferCallBack(uint8_t* buff, uint32_t len)
{
   if( hosci.in.buffer == 0 ) // If the buffer pointer is free for the next transfer?
   {
      hosci.in.buffer = buff;
      hosci.in.lenght = len;

#ifdef WW_NOTIFY_SKIPPED_TRANSFERS
      //if( skippedTransfers != 0 )
      {
    	  // If transfers skipped, note them into first Value of track 0 (REF)
    	  *((uint16_t*)(hosci.in.buffer)) = skippedTransfers;
    	  skippedTransfers = 0;
      }
#endif
      osMessagePut ( OsciEvent, OSCI_REC_BUFFER_AVAILABLE, 0);
   }
   else
   {
      skippedTransfers++;
   }
}

#if 0
/**
  * @brief  Manages the DMA Transfer complete interrupt.
  * @param  None
  * @retval None
  */
static void OSCI_TransferComplete_CallBack(void)
{
  if(hosci.in.state == OSCI_RECORDER_PLAYING)
  {
    BSP_OSCI_OUT_ChangeBuffer((uint16_t*)&hosci.buff[0], OSCI_OUT_BUFFER_SIZE /2);
    osMessagePut ( OsciEvent, OSCI_PLAY_BUFFER_OFFSET_FULL, 0);
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
static void OSCI_HalfTransfer_CallBack(void)
{ 
  if(hosci.in.state == OSCI_RECORDER_PLAYING)
  {
    BSP_OSCI_OUT_ChangeBuffer((uint16_t*)&hosci.buff[OSCI_OUT_BUFFER_SIZE /2], OSCI_OUT_BUFFER_SIZE /2);
    osMessagePut ( OsciEvent, OSCI_PLAY_BUFFER_OFFSET_HALF, 0);
  }
}
#endif
/**
  * @brief  Manages the DMA FIFO error interrupt.
  * @param  None
  * @retval None
  */
static void OSCI_Error_CallBack(void)
{
  hosci.in.state = OSCI_RECORDER_ERROR;
}

/**
  * @brief  Audio task
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
static void Osci_Thread(void const * argument)
{
  uint32_t numOfReadBytes, numOfWrittenBytes;    
  osEvent event;
  FRESULT fr;
  for(;;)
  {
    event = osMessageGet(OsciEvent, 100 );
    
    if( event.status == osEventMessage )
    {
#if 0
      if(hosci.in.state == OSCI_RECORDER_PLAYING)
      {
        switch(event.value.v)
        {
        case OSCI_PLAY_BUFFER_OFFSET_HALF:
          if(f_read(&wav_file, 
                    &hosci.buff[0],
                    OSCI_OUT_BUFFER_SIZE/2,
                    (void *)&numOfReadBytes) == FR_OK)
          { 
            if(numOfReadBytes == 0)
            {  
              OSCI_RECORDER_StopPlayer();
            } 
            
          }
          else
          {
            OSCI_RECORDER_StopPlayer();
          }
          break;  
          
        case OSCI_PLAY_BUFFER_OFFSET_FULL:
          if(f_read(&wav_file, 
                    &hosci.buff[OSCI_OUT_BUFFER_SIZE/2],
                    OSCI_OUT_BUFFER_SIZE/2,
                    (void *)&numOfReadBytes) == FR_OK)
          { 
            if(numOfReadBytes == 0)
            { 
              OSCI_RECORDER_StopPlayer();
            } 
          }
          else
          {
            OSCI_RECORDER_StopPlayer();
          }
          break;   
          
        default:
          break;
        }
      }
#endif
      /* Recording .... */
      if(hosci.in.state == OSCI_RECORDER_RECORDING)
      {
        switch(event.value.v)
        {
          
        case OSCI_REC_BUFFER_AVAILABLE:
          //SCB_CleanInvalidateDCache();
          //__disable_irq();
          fr = f_write(&wav_file, (uint8_t*)(hosci.in.buffer), hosci.in.lenght, (void*)&numOfWrittenBytes);
          //__enable_irq();
          if(fr == FR_OK)
          { 
            hosci.in.buffer = 0; // Set the buffer free for the next transfer
            //SCB_CleanDCache();
            if(numOfWrittenBytes == 0)
            { 
              OSCI_RECORDER_StopRec();
            } 
          }
          else
          {
            OSCI_RECORDER_StopRec();
          }
          hosci.in.fptr += numOfWrittenBytes;
          break; 
          
#if 0
        case OSCI_REC_BUFFER_OFFSET_FULL:
          if(f_write(&wav_file, (uint8_t*)(hosci.buff + OSCI_IN_BUFFER_SIZE/2),
                     OSCI_IN_BUFFER_SIZE/2,
                     (void*)&numOfWrittenBytes) == FR_OK)
          { 
            if(numOfWrittenBytes == 0)
            { 
              OSCI_RECORDER_StopRec();
            } 
          }
          else
          {
            OSCI_RECORDER_StopRec();
          }
          hosci.in.fptr += numOfWrittenBytes;
          break; 
#endif
          
        default:
          break;
        }
      }
      
    }

  }
}

/**
  * @brief  Return Elapsed recording Time
  * @param  None
  * @retval Elapsed Time in millisecond
  */

uint32_t OSCI_RECORDER_GetElapsedTime(void)
{
  uint32_t duration;
  
  duration = hosci.in.fptr / OsciInfo.ByteRate;
  return duration;
  
}

/**
  * @brief  Return remain playing time
  * @param  None
  * @retval Elapsed Time in millisecond
  */

uint32_t OSCI_RECORDER_GetPlayedTime(void)
{
  uint32_t duration;
  
  duration = (wav_file.fptr) / OsciInfo.ByteRate;
  return duration;
  
}


/**
  * @brief  Return remain playing time
  * @param  None
  * @retval Elapsed Time in millisecond
  */

uint32_t OSCI_RECORDER_GetTotalTime(void)
{
  uint32_t duration;
  
  duration = f_size(&wav_file) / OsciInfo.ByteRate;
  return duration;
  
}
/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Encoder initialization.
  * @param  Freq: Sampling frequency.
  * @param  pHeader: Pointer to the WAV file header to be written.  
  * @retval 0 if success, !0 else.
  */
static uint32_t WavProcess_EncInit(uint32_t Freq, uint8_t *pHeader)
{  
  /* Initialize the encoder structure */
	OsciInfo.SampleRate = Freq;        /* Audio sampling frequency */
	OsciInfo.NbrChannels = 3;          /* Number of channels: 1:Mono or 2:Stereo */
	OsciInfo.BitPerSample = 16;        /* Number of bits per sample (16, 24 or 32) */
	OsciInfo.FileSize = 0x001D4C00;    /* Total length of useful audio data (payload) */
	OsciInfo.SubChunk1Size = 44;       /* The file header chunk size */
	OsciInfo.ByteRate = (OsciInfo.SampleRate * \
                        (OsciInfo.BitPerSample/8) * \
						OsciInfo.NbrChannels);     /* Number of bytes per second  (sample rate * block align)  */
	OsciInfo.BlockAlign = OsciInfo.NbrChannels * \
                         (OsciInfo.BitPerSample/8); /* channels * bits/sample / 8 */
  
  /* Parse the wav file header and extract required information */
  if(WavProcess_HeaderInit(pHeader, &OsciInfo))
  {
    return 1;
  }
  return 0;
}

/**
  * @brief  Initialize the wave header file
  * @param  pHeader: Header Buffer to be filled
  * @param  pAudioInfoStruct: Pointer to the wave structure to be filled.
  * @retval 0 if passed, !0 if failed.
  */
static uint32_t WavProcess_HeaderInit(uint8_t* pHeader, WAV_InfoTypedef* pAudioInfoStruct)
{
  /* Write chunkID, must be 'RIFF'  ------------------------------------------*/
  pHeader[0] = 'R';
  pHeader[1] = 'I';
  pHeader[2] = 'F';
  pHeader[3] = 'F';
  
  /* Write the file length ---------------------------------------------------*/
  /* The sampling time: this value will be written back at the end of the 
     recording operation.  Example: 661500 Btyes = 0x000A17FC, byte[7]=0x00, byte[4]=0xFC */
  pHeader[4] = 0x00;
  pHeader[5] = 0x4C;
  pHeader[6] = 0x1D;
  pHeader[7] = 0x00;
  /* Write the file format, must be 'WAVE' -----------------------------------*/
  pHeader[8]  = 'W';
  pHeader[9]  = 'A';
  pHeader[10] = 'V';
  pHeader[11] = 'E';
  
  /* Write the format chunk, must be'fmt ' -----------------------------------*/
  pHeader[12]  = 'f';
  pHeader[13]  = 'm';
  pHeader[14]  = 't';
  pHeader[15]  = ' ';
  
  /* Write the length of the 'fmt' data, must be 0x10 ------------------------*/
  pHeader[16]  = 0x10;
  pHeader[17]  = 0x00;
  pHeader[18]  = 0x00;
  pHeader[19]  = 0x00;
  
  /* Write the audio format, must be 0x01 (PCM) ------------------------------*/
  pHeader[20]  = 0x01;
  pHeader[21]  = 0x00;
  
  /* Write the number of channels, ie. 0x01 (Mono) ---------------------------*/
  pHeader[22]  = pAudioInfoStruct->NbrChannels;
  pHeader[23]  = 0x00;
  
  /* Write the Sample Rate in Hz ---------------------------------------------*/
  /* Write Little Endian ie. 8000 = 0x00001F40 => byte[24]=0x40, byte[27]=0x00*/
  pHeader[24]  = (uint8_t)((pAudioInfoStruct->SampleRate & 0xFF));
  pHeader[25]  = (uint8_t)((pAudioInfoStruct->SampleRate >> 8) & 0xFF);
  pHeader[26]  = (uint8_t)((pAudioInfoStruct->SampleRate >> 16) & 0xFF);
  pHeader[27]  = (uint8_t)((pAudioInfoStruct->SampleRate >> 24) & 0xFF);
  
  /* Write the Byte Rate -----------------------------------------------------*/
  pHeader[28]  = (uint8_t)((pAudioInfoStruct->ByteRate & 0xFF));
  pHeader[29]  = (uint8_t)((pAudioInfoStruct->ByteRate >> 8) & 0xFF);
  pHeader[30]  = (uint8_t)((pAudioInfoStruct->ByteRate >> 16) & 0xFF);
  pHeader[31]  = (uint8_t)((pAudioInfoStruct->ByteRate >> 24) & 0xFF);
  
  /* Write the block alignment -----------------------------------------------*/
  pHeader[32]  = pAudioInfoStruct->BlockAlign;
  pHeader[33]  = 0x00;
  
  /* Write the number of bits per sample -------------------------------------*/
  pHeader[34]  = pAudioInfoStruct->BitPerSample;
  pHeader[35]  = 0x00;
  
  /* Write the Data chunk, must be 'data' ------------------------------------*/
  pHeader[36]  = 'd';
  pHeader[37]  = 'a';
  pHeader[38]  = 't';
  pHeader[39]  = 'a';
  
  /* Write the number of sample data -----------------------------------------*/
  /* This variable will be written back at the end of the recording operation */
  pHeader[40]  = 0x00;
  pHeader[41]  = 0x4C;
  pHeader[42]  = 0x1D;
  pHeader[43]  = 0x00;
  
  /* Return 0 if all operations are OK */
  return 0;
}

/**
  * @brief  Initialize the wave header file
  * @param  pHeader: Header Buffer to be filled
  * @param  pAudioInfoStruct: Pointer to the wave structure to be filled.
  * @retval 0 if passed, !0 if failed.
  */
static uint32_t WavProcess_HeaderUpdate(uint8_t* pHeader, WAV_InfoTypedef* pAudioInfoStruct)
{
  /* Write the file length ---------------------------------------------------*/
  /* The sampling time: this value will be written back at the end of the 
     recording operation.  Example: 661500 Btyes = 0x000A17FC, byte[7]=0x00, byte[4]=0xFC */
  pHeader[4] = (uint8_t)(hosci.in.fptr);
  pHeader[5] = (uint8_t)(hosci.in.fptr >> 8);
  pHeader[6] = (uint8_t)(hosci.in.fptr >> 16);
  pHeader[7] = (uint8_t)(hosci.in.fptr >> 24);
  /* Write the number of sample data -----------------------------------------*/
  /* This variable will be written back at the end of the recording operation */
  hosci.in.fptr -=44;
  pHeader[40] = (uint8_t)(hosci.in.fptr);
  pHeader[41] = (uint8_t)(hosci.in.fptr >> 8);
  pHeader[42] = (uint8_t)(hosci.in.fptr >> 16);
  pHeader[43] = (uint8_t)(hosci.in.fptr >> 24);
  
  /* Return 0 if all operations are OK */
  return 0;
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
