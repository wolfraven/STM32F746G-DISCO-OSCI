/**
  ******************************************************************************
  * @file         adc.c
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *               Workaround for ADC trigger problem
  *               Adaption to 100MHz timer clock (instead of 84MHz).
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : adc.c
  * Datum    : 05.01.2014
  * Version  : 1.1
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "menu.h"
#include "adc.h"


//--------------------------------------------------------------
// Definition der benutzten ADC Pins (max=16)
// Reihenfolge wie bei ADC1d_NAME_t
//--------------------------------------------------------------
ADC1d_t ADC1d[] = {
  //NAME  ,PORT , PIN      , CLOCK              , Kanal
  {ADC_PA0, GPIOA,LL_GPIO_PIN_0 ,LL_AHB1_GRP1_PERIPH_GPIOA,LL_ADC_CHANNEL_0},   // ADC an PA0  = ADC123_IN0
  {ADC_PF10,GPIOF,LL_GPIO_PIN_10,LL_AHB1_GRP1_PERIPH_GPIOF,LL_ADC_CHANNEL_8},   // ADC an PF10 = ADC3_IN8
};

//--------------------------------------------------------------
// Globale ADC-Puffer
// Jeder Buffer ist so gross wie ein kompletter Oszi-Screen
// (400 Pixel x 3 Kanäle x 16bit)
//--------------------------------------------------------------
#ifndef WW_USE_AUDIO_BUFFER_FOR_ADC
volatile uint16_t ADC_DMA_Buffer_A[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (A) Roh-Daten per DMA
volatile uint16_t ADC_DMA_Buffer_B[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (B) Roh-Daten per DMA
volatile uint16_t ADC_DMA_Buffer_C[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (C) sortierte Daten
#endif

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_ADC_InitIO(void);
void P_ADC_InitDMA_DoubleBuffer(void);
void P_ADC_InitNVIC(void);
void P_ADC_InitADC(void);
void P_ADC_Start(void);
void P_ADC_InitTimer4(void);
void P_ADC_Clear(void);
void ADC_searchTrigger_A1(void);
void ADC_searchTrigger_B1(void);
void ADC_searchTrigger_A2(void);
void ADC_searchTrigger_B2(void);



//--------------------------------------------------------------
// init vom ADC1 und ADC2 im DMA Mode
// und starten der zyklischen Wandlung
//--------------------------------------------------------------
void ADC_Init_ALL(void)
{
  // init aller Variablen
  ADC_UB.status=ADC_VORLAUF;
  ADC_UB.trigger_pos=0;
  ADC_UB.trigger_quarter=0;
  ADC_UB.dma_status=0;
  ADC_UB.recordCallback = 0;

  P_ADC_Clear();
  P_ADC_InitIO();
  P_ADC_InitDMA_DoubleBuffer();
  P_ADC_InitNVIC();
  P_ADC_InitADC();
  /* PROBLEM:
     WW: ADC was not triggered from TIM4-TRGO update event!!!
   */
  /* RM0385 Rev 8 Page 793/1724
     Note: The clock of the slave peripherals (timer, ADC, ...) receiving the TRGO or the TRGO2
     signals must be enabled prior to receive events from the master timer, and the clock
     frequency (prescaler) must not be changed on-the-fly while triggers are received from
     the master timer.

     WW: So moved the timer initialization behind the ADC initialization => but that did NOT solve the problem.
   */
  /* ES0290 Rev 6 Page 7/28
     2.2.1 Missed ADC triggers from TIM1/TIM8, TIM2/TIM5/TIM4/TIM6/TRGO or TGRO2 event
     Description
        The ADC external triggers for regular and injected channels by the TIM1, TIM8, TIM2, TIM5,
        TIM4 and TIM6 TRGO or TRGO2 events are missed at the following conditions:
        - Prescaler enabled on the PCLK2 clock.
        - TIMxCLK = 2xADCCLK and the master mode selection (MMS or MMS2 bits in the
          TIMx_CR2 timer register) as reset, update, or compare pulse configuration.
        - TIMxCLK = 4xADCCLK.
     Workarounds
        - For TIM1 and TIM8 TRGO or TRGO 2 events: select the trigger detection on both the
          rising and falling edges. The EXTEN[1:0] or JEXTEN[1:0] bits must be set to 0x11 in
          the ADC_CR2 register.
        - For TIM2/TIM4/TIM5/TIM6/ TRGO or TGRO2 events: enable the DAC peripheral clock
          in the RCC_APB1ENR register.

     WW: So just added the DAC peripheral clock enable in the ADC initialization
         => that solved the problem!!!
   */
  P_ADC_InitTimer4();
  P_ADC_Start();
}


//--------------------------------------------------------------
// �ndern der Frequenz vom Timer2
// (Timebase der Abtastrate)
//
// n : [0...16]
//--------------------------------------------------------------
void ADC_change_Frq(uint32_t n)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  uint32_t prescaler, period;

  // Timer anhalten
  LL_TIM_DisableCounter(TIM4);

  // On the STM32F746 we use TIM4 (APB1 timer) and therfore have 100MHz (instead of 84MHz on the 32F4).
  // So the timer values need to be adapted for the higher frequency.
  switch(n) {
    case 0 : // 5s=>5Hz=5s
      prescaler=6399;period=3124;    // 5375 -> 6399
    break;
    case 1 : // 2s=>12,5Hz=80ms
      prescaler=3199;period=2499;    // 2687 -> 3199
    break;
    case 2 : // 1s=>25Hz=40ms
      prescaler=1599;period=2499;    // 1343 -> 1599
    break;
    case 3 : // 500ms=>50Hz=20ms
      prescaler=799;period=2499;     // 671 -> 799
    break;
    case 4 : // 200ms=>125Hz=8ms
      prescaler=399;period=1999;     // 335 -> 399
    break;
    case 5 : // 100ms=>250Hz=4ms
      prescaler=199;period=1999;     // 167 -> 199
    break;
    case 6 : // 50ms=>500Hz=2ms
      prescaler=99;period=1999;      // 83 -> 99
    break;
    case 7 : // 20ms=>1,25kHz=800us
      prescaler=49;period=1599;      // 41 -> 49
    break;
    case 8 : // 10ms=>2,5kHz400us
      prescaler=24;period=1599;      // 20 -> 24
    break;
    case 9 : // 5ms=>5kHz=200us
      prescaler=24;period=799;
    break;
    case 10 : // 2ms=>12,5kHz=80us
      prescaler=24;period=319;
    break;
    case 11 : // 1ms=>25kHz=40us
      prescaler=24;period=159;
    break;
    case 12 : // 500us=>50kHz=20us
      prescaler=24;period=79;
    break;
    case 13 : // 200us=>125kHz=8us
      prescaler=24;period=31;
    break;
    case 14 : // 100us=>250kHz=4us
      prescaler=24;period=15;
    break;
    case 15 : // 50us=>500kHz=2us
      prescaler=24;period=7;
    break;
    case 16 : // 25us=>1MHz=1us
      prescaler=24;period=3;
    break;
    default :
      prescaler=OSZI_TIM2_PRESCALE;
      period=OSZI_TIM2_PERIODE;
  }

  TIM_InitStruct.Prescaler = prescaler;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = period;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM4, &TIM_InitStruct);

  // Timer wieder starten, falls notwendig
  if(ADC_UB.status!=ADC_READY) {
    // Timer2 enable
    LL_TIM_EnableARRPreload(TIM4);
    LL_TIM_EnableCounter(TIM4);
  }
}



//--------------------------------------------------------------
// �ndern vom Mode des DMA
// n != 1 => Double-Buffer-Mode
// n = 1  => Single-Buffer-Mode
//--------------------------------------------------------------
void ADC_change_Mode(uint32_t n)
{
  DMA_InitTypeDef       DMA_InitStructure;

  // Merker setzen
  ADC_UB.dma_status=1;

  // Timer analten
  LL_TIM_DisableCounter(TIM4);

  // DMA-Disable
  LL_DMA_DisableStream(ADC_DMA, ADC_DMA_STREAM);
  // warten bis DMA-Stream disable
  while(LL_DMA_IsEnabledStream(ADC_DMA, ADC_DMA_STREAM)==ENABLE);
  LL_DMA_DeInit(ADC_DMA, ADC_DMA_STREAM);

  // DMA-Config
  LL_DMA_SetChannelSelection(ADC_DMA, ADC_DMA_STREAM, ADC_DMA_CHANNEL);
  LL_DMA_SetPeriphAddress(ADC_DMA, ADC_DMA_STREAM, (uint32_t)LL_ADC_DMA_GetRegAddr(ADC1,LL_ADC_DMA_REG_REGULAR_DATA));
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
  LL_DMA_SetMemoryAddress(ADC_DMA, ADC_DMA_STREAM, (uint32_t)ADC_DMA_Buffer_A_1);
#else
  LL_DMA_SetMemoryAddress(ADC_DMA, ADC_DMA_STREAM, (uint32_t)&ADC_DMA_Buffer_A);
#endif
  LL_DMA_SetDataTransferDirection(ADC_DMA, ADC_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetDataLength(ADC_DMA, ADC_DMA_STREAM, ADC1d_ANZ*ADC_ARRAY_LEN);
  LL_DMA_SetPeriphIncMode(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PDATAALIGN_HALFWORD);
  LL_DMA_SetMemorySize(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MDATAALIGN_HALFWORD);
  LL_DMA_SetMode(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MODE_CIRCULAR);
  LL_DMA_SetStreamPriorityLevel(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PRIORITY_HIGH);
  //LL_DMA_DisableFifoMode(ADC1_DMA, ADC1_DMA_STREAM);
  LL_DMA_SetFIFOThreshold(ADC_DMA, ADC_DMA_STREAM, LL_DMA_FIFOTHRESHOLD_1_2);
  LL_DMA_SetMemoryBurstxfer(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MBURST_SINGLE);
  LL_DMA_SetPeriphBurstxfer(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PBURST_SINGLE);

  if(n!=1)
  {
    // Double-Buffer-Mode
    LL_DMA_EnableDoubleBufferMode(ADC_DMA, ADC_DMA_STREAM);
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
    LL_DMA_SetMemory1Address(ADC_DMA, ADC_DMA_STREAM, (uint32_t)ADC_DMA_Buffer_B_1);
#else
    LL_DMA_SetMemory1Address(ADC_DMA, ADC_DMA_STREAM, (uint32_t)&ADC_DMA_Buffer_B);
#endif
  }
  else
  {
    // Normal Mode
      LL_DMA_DisableDoubleBufferMode(ADC_DMA, ADC_DMA_STREAM);
  }

  // Flags l�schen
  LL_DMA_ClearFlag_TC0(ADC_DMA);
  LL_DMA_ClearFlag_HT0(ADC_DMA);

  // DMA-enable
  LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);

  // warten bis DMA-Stream enable
  while(LL_DMA_IsEnabledStream(ADC_DMA, ADC_DMA_STREAM)==DISABLE);

  // NVIC neu initialisieren
  P_ADC_InitNVIC();

  // Timer wieder starten, falls notwendig
  if((ADC_UB.status!=ADC_READY) || (n==1)) {
    // Timer2 enable
    LL_TIM_EnableARRPreload(TIM4);
    LL_TIM_EnableCounter(TIM4);
  }

  // Merker zur�cksetzen
  ADC_UB.dma_status=0;

}


//--------------------------------------------------------------
// interne Funktion
// l�schen aller ADC-Arrays
//--------------------------------------------------------------
void P_ADC_Clear(void)
{
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
   memset(ADC_DMA_Buffer_A_1, 0, 6*ADC_HALF_BUFFER_BYTE_SIZE);
#else
  uint32_t n;

  for(n=0;n<ADC_ARRAY_LEN;n++)
  {
    ADC_DMA_Buffer_A[n*ADC1d_ANZ] = 0;
    ADC_DMA_Buffer_A[(n*ADC1d_ANZ)+1] = 0;
    ADC_DMA_Buffer_A[(n*ADC1d_ANZ)+2] = 0;

    ADC_DMA_Buffer_B[n*ADC1d_ANZ] = 0;
    ADC_DMA_Buffer_B[(n*ADC1d_ANZ)+1] = 0;
    ADC_DMA_Buffer_B[(n*ADC1d_ANZ)+2] = 0;

    ADC_DMA_Buffer_C[n*ADC1d_ANZ] = 0;
    ADC_DMA_Buffer_C[(n*ADC1d_ANZ)+1] = 0;
    ADC_DMA_Buffer_C[(n*ADC1d_ANZ)+2] = 0;
  }
#endif
}


//--------------------------------------------------------------
// interne Funktion
// Init aller IO-Pins
//--------------------------------------------------------------
void P_ADC_InitIO(void) {
  LL_GPIO_InitTypeDef GPIO_InitStructure = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA|LL_AHB1_GRP1_PERIPH_GPIOF);

  // Config des Pins als Analog-Eingang
  GPIO_InitStructure.Pin = LL_GPIO_PIN_0;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO ;
  LL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = LL_GPIO_PIN_10;
  LL_GPIO_Init(GPIOF, &GPIO_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// Init vom DMA (im Double-Buffer-Mode)
//--------------------------------------------------------------
void P_ADC_InitDMA_DoubleBuffer(void)
{
  //DMA_InitTypeDef       DMA_InitStructure;

  // Clock Enable
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

  // DMA-Disable
  LL_DMA_DisableStream(ADC_DMA, ADC_DMA_STREAM);

  // warten bis DMA-Stream disable
  while(LL_DMA_IsEnabledStream(ADC_DMA, ADC_DMA_STREAM)==ENABLE);
  LL_DMA_DeInit(ADC_DMA, ADC_DMA_STREAM);

  // DMA-Config
  LL_DMA_SetChannelSelection(ADC_DMA, ADC_DMA_STREAM, ADC_DMA_CHANNEL);
  LL_DMA_SetPeriphAddress(ADC_DMA, ADC_DMA_STREAM, (uint32_t)LL_ADC_DMA_GetRegAddr(ADC1,LL_ADC_DMA_REG_REGULAR_DATA_MULTI));
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
   LL_DMA_SetMemoryAddress(ADC_DMA, ADC_DMA_STREAM, (uint32_t)ADC_DMA_Buffer_A_1);
#else
  LL_DMA_SetMemoryAddress(ADC_DMA, ADC_DMA_STREAM, (uint32_t)&ADC_DMA_Buffer_A);
#endif
  LL_DMA_SetDataTransferDirection(ADC_DMA, ADC_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetDataLength(ADC_DMA, ADC_DMA_STREAM, ADC1d_ANZ*ADC_ARRAY_LEN);
  LL_DMA_SetPeriphIncMode(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PDATAALIGN_HALFWORD);
  LL_DMA_SetMemorySize(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MDATAALIGN_HALFWORD);
  LL_DMA_SetMode(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MODE_CIRCULAR);
  LL_DMA_SetStreamPriorityLevel(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PRIORITY_HIGH);
  LL_DMA_DisableFifoMode(ADC_DMA, ADC_DMA_STREAM);
  //LL_DMA_SetFIFOThreshold(ADC_DMA, ADC_DMA_STREAM, LL_DMA_FIFOTHRESHOLD_1_2);
  //LL_DMA_SetMemoryBurstxfer(ADC_DMA, ADC_DMA_STREAM, LL_DMA_MBURST_SINGLE);
  //LL_DMA_SetPeriphBurstxfer(ADC_DMA, ADC_DMA_STREAM, LL_DMA_PBURST_SINGLE);

  // Double-Buffer-Mode
  LL_DMA_EnableDoubleBufferMode(ADC_DMA, ADC_DMA_STREAM);
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
   LL_DMA_SetMemory1Address(ADC_DMA, ADC_DMA_STREAM, (uint32_t)ADC_DMA_Buffer_B_1);
#else
   LL_DMA_SetMemory1Address(ADC_DMA, ADC_DMA_STREAM, (uint32_t)&ADC_DMA_Buffer_B);
#endif

  // Flags l�schen
  LL_DMA_ClearFlag_TC0(ADC_DMA);
  LL_DMA_ClearFlag_HT0(ADC_DMA);

  // DMA-enable
  LL_DMA_EnableStream(ADC_DMA, LL_DMA_STREAM_0);

  // warten bis DMA-Stream enable
  while(LL_DMA_IsEnabledStream(ADC_DMA, ADC_DMA_STREAM)==DISABLE);
}


//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P_ADC_InitNVIC(void)
{
  //---------------------------------------------
  // init vom DMA Interrupt
  // f�r TransferComplete Interrupt
  // und HalfTransferComplete Interrupt
  // DMA2, Stream0, Channel0
  //---------------------------------------------

  LL_DMA_EnableIT_TC(ADC_DMA, ADC_DMA_STREAM); // Enable Transfer complete interrupt.
  LL_DMA_EnableIT_HT(ADC_DMA, ADC_DMA_STREAM); // Enable Half transfer interrupt.

  // NVIC config
  NVIC_SetPriority(DMA2_Stream0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),7, 0));
  NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}


//--------------------------------------------------------------
// interne Funktion
// Init von ADC Nr.1+2 (im Dual regular simultaneous mode)
//
// @ 12bit + ADC_TwoSamplingDelay_5Cycles + 21MHz ADC-Clock :
//
// ADC_SampleTime_3Cycles  => Sample_Time =  3+12+5=20 => 952ns
// ADC_SampleTime_15Cycles => Sample_Time = 15+12+5=32 => 1.52us
// ADC_SampleTime_28Cycles => Sample_Time = 28+12+5=45 => 2.14us
//--------------------------------------------------------------
void P_ADC_InitADC(void)
{
  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  /* PROBLEM:
     WW: ADC was not triggered from TIM4-TRGO update event!!!
   */
  /* ES0290 Rev 6 Page 7/28
     2.2.1 Missed ADC triggers from TIM1/TIM8, TIM2/TIM5/TIM4/TIM6/TRGO or TGRO2 event
     Description
        The ADC external triggers for regular and injected channels by the TIM1, TIM8, TIM2, TIM5,
        TIM4 and TIM6 TRGO or TRGO2 events are missed at the following conditions:
        - Prescaler enabled on the PCLK2 clock.
        - TIMxCLK = 2xADCCLK and the master mode selection (MMS or MMS2 bits in the
          TIMx_CR2 timer register) as reset, update, or compare pulse configuration.
        - TIMxCLK = 4xADCCLK.
     Workarounds
        - For TIM1 and TIM8 TRGO or TRGO 2 events: select the trigger detection on both the
          rising and falling edges. The EXTEN[1:0] or JEXTEN[1:0] bits must be set to 0x11 in
          the ADC_CR2 register.
        - For TIM2/TIM4/TIM5/TIM6/ TRGO or TGRO2 events: enable the DAC peripheral clock
          in the RCC_APB1ENR register.

     WW: So just added the DAC peripheral clock enable in the ADC initialization
         => that solved the problem!!!
   */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);

  // Clock Enable
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1|LL_APB2_GRP1_PERIPH_ADC2|LL_APB2_GRP1_PERIPH_ADC3);

  //-------------------------------------
  // ADC-Config (TripleMode)
  //-------------------------------------

  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_TRIPLE_REG_SIMULT;
  ADC_CommonInitStruct.MultiTwoSamplingDelay = LL_ADC_MULTI_TWOSMP_DELAY_5CYCLES;
  ADC_CommonInitStruct.MultiDMATransfer = LL_ADC_MULTI_REG_DMA_UNLMT_1; /*!< ADC multimode group regular conversions are transferred by DMA,
                                                                             one DMA channel for all ADC instances (DMA of ADC master),
                                                                             in unlimited mode: DMA transfer requests are unlimited,
                                                                             whatever number of DMA data transferred (number of ADC conversions)
                                                                             is reached. This ADC mode is intended to be used with DMA mode non-circular.
                                                                             Setting of DMA mode 1: 2 or 3 (dual or triple mode) half-words one by one,
                                                                             ADC1 then ADC2 then ADC3. */
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);

  //-------------------------------------
  // ADC1 (Master)
  //-------------------------------------
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
#ifdef WW_ADC_SELF_TRIGGER
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
#else
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM4_TRGO;
#endif
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE; // sequencer disable (equivalent to sequencer of 1 rank: ADC conversion on only 1 channel)
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_REG_SetFlagEndOfConversion(ADC1, LL_ADC_REG_FLAG_EOC_SEQUENCE_CONV);
  LL_ADC_DisableIT_EOCS(ADC1);
#ifndef WW_ADC_SELF_TRIGGER
  LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);
#endif

  /**Configure Regular Channel */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_VREFINT);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_VREFINT, LL_ADC_SAMPLINGTIME_3CYCLES);
#if 1
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT);
#else
  /* Enable the TSVREFE channel*/
  ADC->CCR |= ADC_CCR_TSVREFE;
#endif

  //-------------------------------------
  // ADC2 (Slave)
  //-------------------------------------
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC2, &ADC_InitStruct);
  //ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM4_TRGO;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE; // sequencer disable (equivalent to sequencer of 1 rank: ADC conversion on only 1 channel)
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  LL_ADC_REG_Init(ADC2, &ADC_REG_InitStruct);
  LL_ADC_REG_SetFlagEndOfConversion(ADC2, LL_ADC_REG_FLAG_EOC_UNITARY_CONV);
  LL_ADC_DisableIT_EOCS(ADC2);
  //LL_ADC_REG_StartConversionExtTrig(ADC2, LL_ADC_REG_TRIG_EXT_RISING);
  /**Configure Regular Channel */
  LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
  LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_3CYCLES);

  //-------------------------------------
  // ADC3 (Slave)
  //-------------------------------------
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC3, &ADC_InitStruct);
  //ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM4_TRGO;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE; // sequencer disable (equivalent to sequencer of 1 rank: ADC conversion on only 1 channel)
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  LL_ADC_REG_Init(ADC3, &ADC_REG_InitStruct);
  LL_ADC_REG_SetFlagEndOfConversion(ADC3, LL_ADC_REG_FLAG_EOC_UNITARY_CONV);
  LL_ADC_DisableIT_EOCS(ADC3);
  //LL_ADC_REG_StartConversionExtTrig(ADC3, LL_ADC_REG_TRIG_EXT_RISING);
  /**Configure Regular Channel */
  LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);
  LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_CHANNEL_8, LL_ADC_SAMPLINGTIME_3CYCLES);


#ifdef WW_ADC_SELF_TRIGGER // Just for testing - because trigger did not work
  /* ADC1 interrupt Init */
  LL_ADC_EnableIT_EOCS(ADC1);
  NVIC_SetPriority(ADC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(ADC_IRQn);
#endif
}

#ifdef WW_ADC_SELF_TRIGGER
volatile uint32_t ADC_IRQHits = 0;
void ADC_IRQHandler(void)
{
   LL_ADC_ClearFlag_EOCS(ADC1);
   ADC_IRQHits++;
   LL_ADC_REG_StartConversionSWStart(ADC1);
}
#endif

//--------------------------------------------------------------
// interne Funktion
// Enable und start vom ADC und DMA
//--------------------------------------------------------------
void P_ADC_Start(void)
{
  //LL_ADC_SetMultiDMATransfer(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_MULTI_REG_DMA_EACH_ADC); //ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
  //LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
  LL_ADC_Enable(ADC1);
  LL_ADC_Enable(ADC2);
  LL_ADC_Enable(ADC3);
  // Timer2 enable
  LL_TIM_EnableARRPreload(TIM4);
  LL_TIM_EnableCounter(TIM4);

#ifdef WW_ADC_SELF_TRIGGER
  LL_ADC_REG_StartConversionSWStart(ADC1);
#endif
}


//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void P_ADC_InitTimer4(void)
{
	  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  //---------------------------------------------
  // init Timer4
  // Clock-Source for ADC-Conversion
  //---------------------------------------------


  // Clock enable
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  // Timer4 init
  TIM_InitStruct.Prescaler = OSZI_TIM2_PRESCALE;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = OSZI_TIM2_PERIODE;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM4);
  LL_TIM_SetClockSource(TIM4, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_UPDATE);
  LL_TIM_DisableMasterSlaveMode(TIM4);

#ifdef WW_ADC_TIMERIRQ_SOFTWARE_TRIGGER
  LL_TIM_EnableIT_UPDATE(TIM4);
  NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM4_IRQn);
#endif
}

#ifdef WW_ADC_TIMERIRQ_SOFTWARE_TRIGGER
volatile uint32_t TIM4_IRQHits = 0;
void TIM4_IRQHandler(void)
{
	LL_TIM_ClearFlag_UPDATE(TIM4);
	TIM4_IRQHits++;
    LL_ADC_REG_StartConversionSWStart(ADC1);
}
#endif

//--------------------------------------------------------------
// Interrupt (ISR-Funktion)
// wird bei DMA Interrupt aufgerufen
//   (Bei HalfTransferComplete und TransferCompleteInterrupt)
//
//--------------------------------------------------------------
void DMA2_Stream0_IRQHandler(void)
{
#if (ENABLE_ADC_DMA_CACHE_MAINTENANCE == 1)
  uint32_t alignedAddr;
#endif

  if(LL_DMA_IsActiveFlag_HT0(ADC_DMA))
  {
    // HalfTransferInterruptComplete Interrupt von DMA2 ist aufgetreten
    LL_DMA_ClearFlag_HT0(ADC_DMA);
    uint32_t currentTargetMem = LL_DMA_GetCurrentTargetMem(ADC_DMA, ADC_DMA_STREAM);

#if (ENABLE_ADC_DMA_CACHE_MAINTENANCE == 1)
   /*
    the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
    adjust the address and the D-Cache size to invalidate accordingly.
    */
   alignedAddr = (uint32_t)ADC_DMA_Buffer_A_1 & ~0x1F;
   SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, 4*ADC_HALF_BUFFER_BYTE_SIZE + ((uint32_t)ADC_DMA_Buffer_A_1 - alignedAddr));
#endif

    if( ADC_UB.recordCallback != 0 )
    {
       if (currentTargetMem == LL_DMA_CURRENTTARGETMEM0)
       {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
          ADC_UB.recordCallback(ADC_DMA_Buffer_A_1, ADC_HALF_BUFFER_BYTE_SIZE);
#else
          ADC_UB.recordCallback((uint8_t*)&ADC_DMA_Buffer_A[0], ADC1d_ANZ*2*ADC_HALF_ARRAY_LEN);
#endif
       }
       else
       {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
          ADC_UB.recordCallback(ADC_DMA_Buffer_B_1, ADC_HALF_BUFFER_BYTE_SIZE);
#else
          ADC_UB.recordCallback((uint8_t*)&ADC_DMA_Buffer_B[0], ADC1d_ANZ*2*ADC_HALF_ARRAY_LEN);
#endif
       }
    }

    if(ADC_UB.dma_status==0)
    {
      if((ADC_UB.status==ADC_RUNNING) || (ADC_UB.status==ADC_PRE_TRIGGER))
      {
        if (currentTargetMem == LL_DMA_CURRENTTARGETMEM0)
        {
          ADC_searchTrigger_A1();
        }
        else
        {
          ADC_searchTrigger_B1();
        }
      }
    }
  }
  else if(LL_DMA_IsActiveFlag_TC0(ADC_DMA))
  {
    // TransferInterruptComplete Interrupt von DMA2 ist aufgetreten
    LL_DMA_ClearFlag_TC0(ADC_DMA);
    uint32_t currentTargetMem = LL_DMA_GetCurrentTargetMem(ADC_DMA, ADC_DMA_STREAM);

    if( ADC_UB.recordCallback != 0 )
    {
       if (currentTargetMem == LL_DMA_CURRENTTARGETMEM1) //WW: The buffer has changed, so use the other one!!!
       {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
          ADC_UB.recordCallback(ADC_DMA_Buffer_A_2, ADC_HALF_BUFFER_BYTE_SIZE);
#else
          ADC_UB.recordCallback((uint8_t*)&ADC_DMA_Buffer_A[ADC1d_ANZ*ADC_HALF_ARRAY_LEN], ADC1d_ANZ*2*ADC_HALF_ARRAY_LEN);
#endif
       }
       else
       {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
          ADC_UB.recordCallback(ADC_DMA_Buffer_B_2, ADC_HALF_BUFFER_BYTE_SIZE);
#else
          ADC_UB.recordCallback((uint8_t*)&ADC_DMA_Buffer_B[ADC1d_ANZ*ADC_HALF_ARRAY_LEN], ADC1d_ANZ*2*ADC_HALF_ARRAY_LEN);
#endif
       }
    }

    if(ADC_UB.dma_status==0)
    {
      LL_TIM_DisableCounter(TIM4);

      if(ADC_UB.status!=ADC_VORLAUF)
      {
        if(ADC_UB.status==ADC_TRIGGER_OK)
        {
          ADC_UB.status=ADC_READY;
        }
        else
        {
        	LL_TIM_EnableCounter(TIM4);
          if (currentTargetMem == LL_DMA_CURRENTTARGETMEM1)
          {
            ADC_searchTrigger_A2();
          }
          else
          {
            ADC_searchTrigger_B2();
          }
        }
      }
      else
      {
    	  LL_TIM_EnableCounter(TIM4);
        ADC_UB.status=ADC_RUNNING;
      }
    }

    UB_Led_Toggle(LED_GREENx);
  }
}

//--------------------------------------------------------------
// Interrupt (ISR-Funktion)
// wird bei DMA Interrupt aufgerufen
//   (Bei HalfTransferComplete und TransferCompleteInterrupt)
//
//--------------------------------------------------------------
uint8_t wwTriggerQuater = 0;
void WW_Trigger_Processing(bool lowerHalfBuffer, bool targetA)
{
  if(lowerHalfBuffer)
  {
    // HalfTransfer

    if(ADC_UB.dma_status==0)
    {
      if((ADC_UB.status==ADC_RUNNING) || (ADC_UB.status==ADC_PRE_TRIGGER))
      {
        if (targetA)
        {
          ADC_searchTrigger_A1();
          wwTriggerQuater = 1;
        }
        else
        {
          ADC_searchTrigger_B1();
          wwTriggerQuater = 3;
        }
      }
    }
  }
  else
  {
    // TransferComplete
    if(ADC_UB.dma_status==0)
    {
      if(ADC_UB.status!=ADC_VORLAUF)
      {
        if(ADC_UB.status==ADC_TRIGGER_OK)
        {
          ADC_UB.status=ADC_READY;
        }
        else
        {
          if (targetA)
          {
            ADC_searchTrigger_A2();
            wwTriggerQuater = 2;
          }
          else
          {
            ADC_searchTrigger_B2();
            wwTriggerQuater = 4;
          }
        }
      }
      else
      {
        ADC_UB.status=ADC_RUNNING;
      }
    }
    UB_Led_Toggle(LED_GREENx);
  }
}

//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-1
//--------------------------------------------------------------
void ADC_searchTrigger_A1(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if( (Menu.trigger.mode==TRIGGER_MODE_AUTO) || (Menu.trigger.source==CHANNEL_SEL_NONE) )
     return;

  if(Menu.trigger.source==CHANNEL_SEL_CH1)
  {
    ch=1;
    trigger=Menu.trigger.value_ch1;
  }
  else
  {
    ch=2;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==TRIGGER_EDGE_RISING)
  {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++)
    {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_A_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING)
      {
        if(wert<trigger)
        {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger)
        {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=1;
          break;
        }
      }
    }
  }
  else  if(Menu.trigger.edge==TRIGGER_EDGE_FALLING)
  {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++)
    {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_A_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING)
      {
        if(wert>trigger)
        {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else
      {
        if(wert<=trigger)
        {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=1;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-2
//--------------------------------------------------------------
void ADC_searchTrigger_A2(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if( (Menu.trigger.mode==TRIGGER_MODE_AUTO) || (Menu.trigger.source==CHANNEL_SEL_NONE) )
     return;

  if(Menu.trigger.source==CHANNEL_SEL_CH1) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==TRIGGER_EDGE_RISING)
  {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_A_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=2;
          break;
        }
      }
    }
  }
  else if(Menu.trigger.edge==TRIGGER_EDGE_FALLING)
  {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_A_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=2;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-3
//--------------------------------------------------------------
void ADC_searchTrigger_B1(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if( (Menu.trigger.mode==TRIGGER_MODE_AUTO) || (Menu.trigger.source==CHANNEL_SEL_NONE) )
     return;

  if(Menu.trigger.source==CHANNEL_SEL_CH1) {
    ch=1;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=2;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==TRIGGER_EDGE_RISING)
  {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_B_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=3;
          break;
        }
      }
    }
  }
  else  if(Menu.trigger.edge==TRIGGER_EDGE_FALLING)
  {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_B_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=3;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-4
//--------------------------------------------------------------
void ADC_searchTrigger_B2(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if( (Menu.trigger.mode==TRIGGER_MODE_AUTO) || (Menu.trigger.source==CHANNEL_SEL_NONE) )
     return;

  if(Menu.trigger.source==CHANNEL_SEL_CH1) {
    ch=1;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=2;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==TRIGGER_EDGE_RISING)
  {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_B_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=4;
          break;
        }
      }
    }
  }
  else if(Menu.trigger.edge==TRIGGER_EDGE_FALLING)
  {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
#ifdef WW_USE_AUDIO_BUFFER_FOR_ADC
      wert=*(((uint16_t*)ADC_DMA_Buffer_B_1)+((n*2)+ch));
#else
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
#endif
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=4;
          break;
        }
      }
    }
  }
}
