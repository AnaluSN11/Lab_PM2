/*
 * PostLab6_PM2.c
 *
 * Created: 26/04/2026 21:44:24
 * Author: AnaLucia
 * Description: Menú en la hiperterminal: 
 *				1. Leer potenciómetro (solo se realiza una lectura)
 *				2. Enviar carácter y mostrarlo en LEDs en ASCII
 */
/****************************************/
// Encabezado (Libraries)
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ADC/ADC.h"
#include "UART/UART.h"

// Variables globales
volatile uint8_t receivedChar	= 0; // Carácter recibido por UART
volatile uint8_t dataReady		= 0; // Bandera, hay dato nuevo recibido
/****************************************/
// Function prototypes
char mapADCtoChar(uint16_t adcValue);
void writeString(const char* str);

/****************************************/
// Main Function
int main(void)
{
	cli();
	
	// PB0-PB5 & PD2-PD3 como salidas (LEDs)
	DDRB	= 0x3F;
	DDRD	|= (1<<DDD2) | (1<<DDD3);
	PORTB	= 0x00;		// Inicialmente apagados
	PORTD	&= ~((1<<PORTD2) | (1<<PORTD3));
	// Inicializamos el ADC
	initADC();
	// Inicializamos UART
	initUART(async, disabled_parity, one_stop_bit, 8);
	
	sei();
	_delay_ms(100); // Damos tiempo a la terminal
	
	while (1)
	{
		// Mostramos el menú
		writeString("===== MENU =====\r\n");
		writeString("1. Leer Potenciómetro\r\n");
		writeString("2. Enviar caracter a LEDs\r\n");
		writeString("Ingrese número: ");
		
		// Esperar a que eliga la opción
		while (!dataReady);
		uint8_t option = receivedChar;
		dataReady = 0;
		
		// Limpiamos el \r o \n que queda del Enter
		_delay_ms(10);
		dataReady = 0;
		
		writeString("\r\n");
		
		if (option == '1')
		{
			// Leer potenciómetro
			uint16_t adcVal = readADC(0); // Leer canal ADC0 (PC0)
			writeString("Potenciómetro: ");		// Enviar por UART a hiperterminal
			char toSend = mapADCtoChar(adcVal); // Convertir a carácter ASCII
			writeChar(toSend);
			writeString("\r\n");
		} 
		else if (option == '2')
		{
			writeString("Ingresar un caracter: ");
			
			while (!dataReady);
			uint8_t data = receivedChar;
			dataReady = 0;
			
			// Limpiar el \r o \n del Enter
			_delay_ms(10);
			dataReady = 0;
			
			// Mostramos en LEDs 
			// Bits 0-5 -> PB0-PB5 directo
			PORTB	= (PORTB & 0xC0) | (data & 0x3F);
			// Bits 6-7 del dato -> PD2-PD3 (desplazamos 4 posiciones a la derecha)
			PORTD	= (PORTD & ~((1<<PD2) | (1<<PD3))) | ((data>>4) & ((1<<PD2) | (1<<PD3)));
			
			writeString("\r\n");
		}
		
		_delay_ms(500);
	}
}

/****************************************/
// NON-Interrupt subroutines
char mapADCtoChar(uint16_t adcValue)
{
	uint8_t index = (uint8_t)((adcValue * 94UL) / 1023); // Conversión a ASCII
	return (char)(32 + index);
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
	receivedChar	= UDR0;	// Leer dato recibido
	dataReady		= 1;	// Avisar al main que hay dato nuevo 
}
