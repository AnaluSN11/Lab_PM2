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
#include "PWM/PWM.h"
#include "UART/UART.h"
#include "ADC/ADC.h"

/****************************************/
/* Definiciones                         */
/****************************************/
#define MODO_MANUAL     0
#define MODO_ADAFRUIT   1

// LED de modo: PB0
#define LED_MODO_DDR    DDRB
#define LED_MODO_PORT   PORTB
#define LED_MODO_PIN    DDB0

// Mapeo OCR1 (Timer1, ICR1=4999, prescaler 64 -> 50Hz exacto)
#define SERVO_MIN_T1    250
#define SERVO_MAX_T1    700

// Mapeo OCR2 (Timer2, valores verificados en hardware)
#define SERVO_MIN_T2    12
#define SERVO_MAX_T2    37

/****************************************/
/* Variables globales                   */
/****************************************/
volatile uint8_t receivedChar   = 0;
volatile uint8_t dataReady      = 0;
volatile uint8_t modo_actual    = MODO_MANUAL;

/****************************************/
/* Prototipos                           */
/****************************************/
void setup(void);
void writeString(const char *str);
void actualizarModo(void);
void modoAdafruit(void);

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
    initTimer2(fastPWM, 1024);
    initPWM2A(no_invertir);   // PB3 -> Servo 3
    initPWM2B(no_invertir);   // PD3 -> Servo 4

    // ADC 
    initADC();
    ADCSRA |= (1 << ADSC);

    // UART
    initUART(async, disabled_parity, one_stop_bit, 8);

    sei();
    _delay_ms(100);
    writeString("Modo: MANUAL\r\n");

    while (1)
    {
        actualizarModo();

        if (modo_actual == MODO_ADAFRUIT)
        {
            modoAdafruit();
        }
    }
}

/****************************************/
/* Setup                                */
/****************************************/
void setup(void)
{
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    DDRC &= ~((1 << DDC0) | (1 << DDC1) | (1 << DDC2) | (1 << DDC3));

    LED_MODO_DDR  |=  (1 << LED_MODO_PIN);
    LED_MODO_PORT &= ~(1 << LED_MODO_PIN);
}

/****************************************/
/* actualizarModo                       */
/****************************************/
void actualizarModo(void)
{
    if (!dataReady) return;
    dataReady = 0;

    if (receivedChar == 'M')
    {
        if (modo_actual == MODO_MANUAL)
        {
            modo_actual = MODO_ADAFRUIT;
            LED_MODO_PORT |=  (1 << LED_MODO_PIN);
            writeString("Modo: ADAFRUIT\r\n");
        }
        else
        {
            modo_actual = MODO_MANUAL;
            LED_MODO_PORT &= ~(1 << LED_MODO_PIN);
            writeString("Modo: MANUAL\r\n");
            ADMUX  = (1 << REFS0); // Reiniciar en canal 0
            ADCSRA |= (1 << ADSC);
        }
    }
}

/****************************************/
/* modoAdafruit                         */
/****************************************/
void modoAdafruit(void)
{
    // Por desarrollar.
}

/****************************************/
/* writeString                          */
/****************************************/
void writeString(const char *str)
{
    while (*str) writeChar(*str++);
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

    if (modo_actual != MODO_MANUAL)  return;

    if (estabilizando)
    {
        estabilizando = 0;          // Descartar esta lectura, la siguiente es válida
        ADCSRA |= (1 << ADSC);
        return;
    }

    switch (canal)
    {
        case 0: // A0 -> Servo 1 (OC1A, Timer1)
        {
            uint16_t ocr = SERVO_MIN_T1 + ((uint32_t)lectura * (SERVO_MAX_T1 - SERVO_MIN_T1)) / 1023;
            updateDutyCycle1A(ocr);
            ADMUX = (1 << REFS0) | (1 << MUX0); // Siguiente: ADC1
            canal = 1;
            estabilizando = 1;
        }
        break;

        case 1: // A1 -> Servo 2 (OC1B, Timer1)
        {
            uint16_t ocr = SERVO_MIN_T1 + ((uint32_t)lectura * (SERVO_MAX_T1 - SERVO_MIN_T1)) / 1023;
            updateDutyCycle1B(ocr);
            ADMUX = (1 << REFS0) | (1 << MUX1); // Siguiente: ADC2
            canal = 2;
            estabilizando = 1;
        }
        break;

        case 2: // A2 -> Servo 3 (OC2A, Timer2)
        {
            uint8_t ocr = SERVO_MIN_T2 + ((uint32_t)lectura * (SERVO_MAX_T2 - SERVO_MIN_T2)) / 1023;
            updateDutyCycle2A(ocr);
            ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0); // Siguiente: ADC3
            canal = 3;
            estabilizando = 1;
        }
        break;

        case 3: // A3 -> Servo 4 (OC2B, Timer2)
        {
            uint8_t ocr = SERVO_MIN_T2 + ((uint32_t)lectura * (SERVO_MAX_T2 - SERVO_MIN_T2)) / 1023;
            updateDutyCycle2B(ocr);
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
/* ISR: UART RX                         */
/****************************************/
ISR(USART_RX_vect)
{
    receivedChar = UDR0;
    writeChar(receivedChar);
    dataReady = 1;
} 
