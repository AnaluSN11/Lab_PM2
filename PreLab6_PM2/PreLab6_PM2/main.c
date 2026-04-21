/*
 * PreLab6_PM2.c
 *
 * Created: 20/04/2026 20:35:38
 * Author: AnaLucia
 * Description: 
 */
/****************************************/
// Encabezado (Libraries)
//#define F_CPU 16000000UL
//#include <avr/io.h>
//#include <avr/interrupt.h>
//#include <util/delay.h>
//#include "UART/UART.h"
//#include "ADC/ADC6.h"
//
//volatile char receivedChar = 0; // Carácter recibido por UART
//volatile uint8_t dataReady = 0; // Bandera, hay dato nuevo recibido
///****************************************/
//// Function prototypes
//char mapADCtoChar(uint16_t adcValue);
//
///****************************************/
//// Main Function
//int main(void)
//{
//	cli();
//	// Puerto PC0-PB5 & PD2-PD3 como salida (LEDs)
//	DDRB	= 0x3F;
//	DDRD	|= (1<<DDD2) | (1<<DDB3);
//	PORTB	= 0x00;
//	PORTD	&= ~((1<<PORTD2) | (1<<PORTD3));
//	// Inicializar ADC
//	initADC();
//	// Inicializar UART: asíncrono, sin paridad, 1 stop bit, 8 bits
//	initUART(async, disabled_parity, one_stop_bit, 8);
//	// Habilitar interrupciones globales
//	sei();
//	
//	while (1)
//	{
//		// PARTE 1: Leer potenciómetro y enviar carácter
//		uint16_t adcVal = readADC(0);		// Leer canal ADC0 (PC0)
//		char toSend = mapADCtoChar(adcVal); // Convertir a carácter ASCII
//		writeChar(toSend);					// Enviar por UART a hiperterminal
//		// PARTE 2: Mostrar carácter recibido en Puerto B
//		if (dataReady)
//		{
//			uint8_t data = (uint8_t)receivedChar; // Cáracter ASCII -> LEDs
//			// 6 bits bajos -> PB0-PB5
//			PORTB	= data & 0x3F;
//			// 2 bits altos -> PD2-PD3
//			PORTD	= (PORTD & 0xF3) | ((data & 0xC0)>>4);
//			
//			dataReady = 0;
//		}
//		_delay_ms(100); // Enviar cada 100ms
//	}
//}
///****************************************/
//// NON-Interrupt subroutines
//char mapADCtoChar(uint16_t adcValue)
//{
//	uint8_t index = (uint8_t)((adcValue * 94UL) / 1023);
//	return (char)(32 + index);
//}
//
///****************************************/
//// Interrupt routines
//ISR(USART_RX_vect)
//{
//	receivedChar	= UDR0; // Leer dato recibido 
//	dataReady		= 1;	// Avisar al main qye hay dato nuevo
//	
//	writeChar(receivedChar);
//}

#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
	UBRR0 = 103; // 9600 baud

	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

	while (1)
	{
		if (UCSR0A & (1<<RXC0))
		{
			char c = UDR0;
			while (!(UCSR0A & (1<<UDRE0)));
			UDR0 = c; // eco
		}
	}
}