/*
 * Proyecto2_Fase1_PM2.c
 *
 * Created: 28/04/2026 10:23:32
 * Author: AnaLucia
 * Description: 
 */
/****************************************/
/* Encabezado                           */
/****************************************/
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "PWM/PWM.h"
#include "UART/UART.h"
#include "ADC/ADC.h"

/****************************************/
/* Definiciones                         */
/****************************************/
#define MANUAL     0
#define ADAFRUIT   1
#define EEPROM		2

// Mapeo OCR1 
#define SERVO_MIN_T1    250
#define SERVO_MAX_T1    700
// Mapeo OCR2 
#define SERVO_MIN_T2    8
#define SERVO_MAX_T2    16

// UART
#define UART_BUF_SIZE	8 // S1:090\n + 1 seguro
// EEPROM
#define NUM_POSICIONES	4
#define EEPROM_BASE 0x00

// LEDs
#define LED1_ON   PORTB |=  (1 << PB0)
#define LED1_OFF  PORTB &= ~(1 << PB0)
#define LED2_ON   PORTB |=  (1 << PB4)
#define LED2_OFF  PORTB &= ~(1 << PB4)

/****************************************/
/* Variables globales                   */
/****************************************/
volatile uint8_t modo		= MANUAL;
volatile uint8_t cambioModo = 0;
// Variables para los servos
volatile uint8_t servo1 = 90;
volatile uint8_t servo2 = 90;
volatile uint8_t servo3 = 90;
volatile uint8_t servo4 = 90;

// UART
volatile char uartBuf[UART_BUF_SIZE];
volatile uint8_t uartIdx = 0;
volatile uint8_t uartListo = 0;

// Variables para EEPROM
volatile uint8_t poseActual		= 0; //0-3
volatile uint8_t grabarPose		= 0; // bandera PD4
volatile uint8_t navegarPose	= 0; // 0=nada, 1=siguiente, 2=anterior

/****************************************/
/* Prototipos                           */
/****************************************/
void setup(void);
void modoManual(void);
void modoAdafruit(void);
void modoEEPROM(void);
uint16_t mapeoT1(uint8_t angulo);
uint8_t mapeoT2(uint8_t angulo);
void guardarPose(uint8_t pose, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4);
void cargarPose(uint8_t pose);
void enviarPosicion(uint8_t servo, uint8_t angulo);

/****************************************/
/* Main                                 */
/****************************************/
int main(void)
{
    cli();
    setup();

    // Timer1: prescaler 64 -> 50 Hz exacto con ICR1=4999
    initTimer1(fastPWM, 64);
    initPWM1A(no_invertir);   // PB1 -> Servo 1
    initPWM1B(no_invertir);   // PB2 -> Servo 2

    // Timer2: Servo 3 y 4
    initTimer2(phasePWM, 1024);
    initPWM2A(no_invertir);   // PB3 -> Servo 3
    initPWM2B(no_invertir);   // PD3 -> Servo 4
	
	updateDutyCycle1A(mapeoT1(90));
	updateDutyCycle1B(mapeoT1(90));
	updateDutyCycle2A(mapeoT2(90));
	updateDutyCycle2B(mapeoT2(90));

    // ADC 
    initADC();
    ADCSRA |= (1 << ADSC);

    // UART
    initUART(async, disabled_parity, one_stop_bit, 8);

    sei();
    _delay_ms(100);
    while (1)
    {
		if (cambioModo)
		{
			cambioModo = 0;
			switch (modo)
			{
				case MANUAL: LED1_ON; LED2_OFF; break;
				case ADAFRUIT: LED1_OFF; LED2_ON; break;
				case EEPROM: 
				LED1_ON; LED2_ON; 
				poseActual = 0; // Siempre empieza en pose 0
				cargarPose(poseActual); // Carga inmediatamente
				break;
			}
		}
		
		switch(modo)
		{
			case MANUAL: modoManual(); break;
			case ADAFRUIT: modoAdafruit(); break;
			case EEPROM: modoEEPROM(); break;
		}
	}
}

/****************************************/
/* Setup                                */
/****************************************/
void setup(void)
{
	// Clock a 16MHz
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;
	// De A0-A3 son entradas -> potenciómetros
    DDRC	&= ~((1 << DDC0) | (1 << DDC1) | (1 << DDC2) | (1 << DDC3));
	// LEDs
	DDRB	|= (1<<DDB0) | (1<<DDB4);
	LED1_ON; LED2_OFF;
	// PD2 -> Boton Modo
	DDRD &= ~(1<<DDD2); 
	PORTD |= (1<<PORTD2);
	// PD4 -> Boton para grabar en EEPROM
	DDRD &= ~(1<<DDD4); 
	PORTD |= (1<<PORTD4);
	// PD5 -> Pose siguiente
	DDRD &= ~(1<<DDD5);
	PORTD |= (1<<PORTD5); // Pull-up interno
	// PD6 -> Pose anterior
	DDRD &= ~(1<<DDD6);
	PORTD |= (1<<PORTD6); // Pull-up interno
	// INT0 
	EICRA |= (1<<ISC01); 
	EICRA &= ~(1<<ISC00); 
	EIMSK |= (1<<INT0); 
	// PCINT para PD4, PD5, PD6 (todos en PCIE2)
	PCICR	|= (1<<PCIE2);
	PCMSK2	|= (1<<PCINT20) | (1<<PCINT21) | (1<<PCINT22);
}
/****************************************/
/* MODOS                                */
/****************************************/
void modoManual(void)
{
    if (grabarPose)
    {
	    grabarPose = 0;
	    guardarPose(poseActual, servo1, servo2, servo3, servo4);
	    poseActual = (poseActual + 1) % NUM_POSICIONES; // avanza ranura
    }
}

void modoAdafruit(void)
{
	if (uartListo)
	{
		uartListo = 0;
		if (uartBuf[0] == 'S' && uartBuf[2] == ':')
		{
			uint8_t servo	= uartBuf[1] - '0';
			uint8_t angulo	= (uartBuf[3] - '0') * 100
							+ (uartBuf[4] - '0') * 10
							+ (uartBuf[5] - '0');
			if (servo >= 1 && servo <= 4 && angulo <= 180)
			{
				switch (servo)
				{
					case 1: servo1 = angulo; updateDutyCycle1A(mapeoT1(angulo)); enviarPosicion(1, angulo); break;
					case 2: servo2 = angulo; updateDutyCycle1B(mapeoT1(angulo)); enviarPosicion(2, angulo); break;
					case 3: servo3 = angulo; updateDutyCycle2A(mapeoT2(angulo)); enviarPosicion(3, angulo); break;
					case 4: servo4 = angulo; updateDutyCycle2B(mapeoT2(angulo)); enviarPosicion(4, angulo); break;
				}
			}
		}
	}
}

void modoEEPROM(void)
{
    if (navegarPose)
    {
	    if (navegarPose == 1)       // siguiente
	    poseActual = (poseActual + 1) % NUM_POSICIONES;
	    else if (navegarPose == 2)  // anterior
	    poseActual = (poseActual + NUM_POSICIONES - 1) % NUM_POSICIONES;

	    navegarPose = 0;
	    cargarPose(poseActual);
    }
}

/****************************************/
/* Función auxiliar: mapear ángulo      */
/* a OCR según timer                    */
/****************************************/

uint16_t mapeoT1(uint8_t angulo)
{
	return SERVO_MIN_T1 + ((uint32_t)angulo * (SERVO_MAX_T1 - SERVO_MIN_T1)) / 180;
}

uint8_t mapeoT2(uint8_t angulo)
{
	return SERVO_MIN_T2 + ((uint32_t)angulo * (SERVO_MAX_T2 - SERVO_MIN_T2)) / 180;
}

/****************************************/
/* Grabar pose en EEPROM                */
/****************************************/
void guardarPose(uint8_t pose, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4)
{
	uint8_t *base = (uint8_t*)(EEPROM_BASE + (pose * 4));
	eeprom_update_byte(base,     s1);
	eeprom_update_byte(base + 1, s2);
	eeprom_update_byte(base + 2, s3);
	eeprom_update_byte(base + 3, s4);
}

/****************************************/
/* Cargar pose desde EEPROM             */
/****************************************/
void cargarPose(uint8_t pose)
{
	uint8_t *base = (uint8_t*)(EEPROM_BASE + (pose * 4));
	servo1 = eeprom_read_byte(base);
	servo2 = eeprom_read_byte(base + 1);
	servo3 = eeprom_read_byte(base + 2);
	servo4 = eeprom_read_byte(base + 3);
	
	if (servo1 > 180) servo1 = 90;
	if (servo2 > 180) servo2 = 90;
	if (servo3 > 180) servo3 = 90;
	if (servo4 > 180) servo4 = 90;

    updateDutyCycle1A(mapeoT1(servo1));
    updateDutyCycle1B(mapeoT1(servo2));
    updateDutyCycle2A(mapeoT2(servo3));
    updateDutyCycle2B(mapeoT2(servo4));
	
    // Informar posiciones actuales
    enviarPosicion(1, servo1);
    enviarPosicion(2, servo2);
    enviarPosicion(3, servo3);
    enviarPosicion(4, servo4);
}

/****************************************/
/* Enviar posición de servo por UART    */
/****************************************/
void enviarPosicion(uint8_t servo, uint8_t angulo)
{
	writeChar('R');
	writeChar('0' + servo);
	writeChar(':');
	writeChar('0' + (angulo / 100));
	writeChar('0' + (angulo % 100) / 10);
	writeChar('0' + (angulo % 10));
	writeChar('\n');
}


/****************************************/
/* ISR: ADC                             */
/* Conversión encadenada A0->A1->A2->A3 */
/* Con ciclo de descarte al cambiar canal*/
/****************************************/
ISR(ADC_vect)
{
    static uint8_t canal        = 0;
    static uint8_t estabilizando = 0; // Descarta primera lectura al cambiar canal
    uint16_t lectura = ADC;

    if (modo != MANUAL)
	{
		ADCSRA	|= (1<<ADSC); // Mantener corriendo
		return;
	}

    if (estabilizando)
    {
        estabilizando = 0;          // Descartar esta lectura, la siguiente es válida
        ADCSRA |= (1<<ADSC);
        return;
    }

    switch (canal)
    {
        case 0: // A0 -> Servo 1 (OC1A, Timer1)
        {
            uint16_t ocr = SERVO_MIN_T1 + ((uint32_t)lectura * (SERVO_MAX_T1 - SERVO_MIN_T1)) / 1023;
            updateDutyCycle1A(ocr);
			servo1 = ((uint32_t)lectura * 180) / 1023;
            ADMUX = (1 << REFS0) | (1 << MUX0); // Siguiente: ADC1
            canal = 1;
            estabilizando = 1;
        }
        break;

        case 1: // A1 -> Servo 2 (OC1B, Timer1)
        {
            uint16_t ocr = SERVO_MIN_T1 + ((uint32_t)lectura * (SERVO_MAX_T1 - SERVO_MIN_T1)) / 1023;
            updateDutyCycle1B(ocr);
			servo2 = ((uint32_t)lectura * 180) / 1023;
            ADMUX = (1 << REFS0) | (1 << MUX1); // Siguiente: ADC2
            canal = 2;
            estabilizando = 1;
        }
        break;

        case 2: // A2 -> Servo 3 (OC2A, Timer2)
        {
            uint8_t ocr = SERVO_MIN_T2 + ((uint32_t)lectura * (SERVO_MAX_T2 - SERVO_MIN_T2)) / 1023;
            updateDutyCycle2A(ocr);
			servo3 = ((uint32_t)lectura * 180) / 1023;
            ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0); // Siguiente: ADC3
            canal = 3;
            estabilizando = 1;
        }
        break;

        case 3: // A3 -> Servo 4 (OC2B, Timer2)
        {
            uint8_t ocr = SERVO_MIN_T2 + ((uint32_t)lectura * (SERVO_MAX_T2 - SERVO_MIN_T2)) / 1023;
            updateDutyCycle2B(ocr);
			servo4 = ((uint32_t)lectura * 180) / 1023;
            ADMUX = (1 << REFS0); // Siguiente: ADC0
            canal = 0;
            estabilizando = 1;
        }
        break;

        default:
            ADMUX = (1 << REFS0);
            canal = 0;
        break;
    }

    ADCSRA |= (1 << ADSC);
}
/****************************************/
/* ISR: Botones de la EEPROM (PD4-PD6)  */
/****************************************/
ISR(PCINT2_vect)
{
	// PD4 -> grabar (solo actúa en modo MANUAL)
	if (!(PIND & (1<<PIND4)) && modo == MANUAL)
	{
		grabarPose = 1;
	}
	// PD5 -> siguiente pose
	if (!(PIND & (1<<PIND5)) && modo == EEPROM)
	{
		navegarPose = 1; // siguiente
	}
	// PD6 -> pose anterior
	if (!(PIND & (1<<PIND6)) && modo == EEPROM)
	{
		navegarPose = 2; // anterior
	}
}



/****************************************/
/* ISR: INT0 — Botón modo (PD2)         */
/****************************************/
ISR(INT0_vect)
{
	modo = (modo + 1) % 3; 
	cambioModo = 1; // Activamos bandera para cambiar de modo
}

/****************************************/
/* ISR: UART RX                         */
/****************************************/
ISR(USART_RX_vect)
{
	char c = UDR0;
	
    if (c == '\n' || c == '\r')
    {
	    uartBuf[uartIdx] = '\0';   // cerrar string
	    uartListo = 1;             // avisar al while
	    uartIdx   = 0;             // resetear índice
    }
    else if (uartIdx < UART_BUF_SIZE - 1)
    {
	    uartBuf[uartIdx++] = c;
    }
    // si desborda simplemente ignoramos
} 