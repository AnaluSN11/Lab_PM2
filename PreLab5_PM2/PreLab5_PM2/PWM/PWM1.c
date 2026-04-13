/*
 * PWM1.c
 *
 * Created: 12/04/2026 16:53:05
 *  Author: AnaLucia
 */ 

#include "PWM1.h"

void initPWM1(uint8_t invertido, uint8_t modo, uint16_t prescaler)
{
	// Configurar salida OC1A (PB1)
	DDRB	|= (1<<DDB1);
	
	// Borrar registros para limpiar
	TCCR1A	= 0;
	TCCR1B	= 0;
	
	if (invertido) {
		TCCR1A	|= (1<<COM1A1) | (1<<COM1A0); // Invertido - PB1
	} else {
		TCCR1A	|= (1<<COM1A1); // No invertido - PB1
	}
	
	if (modo) {
		TCCR1A	|= (1<<WGM11);
		TCCR1B	|= (1<<WGM13) | (1<<WGM12); // Fast PWM, TOP: ICR1
	} else {
		TCCR1A	|= (1<<WGM11);
		TCCR1B	|= (1<<WGM13); // Phase PWM, TOP: ICR1
	}
	
	switch(prescaler)
	{
		case 1: 
		TCCR1B	|= (1<<CS10);
		break;
		case 8:
		TCCR1B	|= (1<<CS11);
		break;
		case 64:
		TCCR1B	|= (1<<CS11) | (1<<CS10);
		break;
		case 256:
		TCCR1B	|= (1<<CS12);
		break;
		case 1024:
		TCCR1B	|= (1<<CS12) | (1<<CS10);
		break;
		default:
		TCCR1B	|= (1<<CS10);
		break;
	}
	
	// TOP para 50Hz con 1MHz y prescaler 8
	ICR1	= 2499;
	// Posición inicial: 0° (pulso 1ms)
	OCR1A	= 125;
}

void updateDutyCycle1(uint16_t ciclo)
{
	OCR1A = ciclo;
}