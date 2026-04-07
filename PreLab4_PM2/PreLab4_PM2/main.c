/*
 * PreLab4_PM2.c
 *
 * Created: 4/04/2026 20:00:29
 * Author: AnaLucia
 * Description: Contador binario de 8 bits con antirrebote.
 */
/****************************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>

// Variables globales
volatile uint8_t counter		= 0;	// Valor del contador
volatile uint8_t estado_btn0	= 1;	// Estado anterior PC4
volatile uint8_t estado_btn1	= 1;	// Estado anterior PC5


/****************************************/
// Function prototypes
void setup(void);

/****************************************/
// Main Function
int main(void)
{
	cli	();		// Deshabilitar int. globales
	setup();
	sei();		// Habilitar interrupciones
	while(1)
	{
				// Solo esperamos interrupciones, todo lo demás pasa en la ISR	
	}
}
/****************************************/
// NON-Interrupt subroutines
void setup (void)
{
	// Prescaler_CPU = 16 -> F_cpu = 1MHz
	CLKPR	= (1<<CLKPCE);
	CLKPR	= (1<<CLKPS2);
	
	// Puerto C: PC0-PC3 salida (bits 0-3), PC4-PC5 entrada (botones)
	DDRC	= 0x0F;
	PORTC	= 0x30;							// Pull-up en PC4 y PC5
	// Puerto B: PB0-PB3 salida (bits 4-7)
	DDRB	= 0x0F;
	PORTB	= 0x00;
	
	// Habilitar PinChange para Puerto C
	PCICR	|= (1<<PCIE1);					// Interrupciones en el puerto C
	PCMSK1	|= (1<<PCINT12) | (1<<PCINT13);	// Solo PC4 y PC5
	
}

/****************************************/
// Interrupt routines
ISR (PCINT1_vect)
{
	// Todas las variables primero
	uint8_t pinc_actual = PINC;							// Leemos el estado actual del puerto C
	uint8_t btn0_actual = (pinc_actual >> 4) & 0x01;	// Extraemos el bit PC4
	uint8_t btn1_actual = (pinc_actual >> 5) & 0x01;	// Extraemos el bit PC5
	
	/***** BOTÓN PC4 - INCREMENTAR *****/
	// Detectar flanco 1 -> 0 
	if (estado_btn0 == 1 && btn0_actual == 0)
	counter++;
	estado_btn0 = btn0_actual; // Guardamos nuevo estado
	
	/***** BOTÓN PC5 - DECREMENTAR *****/ 
	// Detectar flanco 1 -> 0
	if (estado_btn1 == 1 && btn1_actual == 0)
	counter--;
	estado_btn1 = btn1_actual; // Guardamos nuevo estado
	
	/***** MOSTRAR LEDS *****/
	// Bits 0-3 van a PC0-PC3
	PORTC	= (PORTC & 0x30) | (counter & 0x0F);	// Conservamos PC4/PC5
	// Bits 4-7 van a PB0-PB3
	PORTB	= (PORTB & 0xF0) | ((counter>>4) & 0x0F); // Conservamos PB4/PB5 y bajamos los bits altos 4 posiciones
}


