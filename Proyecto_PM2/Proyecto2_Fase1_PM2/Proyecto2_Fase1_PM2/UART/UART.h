/*
 * UART.h
 *
 * Created: 28/04/2026 11:29:13
 *  Author: AnaLucia
 */ 


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>

// Modos
#define sync			1
#define async			0
// Paridad
#define disabled_parity 1
#define even_parity		2
#define odd_parity		3
// Stop bits
#define one_stop_bit	1
#define two_stop_bit	2
// Baudrate
#define BAUD_RATE		103 // 9600 baud @16MHz


void initUART(uint8_t sync_mode, uint8_t parity, uint8_t stop_bits, uint8_t char_size);
void writeChar(char data);
char readChar(void);



#endif /* UART_H_ */