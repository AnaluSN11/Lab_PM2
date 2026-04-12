/*
 * PWM0.c
 *
 * Created: 9/04/2026 15:47:50
 *  Author: AnaLucia
 */ 

#include "PWM0.h"

void initPWM0A(uint8_t invertido, uint8_t modo, uint8_t prescaler)
{
	// Condigurar salidas
	DDRD	|= (1<<DDD6);
	
	// Borrar registros para limpiar
	TCCR0A	= 0;
	TCCR0B	= 0;
	
	if (invertido)
	{
		TCCR0A	|= (1<<COM0A1) | (1<<COM0A0); // Invertido - PD6
	} 
	else
	{
		TCCR0A	|= (1<<COM0A1); // No invertido - PD6
	}
	
	if (modo)
	{
		TCCR0A	|= (1<<WGM01) | (1<<WGM00); // Fast PWM
	} 
	else
	{
		TCCR0A	|= (1<<WGM00); // Phase PWM
	}


	switch(prescaler)
	{
		case 1:
			TCCR0B	|= (1<<CS00);
			break;
		case 8:
			TCCR0B	|= (1<<CS01);
			break;
		case 64:
			TCCR0B	|= (1<<CS01) | (1<<CS00);
			break;
		case 256:
			TCCR0B	|= (1<<CS02);
			break;
		case 1024:
			TCCR0B	|= (1<<CS02) | (1<<CS00);
			break;
		default:
			TCCR0B	|= (1<<CS00);
			break;
	}
}

void initPWM0B(uint8_t invertido, uint8_t modo, uint8_t prescaler)
{
	// Condigurar salidas
	DDRD	|= (1<<DDD5);
	
	// Borrar registros para limpiar
	TCCR0A	&= ~((1<<COM0B1) | (1<<COM0B0));
	TCCR0B	= 0;
	
	if (invertido)
	{
		TCCR0A	|= (1<<COM0A1) | (1<<COM0A0); // Invertido - PD6

	}
	else
	{
		TCCR0A	|= (1<<COM0A1); // No invertido - PD6
	}
	
	if (modo)
	{
		TCCR0A	|= (1<<WGM01) | (1<<WGM00); // Fast PWM
	}
	else
	{
		TCCR0A	|= (1<<WGM00); // Phase PWM
	}


	switch(prescaler)
	{
		case 1:
		TCCR0B	|= (1<<CS00);
		break;
		case 8:
		TCCR0B	|= (1<<CS01);
		break;
		case 64:
		TCCR0B	|= (1<<CS01) | (1<<CS00);
		break;
		case 256:
		TCCR0B	|= (1<<CS02);
		break;
		case 1024:
		TCCR0B	|= (1<<CS02) | (1<<CS00);
		break;
		default:
		TCCR0B	|= (1<<CS00);
		break;
	}
}

void updateDutyCicle0A(uint8_t ciclo)
{
	OCR0A	= ciclo;
}

void updateDutyCicle0B(uint8_t ciclo)
{
	OCR0B	= ciclo;
}
