/*
 * PreLab6_PM2.c
 *
 * Created: 20/04/2026 20:35:38
 * Author: AnaLucia
 * Description: Función para mandar frases a la hiperterminal y para escribir un carácter en la hiperterminal y desplegarlo en 8 Leds. 
 */
/****************************************/
// Encabezado (Libraries)
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UART/UART.h"

// Variables globales
volatile uint8_t receivedChar = 0; // Carácter recibido por UART
volatile uint8_t dataReady    = 0; // Bandera, hay dato nuevo recibido

/****************************************/
//  Function prototypes
void writeString(const char* str);

/****************************************/
// Main Function
int main(void)
{
	cli();
	// Puerto PC0-PB5 & PD2-PD3 como salida (LEDs)
	DDRB	= 0x3F;
	DDRD	|= (1<<DDD2) | (1<<DDD3);
	PORTB	= 0x00;
	PORTD	&= ~((1<<PORTD2) | (1<<PORTD3));

	// Inicializar UART
	initUART(async, disabled_parity, one_stop_bit, 8);
	
	sei();

	_delay_ms(100); // Dar tiempo a la terminal

	// PARTE 1: Enviar mensaje inicial
	writeString("Prueba PreLab6\r\n");

	while (1)
	{
		if (dataReady)
		{
			uint8_t data = receivedChar;

			// Bits 0-5 -> PB0-PB5 directo
			PORTB = (PORTB & 0xC0) | (data & 0x3F);

			// Bits 6-7 del dato -> PD2-PD3 (desplazamos 4 posiciones a la derecha)
			// Bit 6 debe ir a PD2: shift >> 4, bit7 a PD3: shift >> 4
			PORTD = (PORTD & ~((1<<PD2)|(1<<PD3))) | ((data >> 4) & ((1<<PD2)|(1<<PD3))); // Aplicammos mascara y enviamos el dato

			dataReady = 0;
		}
		
	}
}

/****************************************/
// NON-Interrupt subroutines
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