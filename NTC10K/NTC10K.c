/*
 * NTC10K.c
 *
 *  Created on: Jun 25, 2021
 *      Author: jcaf
 */
#include "../system.h"
#include "NTC10K.h"
/////////////////////////////////////////////////////////////////////////////////////
//VISHAY NTCLE413E2103F102L 1%
//- 40 C to + 105 C
/////////////////////////////////////////////////////////////////////////////////////


#define RESISTOR_UP 9.960E3//fluke 175
#define NTC_NOMINAL 10.0E3//+-1%

//Steinhart equation
#define ST_A 0.00335401643468053
#define ST_B 0.000300130825115663
#define ST_C 5.08516494379094E-06
#define ST_D 2.18765049258341E-07

float ntc10k_st(uint16_t ADC_CURRENT_VAL, uint16_t ADC_TOP_VAL)
{
  float Rm=RESISTOR_UP/(( ((float)ADC_TOP_VAL)/ADC_CURRENT_VAL) -1);
  //float Rm = RESISTOR_UP/((29890/ADC_CURRENT_VAL) -1);
  float k=log(Rm/NTC_NOMINAL);
  float temperature =(1.0/( ST_A + (ST_B*k) + (ST_C*k*k) + (ST_D*k*k*k)) ) -273.15 ;
  return temperature;
}
