/*
 * Lab4_PM2.c
 *
 * Created: 7/04/2026 14:05:18
 * Author: AnaLucia
 * Description: Voltaje con potenciómetro desplegado en displays. 
 */
/****************************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define TCNT0_value 252
volatile uint8_t counter = 0;
// Variables globales
volatile uint8_t num	= 0;
volatile uint8_t alto	= 0;
volatile uint8_t bajo	= 0;
volatile uint8_t displayFlag = 0;

/****************************************/
// Function prototypes
void setup();
void initADC();
void initTMR0();


const uint8_t tabla7Seg[] = {0xEE, 0x82, 0xDC, 0xD6, 0xB2, 0x76, 0x7E, 0xC2, 0xFE, 0xF6, 0xFA, 0x3E, 0x6C, 0x9E, 0x7C, 0x78};  

/****************************************/
// Main Function
int main(void)
{
	// Deshabilitar int.globales
	cli();
	setup();
	initADC();
	// Iniciar conv. ADC
	ADCSRA	|= (1<<ADSC) | (1<<ADIE);
	// Habiliatr interrupciones por overflow del TMR0
	TIMSK0	|= (1<<TOIE0);
	// Habilitar interrupciones globales
	sei();
	while (1)
	{
	}
}

/****************************************/
// NON-Interrupt subroutines
void setup()
{
	// Prescaler_CPU = -> F_cpu = 1MHz
	CLKPR	= (1<<CLKPCE);
	CLKPR	= (1<<CLKPS2);
	// Configurar salidas (DDRD)
	DDRD	= 0xFF; // Todo el puerto D como salida
	PORTD	= 0x00; // Todo el puerto D apagado
	UCSR0B	= 0x00; // Apagar PD0 y PD1
	// Configurar transistores
	DDRB	|= (1<<DDB4) | (1<<DDB5);
	PORTB	&= ~(1<<PORTB4); // Apagados inicialmente
	PORTB	&= ~(1<<PORTB5);
	// Configurar timer0
	initTMR0();
}

void initADC()
{
	// Borrar ADMUX
	ADMUX	= 0;
	// Vref = AVcc ; Justificado a la izquierda; ADC = ADC6
	ADMUX	|= (1<<REFS0) | (1<<ADLAR) | (1<<MUX2) | (1<<MUX1);
	// Borrar ADCSRA
	ADCSRA	= 0;
	// Haabiliar ADC y Presclaer = 8 -> 1MHz / 8 = 125kHz
	ADCSRA	|= (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);
}

void initTMR0()
{
	// Configurar en Modo Normal
	TCCR0A	&= ~((1<<WGM01) | (1<<WGM00));
	TCCR0B	&= ~(1<<WGM02);
	// Configurar prescaler TMR0 = 64
	TCCR0B	&= ~(1<<CS02);
	TCCR0B	|= ((1<<CS01) | (1<<CS00));
	// Iniciar TCNT0
	TCNT0	= TCNT0_value;
}

/****************************************/
// Interrupt routines
ISR (ADC_vect)
{
	num	= ADCH;
	ADCSRA	|= (1<<ADSC);
}

ISR(TIMER0_OVF_vect)
{
	TCNT0	= TCNT0_value;
	counter++;
	if (counter >= 4)
	{
		counter = 0;
		
		// Guardamos las variables para cada dipslay
		alto = (num >> 4) & 0x0F;
		bajo = num & 0x0F;
		
		// Apagamos ambos displays
		PORTB	&= ~(1<<PORTB4);
		PORTB	&= ~(1<<PORTB5);
		
		// Variar displays
		if (displayFlag == 0) {
			PORTD	= tabla7Seg[bajo];	// Enviar patrón al puerto
			PORTB	|= (1<<PORTB4);
			displayFlag = 1;
			} else {
			PORTD	= tabla7Seg[alto];
			PORTB	|= (1<<PORTB5);
			displayFlag = 0;
			}	
	} 
}