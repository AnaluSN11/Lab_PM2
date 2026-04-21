/*
 * ADC6.h
 *
 * Created: 20/04/2026 21:45:56
 *  Author: AnaLucia
 */ 


#ifndef ADC6_H_
#define ADC6_H_

#include <avr/io.h>

void initADC(void);
uint16_t readADC(uint8_t canal);


#endif /* ADC6_H_ */