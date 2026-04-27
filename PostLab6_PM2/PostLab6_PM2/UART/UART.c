/*
 * UART.c
 *
 * Created: 26/04/2026 22:27:58
 *  Author: AnaLucia
 */ 

#include "UART.h"

void initUART(uint8_t sync_mode, uint8_t parity, uint8_t stop_bits, uint8_t char_size)
{
	// Configruar pines
	DDRD	&= ~(1<<DDD0); // D0 = RX -> entrada
	DDRD	|= (1<<DDD1); // D1 = TX -> salida
	// Limpiar registro UCSR0C antes de configurar
	UCSR0C	= 0x00;
	
	// Seleccionar modo Síncrono/Asíncrono
	if (sync_mode == async) {
		UCSR0C	&= ~(1<<UMSEL01);
		UCSR0C	&= ~(1<<UMSEL00);
		} else {
		UCSR0C	|= (1<<UMSEL00);
		UCSR0C	&= ~(1<<UMSEL01);
	}
	
	// Seleccionar paridad
	switch(parity)
	{
		case disabled_parity:
		UCSR0C	&= ~(1<<UPM01);
		UCSR0C	&= ~(1<<UPM00);
		break;
		case even_parity:
		UCSR0C	|=  (1<<UPM01);
		UCSR0C	&= ~(1<<UPM00);
		break;
		case odd_parity:
		UCSR0C	|= (1<<UPM01);
		UCSR0C	|= (1<<UPM00);
		break;
		default:
		UCSR0C	&= ~(1<<UPM01);
		UCSR0C	&= ~(1<<UPM00);
		break;
	}

	// Seleccionar cantidad de Stop Bits
	if (stop_bits == one_stop_bit) {
		UCSR0C	&= ~(1<<USBS0);
		} else {
		UCSR0C	|= (1<<USBS0);
	}
	
	// Seleccionar cantidad de bits
	UCSR0B &= ~(1<<UCSZ02);  // Limpiar primero
	switch(char_size)
	{
		case 5:
		UCSR0C	&= ~(1<<UCSZ01);
		UCSR0C	&= ~(1<<UCSZ00);
		break;
		case 6:
		UCSR0C	&= ~(1<<UCSZ01);
		UCSR0C	|=  (1<<UCSZ00);
		break;
		case 7:
		UCSR0C	|=  (1<<UCSZ01);
		UCSR0C	&= ~(1<<UCSZ00);
		break;
		case 8:
		default:
		UCSR0C	|= (1<<UCSZ01);
		UCSR0C	|= (1<<UCSZ00);
		break;
		case 9:
		UCSR0B	|=  (1<<UCSZ02);
		UCSR0C	|=  (1<<UCSZ01);
		UCSR0C	|=  (1<<UCSZ00);
		break;
	}
	// Habilitar int. RX; Habilitar RX y TX
	UCSR0B	|= (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
	
	// Cargar UBRR0 (valor obtenido con los cálculos)
	UBRR0 = UBRR_VALUE;
}

// Enviar un carácter por TX
void writeChar(char data)
{
	while (!(UCSR0A & (1<<UDRE0))); // Esperar que el buffer esté listo
	UDR0 = data;
}

// Recibir un carácter por RX
char readChar(void)
{
	while (!(UCSR0A & (1<<RXC0))); // Esperar dato recibido
	return UDR0;
}