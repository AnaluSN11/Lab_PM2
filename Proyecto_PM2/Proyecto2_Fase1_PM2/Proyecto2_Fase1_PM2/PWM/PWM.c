/*
 * PWM.c
 *
 * Created: 28/04/2026 10:29:07
 *  Author: AnaLucia
 */ 

#include "PWM.h"

volatile uint8_t pwm_duty = 0;
volatile uint8_t pwm_counter = 0;

void initTimer1(uint8_t modo, uint16_t prescaler)
{
	// Borrar registros para limpiar
	TCCR1A	= 0;
	TCCR1B	= 0;
	
	// Selección de modo
	if (modo) {
		// Fast PWM, TOP = ICR1 (modo 14)
		TCCR1A	|= (1<<WGM11);
		TCCR1B	|= (1<<WGM13) | (1<<WGM12); 
		} else {
		// Phase Correct PWM, TOP = ICR1 (modo 10)
		TCCR1A	|= (1<<WGM11);
		TCCR1B	|= (1<<WGM13); 
	}
	
	// Prescaler
	switch(prescaler)
	{
		case 1:		TCCR1B	|= (1<<CS10); break;
		case 8:		TCCR1B	|= (1<<CS11); break;
		case 64:	TCCR1B	|= (1<<CS11) | (1<<CS10); break;
		case 256:	TCCR1B	|= (1<<CS12); break;
		case 1024:	TCCR1B	|= (1<<CS12) | (1<<CS10); break;
		default:	TCCR1B	|= (1<<CS10); break;
	}
	
	// TOP -> 20ms(50Hz)
	ICR1	= 4999;
}

void initPWM1A(uint8_t invertido)
{
	// PB1 como salida (OC1A)
	DDRB	|= (1<<DDB1);
	
	// Invertido
	if (invertido) {
		TCCR1A	|= (1<<COM1A1) | (1<<COM1A0); // Invertido - PB1
	} else {
		TCCR1A	|= (1<<COM1A1); // No invertido - PB1
	}
	
	// Puslo inicial 0° (1ms)
	OCR1A	= 250;
}

void updateDutyCycle1A(uint16_t duty)
{
	OCR1A	= duty;
}

void initPWM1B(uint8_t invertido)
{
	// PB2 como salida (OC1B)
	DDRB	|= (1<<DDB2);
	
	// Invertido
	if (invertido) {
		TCCR1A	|= (1<<COM1B1) | (1<<COM1B0); // Invertido - PB2
		} else {
		TCCR1A	|= (1<<COM1B1); // No invertido - PB2
	}
	
	// Puslo inicial 0° (1ms)
	OCR1B	= 250;
}

void updateDutyCycle1B(uint16_t duty)
{
	OCR1B	= duty;
}

/*============================================================================================*/
void initTimer2(uint8_t modo, uint16_t prescaler)
{
	// Borrar registros para limpiar
	TCCR2A	= 0;
	TCCR2B	= 0;
	
	// Selección de modo
	if (modo) {
		// Fast PWM (TOP = 0xFF)
		TCCR2A	|= (1<<WGM21) | (1<<WGM20);
	} else {
		// Phase Correct PWM (TOP = 0xFF)
		TCCR2A	|= (1<<WGM20);
	}
	
	// Prescaler
	switch(prescaler)
	{
		case 1:		TCCR2B	|= (1<<CS20); break;
		case 8:		TCCR2B	|= (1<<CS21); break;
		case 32:	TCCR2B	|= (1<<CS21) | (1<<CS20); break;
		case 64:	TCCR2B	|= (1<<CS22); break;
		case 128:	TCCR2B	|= (1<<CS22) | (1<<CS20); break;
		case 256:	TCCR2B	|= (1<<CS22) | (1<<CS21); break;
		case 1024:	TCCR2B	|= (1<<CS22) | (1<<CS21) | (1<<CS20); break;
		default:	TCCR2B	|= (1<<CS22) | (1<<CS21) | (1<<CS20); break;
	}
	
}

void initPWM2A(uint8_t invertido)
{
	// PB3 como salida (OC2A)
	DDRB	|= (1<<DDB3);
	
	// Invertido
	if (invertido) {
		TCCR2A	|= (1<<COM2A1) | (1<<COM2A0); // Invertido - PB3
	} else {
		TCCR2A	|= (1<<COM2A1); // No invertido - PB3
	}
	
	// Puslo inicial 0° (1ms)
	OCR2A	= 16;
}

void updateDutyCycle2A(uint8_t duty)
{
	OCR2A	= duty;
}

void initPWM2B(uint8_t invertido)
{
	// PD3 como salida (OC2B)
	DDRD	|= (1<<DDD3);
	
	// Invertido
	if (invertido) {
		TCCR2A	|= (1<<COM2B1) | (1<<COM2B0); // Invertido - PD3
		} else {
		TCCR2A	|= (1<<COM2B1); // No invertido - PD3
	}
	
	// Puslo inicial 0° (1ms)
	OCR2B	= 16;
}

void updateDutyCycle2B(uint8_t duty)
{
	OCR2B	= duty;
}
