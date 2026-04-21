/*
 * PWM.h
 *
 * Created: 15/04/2026 21:44:09
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

#define PWM_manual_DDR	DDRB
#define PWM_manual_PORT	PORTB
#define PWM_manual_PIN	DDB0

volatile uint8_t pwm_duty;
volatile uint8_t pwm_counter;

void initPWM1(uint8_t invertido, uint8_t modo, uint16_t prescaler);
void updateDutyCycle1(uint16_t ciclo1);


void initPWM2(uint8_t invertido, uint8_t modo, uint16_t prescaler);
void updateDutyCycle2(uint8_t ciclo2);

void initPWM_Manual(void);
void updateDutyCycleManual(uint8_t duty);

#endif /* PWM_H_ */