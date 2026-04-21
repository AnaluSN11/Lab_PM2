/*
 * Lab5_PM2.c
 *
 * Created: 14/04/2026 14:10:58 
 * Author : AnaLucia 
 * Description: Control de servomotor, con PWM del timer 1, con potenciómetro (lectura ADC)
 */
/****************************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>

#include "PWM/PWM.h"

/****************************************/
// Function prototypes
void setup(void);
void initADC(void);


/****************************************/
// Main Function
int main(void)
{
	cli();
	setup();
	initPWM2(no_invertir, fastPWM, 1024);
	initPWM1(no_invertir, fastPWM, 64);
	initPWM_Manual();
	initADC();
	// Iniciar conversión ADC
	ADCSRA	|= (1<<ADSC);
	sei();
	
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
	
	// A0 y A1 como entrada 
	DDRC	&= ~(1<<DDC0);
	DDRC	&= ~(1<<DDC1);
}

void initADC(void)
{
	// Borrar ADMUX
	ADMUX	= 0;
	// Vref = AVcc ; Justificado a la derecha; inicia con ADC = ADC0
	ADMUX	|= (1<<REFS0);
	// Borrar ADCSRA
	ADCSRA	= 0;
	// Habilitar ADC y prescaler = 128 -> 16MHz/128 = 125kHz
	ADCSRA	|= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

/****************************************/
// Interrupt routines
ISR(TIMER0_OVF_vect)
{
	if (pwm_counter >= pwm_duty)
	{
		// Findel pulso o duty=0: salida en BAJO
		PWM_manual_PORT	&= ~(1<<PWM_manual_PIN);
		pwm_counter = 0; // Reiniciar contador
	}  else {
		// Durante elpuslo: salida en ALTO
		PWM_manual_PORT	|= (1<<PWM_manual_PIN);
		pwm_counter++;
	}
}

ISR(ADC_vect)
{
	static uint8_t canal	= 0;
	uint16_t lectura		= ADC;
	
	switch(canal)
	{
		case 0:
		// ADC0 (PC0) -> Servo con Timer2
		{
			// A0 -> Servo con Timer2
			uint8_t ciclo2	= 12 + ((uint32_t)lectura * 25) / 1023; // Mapear a rango del servomotor
			updateDutyCycle2(ciclo2);
			// Cambiar a A1
			ADMUX	= (1<<REFS0) | (1<<MUX0); // ADC1
			canal	= 1;
		}
		break;
		
		case 1:
		// ADC1 (PC1) -> Servo con Timer1
		{
			// A1 -> Servo con Timer1
			uint16_t ciclo1 = 250 + ((uint32_t)lectura * 450) / 1023; // Mapear a rango del servomotor
			updateDutyCycle1(ciclo1);
			// Cambiar a A2
			ADMUX	= (1<<REFS0) | (1<<MUX1); //ADC2
			canal	= 2;
		}
		break;
		
		case 2:
		// ADC2 (PC2)-> LED PWM manual
		{
			uint8_t duty = (uint8_t)((uint32_t)lectura * 255/ 1023);
			updateDutyCycleManual(duty);
			ADMUX	= (1<<REFS0); // ADC0
			canal	= 0;
		}
		break;
		
		default:
		ADMUX	= (1<<REFS0);
		canal	= 0;
		break;
	}
	ADCSRA	|= (1<<ADSC);
}



