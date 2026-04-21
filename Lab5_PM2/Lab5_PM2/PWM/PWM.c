/*
 * PWM.c
 *
 * Created: 15/04/2026 21:43:59
 *  Author: AnaLucia
 */ 

#include "PWM.h"

volatile uint8_t pwm_duty = 0;
volatile uint8_t pwm_counter = 0;

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
	
	// TOP para 50Hz con 1MHz y prescaler
	ICR1	= 4999;
	// Posición inicial: 0° (pulso 1ms)
	OCR1A	= 250;
}

void updateDutyCycle1(uint16_t ciclo1)
{
	OCR1A = ciclo1;
}


void initPWM2(uint8_t invertido, uint8_t modo, uint16_t prescaler)
{
	// Condigurar salidas - OC2A (PB3)
	DDRB	|= (1<<DDB3);
	
	// Borrar registros para limpiar
	TCCR2A	= 0;
	TCCR2B	= 0;
	
	if (invertido)
	{
		TCCR2A	|= (1<<COM2A1) | (1<<COM2A0); // Invertido
	}
	else
	{
		TCCR2A	|= (1<<COM2A1); // No invertido
	}
	
	if (modo)
	{
		TCCR2A	|= (1<<WGM21) | (1<<WGM20); // Fast PWM - TOP 0xFF
	}
	else
	{
		TCCR2A	|= (1<<WGM20); // Phase PWM - TOP 0xFF
	}

	switch(prescaler)
	{
		case 1:
		TCCR2B	|= (1<<CS20);
		break;
		case 8:
		TCCR2B	|= (1<<CS21);
		break;
		case 32:
		TCCR2B	|= (1<<CS21) | (1<<CS20);
		break;
		case 64:
		TCCR2B	|= (1<<CS22);
		break;
		case 128:
		TCCR2B	|= (1<<CS22) | (1<<CS20);
		break;
		case 256:
		TCCR2B	|= (1<<CS22) | (1<<CS21);
		break;
		case 1024:
		TCCR2B	|= (1<<CS22) | (1<<CS21) | (1<<CS20);
		break;
		default:
		TCCR2B	|= (1<<CS22) | (1<<CS21) | (1<<CS20);
		break;
	}
	
	// Posición inicial: 0° (pulso 1ms)
	OCR2A	= 16;
}


void updateDutyCycle2(uint8_t ciclo)
{
	OCR2A	= ciclo;
}

void initPWM_Manual(void)
{
	// Configurar PB0 como salida
	PWM_manual_DDR	|= (1<<PWM_manual_PIN);
	// LED apagado inicialmente
	PWM_manual_PORT	&= ~(1<<PWM_manual_PIN);
	// Timer0 en modo CTC
	TCCR0A	= (1<<WGM01);
	TCCR0B	= 0;
	// Valor de comparación 
	OCR0A	= 15; // Aprx. 1kHz base
	// Prescaler 64
	TCCR0B	|= (1<<CS01) | (1<<CS00);
	// Habilitar interrupciones para comparación
	TIMSK0	|= (1<<OCIE0A);
}

void updateDutyCycleManual(uint8_t duty)
{
	pwm_duty = duty;
}