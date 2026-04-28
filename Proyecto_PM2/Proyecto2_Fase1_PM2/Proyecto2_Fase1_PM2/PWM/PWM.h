/*
 * PWM.h
 *
 * Created: 28/04/2026 10:29:17
 *  Author: AnaLucia
 */ 


#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define invertir 1
#define no_invertir 0

#define fastPWM 1
#define phasePWM 0

void initTimer1(uint8_t modo, uint16_t prescaler);

void initPWM1A(uint8_t invertido);
void updateDutyCycle1A(uint16_t duty);

void initPWM1B(uint8_t invertido);
void updateDutyCycle1B(uint16_t duty);

void initTimer2(uint8_t modo, uint16_t prescaler);

void initPWM2A(uint8_t invertido);
void updateDutyCycle2A(uint8_t duty);

void initPWM2B(uint8_t invertido);
void updateDutyCycle2B(uint8_t duty);

#endif /* PWM_H_ */