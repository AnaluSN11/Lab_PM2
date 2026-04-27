/*
 * ADC.h
 *
 * Created: 26/04/2026 22:25:24
 *  Author: AnaLucia
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

void initADC(void);
uint16_t readADC(uint8_t canal);


#endif /* ADC_H_ */