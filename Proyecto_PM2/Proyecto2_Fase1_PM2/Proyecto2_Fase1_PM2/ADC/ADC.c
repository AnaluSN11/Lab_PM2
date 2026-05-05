/*
 * ADC.c
 *
 * Created: 28/04/2026 14:16:31
 *  Author: AnaLucia
 */ 

#include "ADC.h"

void initADC(void)
{
	// AVCC como referencia, justificado a la derecha, canal ADC0 (PC0)
	ADMUX	= (1<<REFS0);
	// Habilitar ADC, interrupción habilitada, prescaler 128 (16MHz/128 = 125kHz)
	ADCSRA	= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}