/**  ******************************************************************************  * @file         k_storage.c  * @modified by  Werner Wolfrum  * @date         10.03.2019  * @modification Adaption in drive asignment for usage with SD card.                  Insert "SD_DISK_UNIT"                    Insert compile switch "USE_SD_CARD_DISK"
                  Mounted sd driver manually <- TODO:WW:replace by event handling                   Original file name, author, description, copyright below.  ******************************************************************************  *//**
  ******************************************************************************
  * @file    k_storage.c
  * @author  MCD Application Team
  * @brief   This file provides the kernel storage functions
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
#include "k_storage.h"
#ifdef USE_SD_CARD_DISK
#include "sd_diskio.h"
#endif

/** @addtogroup CORE
  * @{
  */

/** @defgroup KERNEL_STORAGE
  * @brief Kernel storage routines
  * @{
  */


/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
static struct {
  U32 Mask;
  char c;
} _aAttrib[] = {
  { AM_RDO, 'R' },
  { AM_HID, 'H' },
  { AM_SYS, 'S' },
  { AM_DIR, 'D' },
  { AM_ARC, 'A' },
};
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FATFS USBDISK_FatFs;         /* File system object for USB disk logical drive */
char USBDISK_Drive[4];       /* USB Host logical drive number */
USBH_HandleTypeDef  hUSB_Host;
#ifdef USE_SD_CARD_DISK
FATFS SDDISK_FatFs;          /* File system object for SD disk logical drive  */
char  SDDISK_Drive[4];       /* SD CARD logical drive number */
#endif
osMessageQId StorageEvent;
DIR dir;
static char         acAttrib[10];
static char         acExt[FILEMGR_MAX_EXT_SIZE];

static uint32_t StorageStatus[NUM_DISK_UNITS];
/* Private function prototypes -----------------------------------------------*/
static void StorageThread(void const * argument);
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id);
#ifdef USE_SD_CARD_DISK
void SD_EXTI_InterruptHandler(void);
#endif
static void GetExt(char * pFile, char * pExt);
/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Storage drives initialization
  * @param  None 
  * @retval None
  */
BYTE testBuff[512];
void k_StorageInit(void)
{
#ifdef USE_SD_CARD_DISK
  /* Link the USB Host disk I/O driver */
  FATFS_LinkDriver(&SD_Driver, SDDISK_Drive);

  //BSP_SD_Init();

  /* Init SD Card */
#if 1
  SD_initialize(0); //WW: Disk driver in "sd_diskio_dma_rtos.c" does not use "lun"!
                    //    So it can be zero.
  HAL_SD_CardInfoTypeDef CardInfo;
  BSP_SD_GetCardInfo(&CardInfo);

  SD_read(0, &testBuff, 0, 1);

#else
  // Init will be done by fatfs -> diskio.c -> disk_initialize()
  // So no need to do it here!
#endif
#endif

#if 0 //def USE_USBH_MSD_DISK
  /* Link the USB Host disk I/O driver */
  FATFS_LinkDriver(&USBH_Driver, USBDISK_Drive);
  
  /* Init Host Library */
  USBH_Init(&hUSB_Host, USBH_UserProcess, 0);
#endif
  
    /* Create USB background task */
  osThreadDef(STORAGE_Thread, StorageThread, osPriorityLow, 0, 512);
  osThreadCreate (osThread(STORAGE_Thread), NULL);
  
  /* Create Storage Message Queue */
  osMessageQDef(osqueue, 10, uint16_t);
  StorageEvent = osMessageCreate (osMessageQ(osqueue), NULL);
  
#if 0 //def USE_USBH_MSD_DISK
  /* Add Supported Class */
  USBH_RegisterClass(&hUSB_Host, USBH_MSC_CLASS);
  
  /* Start Host Process */
  USBH_Start(&hUSB_Host);
#endif

#if 1 //TODO:WW:Hack!!! Remove if event handler is realized!!!
  f_mount(&SDDISK_FatFs,SDDISK_Drive,  0);
  StorageStatus[SD_DISK_UNIT] = 1;
#endif
}

/**
  * @brief  Storage Thread
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
static void StorageThread(void const * argument)
{
  osEvent event;
  
  for( ;; )
  {
    event = osMessageGet( StorageEvent, osWaitForever );
    
    if( event.status == osEventMessage )
    {
      switch(event.value.v)
      {
      case USBDISK_CONNECTION_EVENT:
        f_mount(&USBDISK_FatFs,USBDISK_Drive,  0);
        StorageStatus[USB_DISK_UNIT] = 1;
        break;
        
      case USBDISK_DISCONNECTION_EVENT:
        f_mount(0, USBDISK_Drive, 0);
        StorageStatus[USB_DISK_UNIT] = 0;
        break;

#ifdef USE_SD_CARD_DISK
      case SDDISK_INSERT_EVENT:
        f_mount(&SDDISK_FatFs,SDDISK_Drive,  0);
        StorageStatus[SD_DISK_UNIT] = 1;
        break;

      case SDDISK_REMOVE_EVENT:
        f_mount(0, SDDISK_Drive, 0);
        StorageStatus[SD_DISK_UNIT] = 0;
        break;
#endif
      }
    }
  }
}

/**
  * @brief  Storage get status
  * @param  unit: logical storage unit index.
  * @retval int
  */
uint8_t k_StorageGetStatus (uint8_t unit)
{  
  return StorageStatus[unit];
}

/**
  * @brief  Storage get capacity
  * @param  unit: logical storage unit index.
  * @retval int
  */
uint32_t k_StorageGetCapacity (uint8_t unit)
{  
  uint32_t   tot_sect = 0;
  FATFS *fs;
  
  if(unit == USB_DISK_UNIT)
  {
    fs = &USBDISK_FatFs;
    tot_sect = (fs->n_fatent - 2) * fs->csize;
  }
#ifdef USE_SD_CARD_DISK
  else if(unit == SD_DISK_UNIT)
  {
    fs = &SDDISK_FatFs;
    tot_sect = (fs->n_fatent - 2) * fs->csize;
  }
#endif
  return (tot_sect);
}

/**
  * @brief  Storage get free space
  * @param  unit: logical storage unit index. 
  * @retval int
  */
uint32_t k_StorageGetFree (uint8_t unit)
{ 
  uint32_t   fre_clust = 0;
  FATFS *fs ;
  FRESULT res = FR_INT_ERR;
  
  if(unit == USB_DISK_UNIT)
  {
    fs = &USBDISK_FatFs;
    res = f_getfree("0:", (DWORD *)&fre_clust, &fs);
  }
#ifdef USE_SD_CARD_DISK
  else if(unit == SD_DISK_UNIT)
  {
    fs = &SDDISK_FatFs;
    res = f_getfree("1:", (DWORD *)&fre_clust, &fs);
  }
#endif
  
  if(res == FR_OK)
  {
    return (fre_clust * fs->csize);
  }
  else
  {
    return 0;
  }
}
/**
  * @brief  User Process
  * @param  phost: Host handle
  * @param  id:    Host Library user message ID
  * @retval None
  */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{  
  switch (id)
  { 
  case HOST_USER_SELECT_CONFIGURATION:
    break;
    
  case HOST_USER_DISCONNECTION:
    osMessagePut ( StorageEvent, USBDISK_DISCONNECTION_EVENT, 0);
    break;
    
  case HOST_USER_CLASS_ACTIVE:
    osMessagePut ( StorageEvent, USBDISK_CONNECTION_EVENT, 0);
    break;
  }
}

#if 1 //def USE_SD_CARD_DISK
/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void SD_Detect_InterruptCallback(void)
{
  if( BSP_SD_IsDetected() == SD_PRESENT )
  {
    osMessagePut ( StorageEvent, SDDISK_INSERT_EVENT, 0);
  }
  else
  {
    osMessagePut ( StorageEvent, SDDISK_REMOVE_EVENT, 0);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   if( GPIO_Pin == SD_DETECT_PIN)
   {
      SD_Detect_InterruptCallback();
   }
}
#endif

/**
  * @brief  Return file extension and removed from file name.
  * @param  pFile: pointer to the file name.
  * @param  pExt:  pointer to the file extension
  * @retval None
  */
static void GetExt(char * pFile, char * pExt) 
{
  int Len;
  int i;
  int j;
  
  /* Search beginning of extension */
  Len = strlen(pFile);
  for (i = Len; i > 0; i--) {
    if (*(pFile + i) == '.') {
      *(pFile + i) = '\0';     /* Cut extension from filename */
      break;
    }
  }
  
  /* Copy extension */
  j = 0;
  while (*(pFile + ++i) != '\0') {
    *(pExt + j) = *(pFile + i);
    j++;
  }
  *(pExt + j) = '\0';          /* Set end of string */
}

/**
  * @brief  Return the extension Only
  * @param  pFile: pointer to the file name.
  * @param  pExt:  pointer to the file extension
  * @retval None
  */
void k_GetExtOnly(char * pFile, char * pExt) 
{
  int Len;
  int i;
  int j;
  
  /* Search beginning of extension */
  Len = strlen(pFile);
  for (i = Len; i > 0; i--) {
    if (*(pFile + i) == '.') {
      break;
    }
  }
  
  /* Copy extension */
  j = 0;
  while (*(pFile + ++i) != '\0') {
    *(pExt + j) = *(pFile + i);
    j++;
  }
  *(pExt + j) = '\0';          /* Set end of string */
}
/**
  * @brief  This function is responsible to pass information about the requested file
  * @param  pInfo: Pointer to structure which contains all details of the requested file.
  * @retval None
  */
int k_GetData(CHOOSEFILE_INFO * pInfo)
{
  char                c;
  int                 i;
  char               tmp[CHOOSEFILE_MAXLEN];  
  FRESULT res = FR_INT_ERR;
  char *fn;
  FILINFO fno;

  switch (pInfo->Cmd) 
  {
  case CHOOSEFILE_FINDFIRST:
    f_closedir(&dir); 
    
    /* reformat path */
    memset(tmp, 0, CHOOSEFILE_MAXLEN);
    strcpy(tmp, pInfo->pRoot);
    
    for(i= CHOOSEFILE_MAXLEN; i > 0 ; i--)
    {
      if(tmp[i] == '/')
      {
        tmp[i] = 0 ;
        break;
      }
    }
    
    res = f_opendir(&dir, tmp);
    
    if (res == FR_OK)
    {
      
      res = f_readdir(&dir, &fno);
    }
    break;
    
  case CHOOSEFILE_FINDNEXT:
    res = f_readdir(&dir, &fno);
    break;
  }
  
  if (res == FR_OK)
  {
    fn = fno.fname;

    while (((fno.fattrib & AM_DIR) == 0) && (res == FR_OK))
    {
      
      if((strstr(pInfo->pMask, ".img")))
      {
        if((strstr(fn, ".bmp")) || (strstr(fn, ".jpg")) || (strstr(fn, ".BMP")) || (strstr(fn, ".JPG")))
        {
          break;
        }
        else
        {
          res = f_readdir(&dir, &fno);
          
          if (res != FR_OK || fno.fname[0] == 0)
          {
            f_closedir(&dir); 
            return 1;
          }
          else
          {
            fn = fno.fname;
          }
        }
        
      }
      else if((strstr(pInfo->pMask, ".audio")))
      {
        if((strstr(fn, ".wav")) || (strstr(fn, ".WAV")))
        {
          break;
        }
        else
        {
          res = f_readdir(&dir, &fno);
          
          if (res != FR_OK || fno.fname[0] == 0)
          {
            f_closedir(&dir); 
            return 1;
          }
          else
          {
            fn = fno.fname;
          }
        }
        
      }
      
      else if((strstr(pInfo->pMask, ".video")))
      {
        if((strstr(fn, ".emf")) || (strstr(fn, ".EMF")))
        {
          break;
        }
        else
        {
          res = f_readdir(&dir, &fno);
          
          if (res != FR_OK || fno.fname[0] == 0)
          {
            f_closedir(&dir); 
            return 1;
          }
          else
          {
            fn = fno.fname;
          }
        }
        
      }      
      else if(strstr(fn, pInfo->pMask) == NULL)
      {
        
        res = f_readdir(&dir, &fno);
        
        if (res != FR_OK || fno.fname[0] == 0)
        {
          f_closedir(&dir); 
          return 1;
        }
        else
        {
          fn = fno.fname;
        }
      }
      else
      {
        break;
      }
    }   
    
    if(fn[0] == 0)
    {
      f_closedir(&dir); 
      return 1;
    } 
    
    pInfo->Flags = ((fno.fattrib & AM_DIR) == AM_DIR) ? CHOOSEFILE_FLAG_DIRECTORY : 0;
    
    for (i = 0; i < GUI_COUNTOF(_aAttrib); i++)
    {
      if (fno.fattrib & _aAttrib[i].Mask)
      {
        c = _aAttrib[i].c;
      }
      else
      {
        c = '-';
      }
      acAttrib[i] = c;
    }
    if((fno.fattrib & AM_DIR) == AM_DIR)
    {
      acExt[0] = 0;
    }
    else
    {
      GetExt(fn, acExt);
    }
    pInfo->pAttrib = acAttrib;
    pInfo->pName = fn;
    pInfo->pExt = acExt;
    pInfo->SizeL = fno.fsize;
    pInfo->SizeH = 0;
    
  }
  return res;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
