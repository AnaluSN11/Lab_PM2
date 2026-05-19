/*
 * Proyecto2_Fase1_PM2.c
 *
 * Created: 28/04/2026 10:23:32
 * Author: AnaLucia
 * Description: Control de 4 servomotores - Animatrónico
 * Modos de funcionamiento:
 * 1. Control manueal mediante potenciómetros (ADC)
 * 2. Control remoto mediante Adafruit IO usando UART
 * 3. Reproducción de posiciones almacenadas en EEPROM
 */
/****************************************/
/* Encabezado                           */
/****************************************/
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdlib.h>

#include "PWM/PWM.h"
#include "UART/UART.h"
#include "ADC/ADC.h"

/****************************************/
/* Definiciones                         */
/****************************************/
#define MANUAL		0
#define ADAFRUIT	1
#define EEPROM		2

// Rango OCR para Timer1 
#define SERVO_MIN_T1    250
#define SERVO_MAX_T1    700
// Rango OCR para Timer2
#define SERVO_MIN_T2    8
#define SERVO_MAX_T2    16

// Tamaño del buffer de recepción UART
#define UART_BUF_SIZE	32 

// Número de poses almacenables en EEPROM
#define NUM_POSICIONES	4
// Dirección base en EEPROM (donde inicia el bloque de poses)
#define EEPROM_BASE		0x00

// Control de LEDs para estados
#define LED1_ON   PORTB |=  (1 << PB0)
#define LED1_OFF  PORTB &= ~(1 << PB0)
#define LED2_ON   PORTB |=  (1 << PB4)
#define LED2_OFF  PORTB &= ~(1 << PB4)

/****************************************/
/* Variables globales                   */
/****************************************/
volatile uint8_t modo = MANUAL;		// Modo de operación actual (MANUAL/ADAFRUIT/EEPROM)
volatile uint8_t cambioModo = 0;	// Bandera que indica cambio de modo pendiente de procesar

// Ángulo actual de cada servo
volatile uint8_t servo1 = 90;
volatile uint8_t servo2 = 90;
volatile uint8_t servo3 = 90;
volatile uint8_t servo4 = 90;

// UART
volatile char uartBuf[UART_BUF_SIZE]; // Buffer UART para recibir comandos carácter a carácter
volatile uint8_t uartIdx = 0;		// índice de escritura en el buffer
volatile uint8_t uartListo = 0;		// 1 cuando se recibió '\n'

// EEPROM
volatile uint8_t poseActual		= 0; // índice de la pose activa en EEPROM (0-3)
volatile uint8_t grabarPose		= 0; // Bandera activada por PD4: guardar la pose actual en EEPROM
volatile uint8_t navegarPose	= 0; // Solicitud de navegación en modo EEPROM (0=nada, 1=siguiente, 2=anterior)

/****************************************/
/* Prototipos                           */
/****************************************/
void setup(void);

void modoManual(void);
void modoAdafruit(void);
void modoEEPROM(void);

uint16_t mapeoT1(uint8_t angulo);
uint8_t mapeoT2(uint8_t angulo);

void moverServo(uint8_t servo, uint8_t angulo);

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

    // Timer1: servo 1 y 2 (prescaler 64 -> 50 Hz exacto con ICR1=4999)
    initTimer1(fastPWM, 64);
    initPWM1A(no_invertir);   // PB1 -> Servo 1
    initPWM1B(no_invertir);   // PB2 -> Servo 2

    // Timer2: Servo 3 y 4 (prescaler 1024 -> aprox 50 Hz con TOP = 255)
    initTimer2(phasePWM, 1024);
    initPWM2A(no_invertir);   // PB3 -> Servo 3
    initPWM2B(no_invertir);   // PD3 -> Servo 4
	
	// Mover todos los servos a posición central (90°) para iniciar
	moverServo(1, 90);
	moverServo(2, 90);
	moverServo(3, 90);
	moverServo(4, 90);
	
    // ADC: inicializar e iniciar primera conversión  
    initADC();
    ADCSRA |= (1 << ADSC);

    // Inicialización de UART
    initUART(async, disabled_parity, one_stop_bit, 8);

    sei();
    _delay_ms(100); // Espera breve para estabilizar periféricos
    while (1)
    {
		/*----- Procesar cambio de modo -----*/
		// La ISR de INT0 (para botones) activa cambioModo, aquí se aplica el efecto
		if (cambioModo)
		{
			cambioModo = 0;
			switch (modo)
			{
				case MANUAL: LED1_ON; LED2_OFF; break;		// LED1 = modo MANUAL activo
				case ADAFRUIT: LED1_OFF; LED2_ON; break;	// LED2 = modo ADAFRUIT activo
				case EEPROM: 
				LED1_ON; LED2_ON;							// Ambos LEDs = modo EEPROM
				poseActual = 0;								// Siempre empieza en pose 0
				cargarPose(poseActual);						// Carga inmediatamente
				break;
			}
		}
		
		/*----- Ejecutar lógica del modo activo -----*/
		if (modo == MANUAL)
		{
			modoManual();
		}
		else if (modo == ADAFRUIT)
		{
			modoAdafruit();
		}
		else
		{
			modoEEPROM();
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
	// PD2 -> Boton cambio de modo 
	DDRD &= ~(1<<DDD2); 
	PORTD |= (1<<PORTD2); // Pull-up interno
	// PD4 -> Boton para grabar en EEPROM
	DDRD &= ~(1<<DDD4); 
	PORTD |= (1<<PORTD4);
	// PD5 -> Pose siguiente
	DDRD &= ~(1<<DDD5);
	PORTD |= (1<<PORTD5); // Pull-up interno
	// PD6 -> Pose anterior
	DDRD &= ~(1<<DDD6);
	PORTD |= (1<<PORTD6); // Pull-up interno
	// INT0 (PD2): configurado para disparar en flanco de bajada
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
	    poseActual = (poseActual + 1) % NUM_POSICIONES; // Siguiente fila EEPROM
    }
}

void modoAdafruit(void)
{
	if (!uartListo)
	return;

	uartListo = 0;
	
	// Copiar buffer volátil a buffer local para un procesamiento seguro
	char buf[UART_BUF_SIZE];
	for (uint8_t k = 0; k < UART_BUF_SIZE; k++)
	buf[k] = uartBuf[k];

	// Echo de depuración: imprime el frame recibido como [frame]
	writeChar('[');
	for (uint8_t k = 0; buf[k] != '\0' && k < UART_BUF_SIZE; k++)
	writeChar(buf[k]);
	writeChar(']');
	writeChar('\n');
	// Buscar delimitador ':' que separa "servo" de "angulo"
	uint8_t i = 0;
	while (buf[i] != ':' && buf[i] != '\0' && i < UART_BUF_SIZE)
	i++;
	// Hay que validar que se encontró ':' y que hay al menos un carácter antes
	if (buf[i] != ':' || i == 0)
	return;
	// Estrar número de servo (dígito antes de ':') y ángulo (texto tras ':')
	uint8_t servo  = buf[i - 1] - '0';
	uint8_t angulo = (uint8_t)atoi(&buf[i + 1]);
	// Mover solo si el servo y ángulo son válidos
	if (servo >= 1 && servo <= 4 && angulo <= 180)
	{
		moverServo(servo, angulo);
		enviarPosicion(servo, angulo);	// Confirmación por UART
	}
}

void modoEEPROM(void)
{
	if (navegarPose)
	{
		if (navegarPose == 1)
		{
			poseActual = (poseActual + 1) % NUM_POSICIONES;		// Siguiente pose
		}
		else if (navegarPose == 2)
		{
			poseActual = (poseActual + NUM_POSICIONES - 1) % NUM_POSICIONES;	// Pose anterior
		}
		
		navegarPose = 0;
		cargarPose(poseActual); // Mover servors
	}
}

/****************************************/
/* Función mover servo                  */
/****************************************/
void moverServo(uint8_t servo, uint8_t angulo)
{
	if (angulo > 180)
		return; // Si es un ángulo fuera de rango lo ignoramos
	
	switch (servo)
	{
		case 1:
			servo1 = angulo;
			updateDutyCycle1A(mapeoT1(angulo));
			break;
		case 2:
			servo2 = angulo;
			updateDutyCycle1B(mapeoT1(angulo));
			break;
		case 3:
			servo3 = angulo;
			updateDutyCycle2A(mapeoT2(angulo));
			break;
		case 4: 
			servo4 = angulo;
			updateDutyCycle2B(mapeoT2(angulo));
			break;
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
	// Se usa eeprom_update_buye para no escribir si el valor no cambió
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
	// Se leen los 4 bytes desde la EEPROM y mueve los servos
	servo1 = eeprom_read_byte(base);
	servo2 = eeprom_read_byte(base + 1);
	servo3 = eeprom_read_byte(base + 2);
	servo4 = eeprom_read_byte(base + 3);
	// Si algún valor leído es invalido lo reemplaza por 90°
	if (servo1 > 180) servo1 = 90;
	if (servo2 > 180) servo2 = 90;
	if (servo3 > 180) servo3 = 90;
	if (servo4 > 180) servo4 = 90;

    moverServo (1, servo1);
    moverServo (2, servo2);
    moverServo (3, servo3);
    moverServo (4, servo4);
	
    // Informar posiciones actuales por UART
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
	writeChar('0' + servo); // Numero de servo
	writeChar(':');
	
	if (angulo >= 100)
	{
		writeChar('0' + (angulo / 100));		// Centenas
	}
	if (angulo >= 10)
	{
		writeChar('0' + (angulo / 10) % 10);	// Decenas
	}
	else
	{
		writeChar('0');							// Relleno con '0' si < 10
	}
	
	writeChar('0' + (angulo % 10));				// Unidades
	writeChar('\n');
}


/****************************************/
/* ISR: ADC                             */
/* Conversión encadenada A0->A1->A2->A3 */
/* Con ciclo de descarte al cambiar canal*/
/****************************************/
ISR(ADC_vect)
{
    static uint8_t canal		 = 0; // Canal activo (0-3 -> A0-A3)
    static uint8_t estabilizando = 0; // 1 = descartar la lectura 
    uint16_t lectura = ADC;
	// En modos no manuales, solo se mantiene el ADC corriendo
    if (modo != MANUAL)
	{
		ADCSRA	|= (1<<ADSC);
		return;
	}
	// Descartar primera lectura
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
			moverServo(1, ((uint32_t)lectura * 180) / 1023);
            ADMUX = (1 << REFS0) | (1 << MUX0); // Siguiente: ADC1
            canal = 1;
            estabilizando = 1;
        }
        break;

        case 1: // A1 -> Servo 2 (OC1B, Timer1)
        {
			moverServo(2, ((uint32_t)lectura * 180) / 1023);
            ADMUX = (1 << REFS0) | (1 << MUX1); // Siguiente: ADC2
            canal = 2;
            estabilizando = 1;
        }
        break;

        case 2: // A2 -> Servo 3 (OC2A, Timer2)
        {
			moverServo(3, ((uint32_t)lectura * 180) / 1023);
            ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0); // Siguiente: ADC3
            canal = 3;
            estabilizando = 1;
        }
        break;

        case 3: // A3 -> Servo 4 (OC2B, Timer2)
        {
			moverServo(4, ((uint32_t)lectura * 180) / 1023);
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

    ADCSRA |= (1 << ADSC); // Iniciar siguiente conversión
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
		if (uartIdx > 0)  // Solo procesa si hay algo en el buffer
		{
			uartBuf[uartIdx] = '\0';
			uartListo = 1;
			uartIdx = 0;
		}
		return;
	}
	
	if (uartIdx < UART_BUF_SIZE - 1)
	{
		uartBuf[uartIdx++] = c;
	}
}