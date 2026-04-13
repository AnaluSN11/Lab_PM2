/*
 * PreLab5_PM2.c
 *
 * Created: 12/04/2026 16:49:45 
 * Author : AnaLucia 
 * Description: Control de servomotor con potenciómetro
 */
/****************************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>
#include "PWM/PWM1.h"

/****************************************/
// Function prototypes
void setup(void);
void initADC(void);

/****************************************/
// Main Function
int main(void)
{
	cli();
	setup();
	initPWM1(no_invertir, fastPWM1, 8);
	initADC();
	// Iniciar conv. ADC
	ADCSRA	|= (1<<ADSC); // Empezar conversiones
	sei();
	
	while(1)
	{
		// Todo lo hace la ISR
	}
	
}

/****************************************/
// NON-Interrupt subroutines
void setup (void)
{
	CLKPR	= (1<<CLKPCE);
	CLKPR	= (1<<CLKPS2); // 16 -> 1MHz
}

void initADC(void)
{
	// Borrar ADMUX
	ADMUX	= 0;
	// Vref = AVcc ; Justificado a la derecha; ADC = ADC6
	ADMUX	|= (1<<REFS0) | (1<<MUX2) | (1<<MUX1);
	// Borrar ADCSRA
	ADCSRA	= 0;
	// Haabiliar ADC y Presclaer = 8 -> 1MHz / 8 = 125kHz
	ADCSRA	|= (1<<ADEN) | (1<<ADIE) | (1<<ADPS1) | (1<<ADPS0);
}
/****************************************/
// Interrupt routines
// ISR - Conversión ADC
ISR(ADC_vect)
{
	uint16_t lectura = ADC; // Leer resultado (0-1023)
	uint16_t ciclo = 125 + ((uint32_t)lectura * 125) / 1023; // Mapear a rango servo
	
	updateDutyCycle1(ciclo);
	ADCSRA	|= (1<<ADSC); // Iniciar siguiente conversión
}