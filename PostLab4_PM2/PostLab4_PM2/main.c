/*
 * PostLab4_PM2.c
 *
 * Created: 9/04/2026 12:44:19
 * Author: AnaLucia
 * Description: Comparador de voltaje manejado por un potenciómetro y desplegado en displays, y contador en leds de 8 bits. 
 */
/****************************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define TCNT0_value 252
volatile uint8_t counter		= 0;
volatile uint8_t counterLED		= 0;
volatile uint8_t estado_btn0	= 1;	// Estado anterior PC4
volatile uint8_t estado_btn1	= 1;	// Estado anterior PC5
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
	// Puerto C: PC0-PC3 salida (bits 0-3), PC4-PC5 entrada (botones)
	DDRC	= 0x0F;
	PORTC	= 0x30;							// Pull-up en PC4 y PC5
	// Puerto B: PB0-PB5 salida (Leds bits 4-7) y transistores
	DDRB	= 0x3F;
	PORTB	= 0x00; // Todo apagado inicialmente
	// Configurar salidas (DDRD)
	DDRD	= 0xFF; // Todo el puerto D como salida
	PORTD	= 0x00; // Todo el puerto D apagado
	UCSR0B	= 0x00; // Apagar PD0 y PD1
	// Habilitar PinChange para Puerto C
	PCICR	|= (1<<PCIE1);					// Interrupciones en el puerto C
	PCMSK1	|= (1<<PCINT12) | (1<<PCINT13);	// Solo PC4 y PC5
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
ISR(PCINT1_vect)
{
	// Todas las variables primero
	uint8_t pinc_actual = PINC;							// Leemos el estado actual del puerto C
	uint8_t btn0_actual = (pinc_actual >> 4) & 0x01;	// Extraemos el bit PC4
	uint8_t btn1_actual = (pinc_actual >> 5) & 0x01;	// Extraemos el bit PC5
	
	/***** BOTÓN PC4 - INCREMENTAR *****/
	// Detectar flanco 1 -> 0
	if (estado_btn0 == 1 && btn0_actual == 0)
	counterLED++;
	estado_btn0 = btn0_actual; // Guardamos nuevo estado
	
	/***** BOTÓN PC5 - DECREMENTAR *****/
	// Detectar flanco 1 -> 0
	if (estado_btn1 == 1 && btn1_actual == 0)
	counterLED--;
	estado_btn1 = btn1_actual; // Guardamos nuevo estado
	
	/***** MOSTRAR LEDS *****/
	// Bits 0-3 van a PC0-PC3
	PORTC	= (PORTC & 0x30) | (counterLED & 0x0F);	// Conservamos PC4/PC5
	// Bits 4-7 van a PB0-PB3
	PORTB	= (PORTB & 0xF0) | ((counterLED>>4) & 0x0F); // Conservamos PB4/PB5 y bajamos los bits altos 4 posiciones
}


ISR (ADC_vect)
{
	num	= ADCH;
	ADCSRA	|= (1<<ADSC);
	
	// Comparar voltaje (num) con contador (counterLED)
	if (num == counterLED)
	{
		PORTD	|= (1<<PD0);
	} 
	else
	{
		PORTD	&= ~(1<<PD0);
	}
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
			PORTD	= (tabla7Seg[bajo] & 0xFE) | (PORTD & 0x01);	// Enviar patrón al puerto conservando alarma
			PORTB	|= (1<<PORTB4);
			displayFlag = 1;
			} else {
			PORTD	= (tabla7Seg[alto] & 0xFE) | (PORTD & 0x01);
			PORTB	|= (1<<PORTB5);
			displayFlag = 0;
			}	
	} 
}