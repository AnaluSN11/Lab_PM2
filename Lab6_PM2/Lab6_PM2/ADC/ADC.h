/*
 * ADC.h
 *
 * Created: 21/04/2026 11:56:40
 *  Author: AnaLucia
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

void initADC(void);
uint16_t readADC(uint8_t canal);


#endif /* ADC_H_ */