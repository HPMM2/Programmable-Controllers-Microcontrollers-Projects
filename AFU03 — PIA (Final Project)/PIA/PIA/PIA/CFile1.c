/************************************************************************* 
 *   Copyright (C) 2025 by Equipo 4                                      *
 *   Heidi Pamela Martínez Martínez    1952947   #6                      *
 *   Steven Neftalí Verdugo Figueroa   2132366   #60                     *
 *   Ángel Armando Solís Jiménez       2132088   #55                     *    
 *   Carlos Emiliano Mendoza Ramírez   2048186   #35                     *
 *                                                                       *
 *   Attribution-NonCommercial-ShareAlike 3.0(CC-BY-NC-SA 3.0)           *
 *   AFU03 - Producto Integrador de aprendizaje (PIA)                    *
 *   Dispositivo: atmega328p                                             *
 *   Lenguaje C                                                          *
 *   Rev: 1.1                                                            *
 *   Fecha: 25/11/25                                                     *
 ************************************************************************/

#include <avr/io.h>
#define F_CPU 16000000UL      // Frecuencia de reloj del microcontrolador 16 MHz
#include <util/delay.h>
#include <avr/interrupt.h>

//=============== DISPLAY DE 7 SEGMENTOS ===============//
// Configuración de los puertos para los segmentos A-F del display 7 segmentos
#define DISPLAY_7SEG_AF_DDRX    DDRB      // Dirección: puerto B para segmentos A-F
#define DISPLAY_7SEG_AF_PORTX   PORTB     // Puerto de salida para segmentos A-F
#define DISPLAY_7SEG_G_DDRX     DDRC      // Dirección: puerto C para segmento G
#define DISPLAY_7SEG_G_PORTX    PORTC     // Puerto de salida para segmento G
#define DISPLAY_7SEG_SEGG       PORTC0    // Segmento G en PC0

//=============== BOTONES ===============//
#define BTN_START_PIN           PORTD2    // Botón START en D2
#define BTN_HOLD_PIN            PORTD3    // Botón HOLD en D3

//=============== DISPLAY MULTIPLEXADO ===============//
// Configuración para el multiplexado de 4 dígitos en el display
#define DM_DDRX                 DDRC
#define DM_PORTX                PORTC
#define DM_DIG1                 PORTC1    // Dígito 1
#define DM_DIG2                 PORTC2    // Dígito 2
#define DM_DIG3                 PORTC3    // Dígito 3
#define DM_DIG4                 PORTC4    // Dígito 4

//=============== LED Y SENSOR ===============//
#define LED_DDRX                DDRD      // Dirección del LED
#define LED_PORTX               PORTD     // Puerto del LED
#define LED_PIN                 PORTD5    // LED en D5

#define SENSOR_ADC_CHANNEL      5         // Sensor de luz en A5 (canal 5 del ADC)

//=============== VARIABLES GLOBALES ===============//
// Array para almacenar los 4 dígitos a mostrar en el display
volatile uint8_t display_digits[4] = {0, 0, 0, 0};

// Patrones binarios para mostrar números 0-9 en el display 7 segmentos
uint8_t display_numbers[10] = {
    0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
    0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111
};

// Patrones binarios para mostrar letras en el display 7 segmentos
const uint8_t LETTER_H     = 0b01110110;  // Letra H
const uint8_t LETTER_O     = 0b00111111;  // Letra O
const uint8_t LETTER_L     = 0b00111000;  // Letra L
const uint8_t LETTER_A     = 0b01110111;  // Letra A
const uint8_t LETTER_d     = 0b01011110;  // Letra d
const uint8_t LETTER_r     = 0b01010000;  // Letra r
const uint8_t LETTER_y     = 0b01101110;  // Letra y
const uint8_t LETTER_SPACE = 0b00000000;  // Espacio en blanco

// Variables para los timers
volatile uint8_t  timer0_multiplex = 0;   // Contador para multiplexado del display
volatile uint16_t timer1_1sec      = 0;   // Contador para bandera de 1 segundo
volatile uint8_t  flag_1sec        = 0;   // Bandera que se activa cada 1 segundo
volatile uint16_t timer1_3sec      = 0;   // Contador para bandera de 3 segundos
volatile uint8_t  flag_3sec        = 0;   // Bandera que se activa cada 3 segundos

// Variables para los botones
volatile uint8_t btn_start = 0;           // Bandera del botón START
volatile uint8_t btn_hold  = 0;           // Bandera del botón HOLD

// Variables para debounce (anti-rebote de botones)
volatile uint8_t hold_debounce_count  = 0;    // Contador debounce para HOLD
volatile uint8_t hold_pressed         = 0;    // Estado presionado de HOLD
volatile uint8_t start_debounce_count = 0;    // Contador debounce para START
volatile uint8_t start_pressed        = 0;    // Estado presionado de START

// Variables del ADC (sensor de luz)
volatile uint16_t adc_value    = 0;       // Valor bruto del ADC
volatile uint8_t  adc_ready    = 0;       // Bandera de conversión completada
volatile uint8_t  sensor_value = 0;       // Valor mapeado (0-100) del sensor
volatile uint8_t  sensor_hold  = 0;       // Valor guardado en HOLD

// Buffer para promediar lecturas del ADC (filtra ruido)
// Se inicializa con 1 para evitar división por cero
volatile uint16_t adc_buffer[8] = {1, 1, 1, 1, 1, 1, 1, 1};
volatile uint8_t adc_buffer_idx = 0;      // Índice actual del buffer

// Estados del sistema
enum states { STATE_IDLE, STATE_START, STATE_HOLD };
volatile enum states state;               // Estado actual del sistema

volatile uint8_t idle_show_rdy = 0;       // Bandera para mostrar "rdy" en IDLE
volatile uint8_t hold_show_val = 1;       // Bandera para alternar visualización en HOLD

//=============== PROTOTIPOS ===============//
void system_init(void);                   // Inicializa todos los periféricos
void display_show_segment(uint8_t code);  // Muestra un patrón en el display
void display_show_number(uint16_t num);   // Muestra un número en el display
void display_show_hola(void);             // Muestra "HOLA" en el display
void display_show_rdy(void);              // Muestra "rdy" en el display
void display_show_hold(void);             // Muestra "HOLD" en el display
void display_multiplex(void);             // Multiplexado del display
void led_set_brightness(uint8_t bright);  // Controla el brillo del LED
uint8_t map_sensor(uint16_t adc);         // Mapea valor ADC a 0-100
uint16_t get_adc_average(void);           // Calcula promedio del buffer

//=============== MAIN ===============//
int main(void)
{
    cli();                // Desabilita interrupciones globales
    system_init();        // Inicializa periféricos

    // Inicializa todas las variables
    state          = STATE_IDLE;
    idle_show_rdy  = 0;
    timer1_3sec    = 0;
    flag_3sec      = 0;
    timer1_1sec    = 0;
    flag_1sec      = 0;
    btn_start      = 0;
    btn_hold       = 0;
    hold_debounce_count  = 0;
    hold_pressed         = 0;
    start_debounce_count = 0;
    start_pressed        = 0;
    hold_show_val        = 1;

    display_show_hola();              // Muestra "HOLA" en el display
    LED_PORTX |= (1<<LED_PIN);        // Enciende el LED al iniciar

    sei();                // Habilita interrupciones globales

    _delay_ms(200);       // Pequeño retardo de inicialización

    while (1)             // Loop principal
    {
        switch (state)
        {
            case STATE_IDLE:
                // Estado de reposo/inicio
                if (idle_show_rdy == 0)
                {
                    display_show_hola();

                    // Después de 3 segundos, cambia a mostrar "rdy"
                    if (flag_3sec == 1)
                    {
                        idle_show_rdy = 1;
                        flag_3sec     = 0;
                        timer1_3sec   = 0;
                        display_show_rdy();
                    }
                }
                else
                {
                    display_show_rdy();
                }

                LED_PORTX |= (1<<LED_PIN);  // LED encendido en IDLE

                // Si presionan START, inicia la lectura del sensor
                if (btn_start == 1)
                {
                    btn_start = 0;
                    state = STATE_START;
                    ADCSRA |= (1<<ADSC);    // Inicia conversión ADC
                }
                break;

            case STATE_START:
                // Estado de lectura del sensor
                if (adc_ready == 1)
                {
                    adc_ready    = 0;
                    
                    // Obtiene promedio de 8 lecturas (filtra ruido)
                    uint16_t adc_avg = get_adc_average();
                    
                    // Mapea el valor promedio a 0-100
                    sensor_value = map_sensor(adc_avg);
                    
                    // Muestra el valor en el display (0-100)
                    display_show_number(sensor_value);
                    
                    // Controla el LED según el valor del sensor
                    led_set_brightness(sensor_value);
                    
                    // Inicia nueva conversión ADC
                    ADCSRA |= (1<<ADSC);
                }

                // Si presionan HOLD, guarda el valor actual
                if (btn_hold == 1)
                {
                    btn_hold      = 0;
                    sensor_hold   = sensor_value;  // Guarda el valor del sensor
                    hold_show_val = 1;
                    timer1_1sec   = 0;
                    flag_1sec     = 0;

                    display_show_number(sensor_hold);
                    LED_PORTX |= (1<<LED_PIN);  // Enciende LED al entrar a HOLD
                    state = STATE_HOLD;         // Entra a estado HOLD
                }
                break;

            case STATE_HOLD:
                // Estado de pausa - el LED parpadea cada segundo
                if (btn_start == 1)
                {
                    // Si presionan START, regresa a leer el sensor
                    btn_start = 0;
                    
                    timer1_1sec   = 0;
                    flag_1sec     = 0;
                    hold_show_val = 1;
                    
                    state = STATE_START;
                    ADCSRA |= (1<<ADSC);
                    break;
                }

                // Cada 1 segundo, alterna la visualización del LED
                if (flag_1sec == 1)
                {
                    flag_1sec = 0;
                    hold_show_val ^= 1;  // Alterna entre 0 y 1

                    if (hold_show_val)
                    {
                        // Muestra el valor y enciende el LED (siempre, sin importar el valor)
                        display_show_number(sensor_hold);
                        LED_PORTX |= (1<<LED_PIN);
                    }
                    else
                    {
                        // Muestra "HOLD" y apaga el LED (efecto parpadeo)
                        display_show_hold();
                        LED_PORTX &= ~(1<<LED_PIN);
                    }
                }
                break;
        }
    }
}

//=============== IMPLEMENTACIÓN ===============//

void system_init(void)
{
    // Configura el display de 7 segmentos como salidas
    DISPLAY_7SEG_AF_DDRX |= 0x3F;        // Segmentos A-F en puerto B
    DISPLAY_7SEG_G_DDRX  |= (1<<DISPLAY_7SEG_SEGG);  // Segmento G en puerto C

    // Configura los dígitos multiplexados como salidas
    DM_DDRX |= (1<<DM_DIG1) | (1<<DM_DIG2) | (1<<DM_DIG3) | (1<<DM_DIG4);

    // Configura los botones como entradas con pull-up interno
    DDRD  &= ~(1<<BTN_START_PIN);  // D2 como entrada
    PORTD |=  (1<<BTN_START_PIN);  // Habilita pull-up en D2

    DDRD  &= ~(1<<BTN_HOLD_PIN);   // D3 como entrada
    PORTD |=  (1<<BTN_HOLD_PIN);   // Habilita pull-up en D3

    // Configura el LED como salida
    LED_DDRX |= (1<<LED_PIN);      // D5 como salida
    LED_PORTX |= (1<<LED_PIN);     // LED encendido desde el inicio

    // Configura el ADC (conversor analógico-digital)
    ADMUX  = (1<<REFS0) | (SENSOR_ADC_CHANNEL & 0x0F);  // Referencia AVCC, canal 5
    ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);  // Habilita ADC con interrupción

    // Configura Timer0: para multiplexado del display (velocidad ~1 kHz)
    TCCR0A = (1<<WGM01);    // Modo CTC
    OCR0A  = 156;           // Comparador para frecuencia
    TIMSK0 = (1<<OCIE0A);   // Habilita interrupción de comparación
    TCCR0B = (1<<CS02)|(1<<CS00);  // Prescaler 1024

    // Configura Timer1: base de 10ms para generar flags de 1s y 3s
    TCCR1B = (1<<WGM12)|(1<<CS11)|(1<<CS10);  // Modo CTC, prescaler 64
    OCR1A  = 2500;          // Genera interrupción cada 10ms
    TIMSK1 = (1<<OCIE1A);   // Habilita interrupción de comparación
}

void display_show_segment(uint8_t code)
{
    // Muestra un patrón de 7 segmentos en el display
    // Los primeros 6 bits van al puerto B (segmentos A-F)
    // El bit 6 va al puerto C (segmento G)
    DISPLAY_7SEG_AF_PORTX = (code & 0x3F) | (DISPLAY_7SEG_AF_PORTX & 0xC0);
    DISPLAY_7SEG_G_PORTX  = ((code>>6) & 0x01) | (DISPLAY_7SEG_G_PORTX & 0xFE);
}

void display_show_number(uint16_t num)
{
    // Convierte un número a sus 4 dígitos individuales
    if (num > 9999) num = 9999;                    // Limita a 4 dígitos
    display_digits[0] = (num / 1000) % 10;         // Miles
    display_digits[1] = (num / 100)  % 10;         // Centenas
    display_digits[2] = (num / 10)   % 10;         // Decenas
    display_digits[3] =  num % 10;                 // Unidades
}

void display_show_hola(void)
{
    // Muestra la palabra "HOLA" en el display (saludo inicial)
    display_digits[0] = 10; // H
    display_digits[1] = 11; // O
    display_digits[2] = 12; // L
    display_digits[3] = 13; // A
}

void display_show_rdy(void)
{
    // Muestra "rdy" en el display (listo/ready)
    display_digits[0] = 14; // espacio
    display_digits[1] = 15; // r
    display_digits[2] = 16; // d
    display_digits[3] = 17; // y
}

void display_show_hold(void)
{
    // Muestra "HOLD" en el display (pausa)
    display_digits[0] = 10; // H
    display_digits[1] = 11; // O
    display_digits[2] = 12; // L
    display_digits[3] = 16; // d
}

void display_multiplex(void)
{
    // Función llamada por Timer0 para mostrar cada dígito secuencialmente
    // Esto crea el efecto de ver los 4 dígitos simultáneamente
    
    uint8_t code;
    uint8_t val = display_digits[timer0_multiplex];  // Obtiene el dígito actual

    // Busca el patrón binario para el dígito o letra
    if (val < 10)
        code = display_numbers[val];  // Es un número
    else
    {
        // Es una letra o símbolo especial
        switch(val)
        {
            case 10: code = LETTER_H;       break;
            case 11: code = LETTER_O;       break;
            case 12: code = LETTER_L;       break;
            case 13: code = LETTER_A;       break;
            case 14: code = LETTER_SPACE;   break;
            case 15: code = LETTER_r;       break;
            case 16: code = LETTER_d;       break;
            case 17: code = LETTER_y;       break;
            default: code = 0;              break;
        }
    }

    // Apaga todos los dígitos
    DM_PORTX &= 0xE1;

    // Enciende el dígito actual
    switch (timer0_multiplex)
    {
        case 0: DM_PORTX |= (1<<DM_DIG1); break;
        case 1: DM_PORTX |= (1<<DM_DIG2); break;
        case 2: DM_PORTX |= (1<<DM_DIG3); break;
        case 3: DM_PORTX |= (1<<DM_DIG4); break;
    }

    // Muestra el patrón en el display
    display_show_segment(code);
}

void led_set_brightness(uint8_t bright)
{
    // Controla el LED basado en el valor del sensor
    // Valores mayores a 50 encienden el LED (mucha luz detectada)
    if (bright > 50)
        LED_PORTX |= (1<<LED_PIN);   // LED encendido
    else
        LED_PORTX &= ~(1<<LED_PIN);  // LED apagado
}

// Obtiene promedio de 8 lecturas del ADC para filtrar ruido
uint16_t get_adc_average(void)
{
    uint32_t suma = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        suma += adc_buffer[i];  // Suma todos los valores del buffer
    }
    return (uint16_t)(suma / 8);  // Retorna el promedio
}

uint8_t map_sensor(uint16_t adc)
{
    // Mapea el valor bruto del ADC (1-13) a un rango de 0-100
    // Rango mínimo: 1 = máxima luz (retorna 100)
    // Rango máximo: 13 = oscuridad total (retorna 0)
    
    const uint16_t ADC_MIN = 1;    // Valor con mucha luz
    const uint16_t ADC_MAX = 13;   // Valor sin luz (oscuro)
    
    // Protección contra valores fuera de rango
    if (adc <= ADC_MIN) return 100;  // Mucha luz = valor máximo
    if (adc >= ADC_MAX) return 0;    // Oscuro = valor mínimo

    // Mapeo INVERTIDO: valores bajos de ADC dan valores altos (más luz)
    uint32_t range  = ADC_MAX - ADC_MIN;
    uint32_t diff   = ADC_MAX - adc;
    uint32_t result = (diff * 100) / range;

    return (result > 100) ? 100 : (uint8_t)result;
}

//=============== ISRs (Rutinas de Interrupción) ===============//

ISR(TIMER0_COMPA_vect)
{
    // Interrupción del Timer0: se ejecuta ~1000 veces por segundo
    // Controla el multiplexado del display
    
    display_multiplex();                    // Muestra el dígito actual
    timer0_multiplex++;                     // Avanza al siguiente dígito
    if (timer0_multiplex >= 4)              // Si completó los 4 dígitos
        timer0_multiplex = 0;               // Vuelve al primero
}

ISR(TIMER1_COMPA_vect)
{
    // Interrupción del Timer1: se ejecuta cada 10ms (base de tiempo)
    
    // Contador de 1 segundo (100 interrupciones × 10ms = 1 segundo)
    timer1_1sec++;
    if (timer1_1sec >= 100)
    {
        timer1_1sec = 0;
        flag_1sec = 1;  // Activa bandera de 1 segundo
    }

    // Contador de 3 segundos solo cuando está en IDLE y no muestra "rdy"
    if (state == STATE_IDLE && idle_show_rdy == 0)
    {
        timer1_3sec++;
        if (timer1_3sec >= 300)  // 300 × 10ms = 3 segundos
        {
            flag_3sec = 1;  // Activa bandera de 3 segundos
        }
    }
    else
    {
        // Reinicia contadores si no cumple condición
        timer1_3sec = 0;
        flag_3sec   = 0;
    }

    // ========== DEBOUNCE BOTÓN START (D2) ==========
    // Lee el estado del botón y implementa anti-rebote
    uint8_t start_pin_state = (PIND & (1 << BTN_START_PIN)) == 0;  // 0 = presionado

    if (start_pin_state)  // Botón presionado (LOW)
    {
        // Incrementa contador de debounce
        if (start_debounce_count < 10)
            start_debounce_count++;

        // Después de 5 interrupciones (50ms de presión), confirma
        if (start_debounce_count >= 5 && !start_pressed)
        {
            start_pressed = 1;  // Marca como presionado
            btn_start     = 1;  // Activa bandera
        }
    }
    else  // Botón suelto (HIGH)
    {
        // Reinicia contadores cuando se suelta el botón
        start_debounce_count = 0;
        start_pressed = 0;
    }

    // ========== DEBOUNCE BOTÓN HOLD (D3) ==========
    // Lee el estado del botón y implementa anti-rebote
    uint8_t hold_pin_state = (PIND & (1 << BTN_HOLD_PIN)) == 0;  // 0 = presionado

    if (hold_pin_state)  // Botón presionado (LOW)
    {
        // Incrementa contador de debounce
        if (hold_debounce_count < 10)
            hold_debounce_count++;

        // Después de 5 interrupciones (50ms de presión), confirma
        if (hold_debounce_count >= 5 && !hold_pressed)
        {
            hold_pressed = 1;  // Marca como presionado
            btn_hold     = 1;  // Activa bandera
        }
    }
    else  // Botón suelto (HIGH)
    {
        // Reinicia contadores cuando se suelta el botón
        hold_debounce_count = 0;
        hold_pressed = 0;
    }
}

ISR(ADC_vect)
{
    // Interrupción del conversor ADC: se ejecuta al completar una conversión
    
    // Lee el valor del ADC (10 bits, rango 0-1023)
    uint16_t val = ADC;
    
    // Solo guarda valores válidos (descarta 0 que indica contacto intermitente)
    if (val > 0)
    {
        // Almacena el valor en el buffer circular
        adc_buffer[adc_buffer_idx] = val;
        
        // Avanza al siguiente índice del buffer (circular)
        adc_buffer_idx = (adc_buffer_idx + 1) % 8;
    }
    
    // Activa bandera indicando que hay nueva lectura disponible
    adc_ready = 1;
}