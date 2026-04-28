/*
 * Proyecto2_Fase1_PM2.c
 *
 * Created: 28/04/2026 10:23:32
 * Author: AnaLucia
 * Description: 
 */
/****************************************/
// Encabezado (Libraries)
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "PWM/PWM.h"
#include "UART/UART.h"

// Variables globales
volatile uint8_t receivedChar	= 0; // Carácter recibido por UART
volatile uint8_t dataReady		= 0; // Bandera, hay dato nuevo recibido
/****************************************/
// Function prototypes
void setup(void);
void writeString(const char* str);


/****************************************/
// Main Function
int main(void)
{
	cli();
	setup();
	// Inicializar servos con Timer1
	initTimer1(fastPWM, 1024);
	initPWM1A(no_invertir);
	initPWM1B(no_invertir);
	// Inicializar servos con Timer2
	initTimer2(fastPWM, 64);
	initPWM2A(no_invertir);
	initPWM2B(no_invertir);
	// Inicializar UART: asíncrono, sin paridad, 1 stop bit, 8 bits
	initUART(async, disabled_parity, one_stop_bit, 8);
	sei();
	
	_delay_ms(100); // Damos tiempo a la terminal
	
	while(1)
	{
		
	}
}

/****************************************/
// NON-Interrupt subroutines
void setup (void)
{
	CLKPR	= (1<<CLKPCE);
	CLKPR	= 0; // No aplicamos prescaler -> 16MHz
	
	// A0-A3 como entrada
	DDRC	&= ~((1<<DDC0) | (1<<DDC1) | (1<<DDC2) | (1<<DDC3));
}

void writeString(const char* str)
{
	while (*str)
	{
		writeChar(*str++);
	}
}

/****************************************/
// Interrupt routines
ISR(USART_RX_vect)
{
	receivedChar = UDR0;

	// Eco en terminal (como el código que te dieron)
	writeChar(receivedChar);

	dataReady = 1;
}
