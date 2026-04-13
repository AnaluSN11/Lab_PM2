/*
 * PWM1.h
 *
 * Created: 12/04/2026 16:53:17
 *  Author: AnaLucia
 */ 


#ifndef PWM1_H_
#define PWM1_H_

#include <avr/io.h>

#define invertir 1
#define no_invertir 0

#define fastPWM1 1
#define phasePWM 0


void initPWM1(uint8_t invertido, uint8_t modo, uint16_t prescaler);
void updateDutyCycle1(uint16_t ciclo);

#endif /* PWM1_H_ */