/**
  ******************************************************************************
  * @file         fft.c
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : fft.c
  * Datum    : 11.03.2014
  * Version  : 1.1
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "fft.h"


//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
static float32_t FFT_CMPLX_DATA[FFT_CMPLX_LENGTH];
static float32_t FFT_MAG_DATA[FFT_LENGTH];

arm_cfft_radix4_instance_f32 S_CFFT;
arm_rfft_instance_f32 S;



//--------------------------------------------------------------
// init vom FFT-Modul
// ret_wert : 0 = Fehler beim init
//            1 = init ok
//--------------------------------------------------------------
uint32_t fft_init(void)
{
  uint32_t ret_wert=0;
  arm_status status;
  uint32_t doBitReverse = 1;
  uint32_t ifftFlag = 0;

  status = ARM_MATH_SUCCESS;
  // FFT init
  status = arm_rfft_init_f32(&S, &S_CFFT, FFT_LENGTH, ifftFlag, doBitReverse);
  if(status!=ARM_MATH_SUCCESS) return(0);

  ret_wert=1;

  return(ret_wert);
}


//--------------------------------------------------------------
// berechnet die FFT
// rechnet die Daten dann noch in Pixelwerte um
//--------------------------------------------------------------
void fft_calc(void)
{
  float32_t maxValue;
  uint32_t n;

  // FFT berechnen
  arm_rfft_f32(&S, FFT_DATA_IN, FFT_CMPLX_DATA);
  arm_cmplx_mag_f32(FFT_CMPLX_DATA, FFT_MAG_DATA, FFT_LENGTH);

  // Maximum manuell suchen
  // die ersten beiden Eintr�ge �berspringen
  maxValue=0.1;
  for(n=2;n<FFT_VISIBLE_LENGTH;n++) {
    if(FFT_MAG_DATA[n]>maxValue) maxValue=FFT_MAG_DATA[n];
  }

  // alle Werte auf das Maximum normieren
  // die ersten beiden Eintr�ge auf 0 setzen
  FFT_UINT_DATA[0]=0;
  FFT_UINT_DATA[1]=0;
  for(n=2;n<FFT_VISIBLE_LENGTH;n++) {
    FFT_UINT_DATA[n]=(uint16_t)(FFT_UINT_MAXWERT*FFT_MAG_DATA[n]/maxValue);
  }
}



