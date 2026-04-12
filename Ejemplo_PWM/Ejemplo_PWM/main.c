/*
 * Ejemplo_PWM.c
 *
 * Created: 9/04/2026 15:20:53
 * Author: AnaLucia
 * Description: Variar el brillo de dos leds de manera invertida y gradualmente. 
 */
/****************************************/
// Encabezado (Libraries)
#define F_CPU 160000000
#include <avr/io.h>
#include <util/delay.h>

#include "PWM/PWM0.h"

/****************************************/
// Function prototypes
void setup(void);



/****************************************/
// Main Function
int main(void)
{
	uint8_t duty = 127;
	setup();
	initPWM0A(no_invertir, fastPWM, 256);
	initPWM0B(invertir, fastPWM, 8);
	while (1)
	{
		updateDutyCicle0A(duty);
		updateDutyCicle0B(duty);
		duty++;
		_delay_ms(1);
	}
}
/****************************************/
// NON-Interrupt subroutines
void setup (void)
{
	CLKPR	= (1<<CLKPCE);
	CLKPR	= (1<<CLKPS2); // 16-> 1Mhz
}


/****************************************/
// Interrupt routines
