/*
 * ADC.c
 *
 * Created: 26/04/2026 22:25:14
 *  Author: AnaLucia
 */ 

#include "ADC.h"

void initADC(void)
{
	// Referencia AVCC, canal ADC0 (PC0)
	ADMUX	= (1<<REFS0);
	// Habilitar ADC, prescaler 128 (16MHz/128 = 125kHz)
	ADCSRA	= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

uint16_t readADC(uint8_t canal)
{
	// Seleccionarcanal (0-7)
	ADMUX	= (ADMUX & 0xF0) | (canal & 0x0F);
	
	ADCSRA	|= (1<<ADSC);		// Iniciar conversión ADC
	while (ADCSRA & (1<<ADSC)); // Esperar fin de conversión
	
	return ADC;					// Retornar valor 10 bits (0-1023)
}