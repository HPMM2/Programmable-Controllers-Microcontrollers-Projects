/*************************************************************************
 *   Sistema de Control Lumínico Inteligente para Invernadero            *
 *   Device: ATmega328P                                                  *
 *   Language: C                                                         *
 *   FIME - UANL                                                         *
 *   PIA - Controladores y Microcontroladores Programables               *
 *   Fecha: Noviembre 2025                                               *
 ************************************************************************/

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

//=============== DISPLAY DE 7 SEGMENTOS (MISMO QUE CRONÓMETRO) ===============//
#define DISPLAY_7SEG_AF_DDRX  DDRB
#define DISPLAY_7SEG_AF_PORTX PORTB
#define DISPLAY_7SEG_G_DDRX   DDRC
#define DISPLAY_7SEG_G_PORTX  PORTC
#define DISPLAY_7SEG_SEGA     PORTB0
#define DISPLAY_7SEG_SEGB     PORTB1
#define DISPLAY_7SEG_SEGC     PORTB2
#define DISPLAY_7SEG_SEGD     PORTB3
#define DISPLAY_7SEG_SEGE     PORTB4
#define DISPLAY_7SEG_SEGF     PORTB5
#define DISPLAY_7SEG_SEGG     PORTC0

//=============== BOTONES (MISMO QUE CRONÓMETRO) ===============//
#define BTN_START_DDRX   DDRD
#define BTN_START_PORTX  PORTD
#define BTN_START_PIN    PORTD2  // INT0 - Botón START

#define BTN_HOLD_DDRX    DDRD
#define BTN_HOLD_PORTX   PORTD
#define BTN_HOLD_PIN     PORTD3  // INT1 - Botón HOLD

#define BTN_RESET_DDRX   DDRD
#define BTN_RESET_PORTX  PORTD
#define BTN_RESET_PINX   PIND
#define BTN_RESET_PIN    PORTD4  // Botón RESET (sin interrupción)

//=============== DISPLAY MULTIPLEXADO (MISMO QUE CRONÓMETRO) ===============//
#define DM_DDRX          DDRC
#define DM_PORTX         PORTC
#define DM_DIG1          PORTC1  // Dígito 1
#define DM_DIG2          PORTC2  // Dígito 2
#define DM_DIG3          PORTC3  // Dígito 3
#define DM_DIG4          PORTC4  // Dígito 4

//=============== NUEVOS COMPONENTES ===============//
#define LED_DDRX         DDRD
#define LED_PORTX        PORTD
#define LED_PIN          PORTD6  // Pin D6

#define SENSOR_ADC_CHANNEL  5    // ADC5 (pin A5)

//=============== CONFIGURACIÓN DE FILTRADO ADC ===============//
#define ADC_SAMPLES 32  // Más muestras para mejor suavizado

//=============== ÍNDICES DE DÍGITOS ===============//
#define DIG_1  0
#define DIG_2  1
#define DIG_3  2
#define DIG_4  3

//=============== VARIABLES GLOBALES ===============//

volatile uint8_t display_digits[4] = {0, 0, 0, 0};

uint8_t display_numbers[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

const uint8_t LETTER_H     = 0b01110110;
const uint8_t LETTER_O     = 0b00111111;
const uint8_t LETTER_L     = 0b00111000;
const uint8_t LETTER_A     = 0b01110111;
const uint8_t LETTER_d     = 0b01011110;
const uint8_t LETTER_r     = 0b01010000;
const uint8_t LETTER_y     = 0b01101110;
const uint8_t LETTER_SPACE = 0b00000000;

volatile uint8_t  timer0_multiplex_counter   = 0;
volatile uint16_t timer1_second_counter      = 0;
volatile uint8_t  second_flag                = 0;
volatile uint16_t timer1_half_second_counter = 0;
volatile uint8_t  half_second_flag           = 0;
volatile uint16_t idle_timer_counter         = 0;
volatile uint8_t  idle_3sec_flag             = 0;

volatile uint8_t btn_start_flag = 0;
volatile uint8_t btn_hold_flag  = 0;

volatile uint16_t adc_value         = 0;
volatile uint8_t  adc_ready         = 0;
volatile uint8_t  sensor_value      = 0;
volatile uint8_t  sensor_value_hold = 0;

// Variables para filtrado ADC
volatile uint16_t adc_samples[ADC_SAMPLES];
volatile uint8_t adc_sample_index = 0;
volatile uint8_t adc_samples_ready = 0;

enum states {
    STATE_IDLE,
    STATE_START,
    STATE_HOLD
} state;

volatile uint8_t idle_showing_rdy   = 0;
volatile uint8_t hold_showing_value = 0;

//=============== PROTOTIPOS ===============//
void display_init(void);
void display_show_segment(uint8_t segment_code);
void display_show_number(uint16_t number);
void display_show_text_hola(void);
void display_show_text_rdy(void);
void display_show_text_hold(void);
void display_multiplex(void);

void buttons_init(void);
void check_reset_button(void);

void led_init(void);
void led_set_brightness(uint8_t brightness);

void adc_init(void);
void adc_start_conversion(void);
uint8_t map_adc_to_scale(uint16_t adc_val);
uint16_t read_filtered_adc(void);

void timer0_init(void);
void timer1_init(void);
void timer0_pwm_init(void);

void dm_init(void);

//=============== MAIN ===============//
int main(void)
{
    cli();

    display_init();
    buttons_init();
    dm_init();
    led_init();
    adc_init();
    timer0_init();
    timer1_init();
    timer0_pwm_init();

    // Inicializar arreglo de muestras ADC
    for(uint8_t i = 0; i < ADC_SAMPLES; i++) {
        adc_samples[i] = 0;
    }

    state = STATE_IDLE;
    idle_showing_rdy = 0;
    idle_timer_counter = 0;
    idle_3sec_flag = 0;
    adc_samples_ready = 0;
    adc_sample_index = 0;

    sei();

    led_set_brightness(100);

    while (1)
    {
        check_reset_button();

        switch (state)
        {
            case STATE_IDLE:
                if (idle_showing_rdy == 0)
                {
                    display_show_text_hola();

                    if (idle_3sec_flag == 1)
                    {
                        idle_showing_rdy = 1;
                        idle_3sec_flag = 0;
                    }
                }
                else
                {
                    display_show_text_rdy();
                }

                led_set_brightness(100);

                if (btn_start_flag == 1)
                {
                    btn_start_flag = 0;
                    state = STATE_START;
                    // Reiniciar el filtro ADC
                    adc_samples_ready = 0;
                    adc_sample_index = 0;
                    for(uint8_t i = 0; i < ADC_SAMPLES; i++) {
                        adc_samples[i] = 0;
                    }
                    adc_start_conversion();
                }
                break;

            case STATE_START:
                if (adc_ready == 1)
                {
                    adc_ready = 0;
                    
                    // Almacenar muestra en el filtro
                    adc_samples[adc_sample_index] = adc_value;
                    adc_sample_index++;
                    
                    if (adc_sample_index >= ADC_SAMPLES) {
                        adc_sample_index = 0;
                        adc_samples_ready = 1;
                    }
                    
                    // Solo procesar cuando tenemos suficientes muestras
                    if (adc_samples_ready) {
                        uint16_t filtered_value = read_filtered_adc();
                        sensor_value = map_adc_to_scale(filtered_value);
                        display_show_number(sensor_value);
                        led_set_brightness(sensor_value);
                    }
                    
                    adc_start_conversion();
                }

                if (btn_hold_flag == 1)
                {
                    btn_hold_flag = 0;
                    sensor_value_hold = sensor_value;
                    hold_showing_value = 0;
                    state = STATE_HOLD;
                }
                break;

            case STATE_HOLD:
                if (second_flag == 1)
                {
                    second_flag = 0;

                    if (hold_showing_value == 0)
                    {
                        display_show_number(sensor_value_hold);
                        hold_showing_value = 1;
                    }
                    else
                    {
                        display_show_text_hold();
                        hold_showing_value = 0;
                    }
                }

                if (half_second_flag == 1)
                {
                    half_second_flag = 0;
                    static uint8_t led_state = 0;

                    if (led_state == 0)
                    {
                        led_set_brightness(100);
                        led_state = 1;
                    }
                    else
                    {
                        led_set_brightness(0);
                        led_state = 0;
                    }
                }

                if (btn_start_flag == 1)
                {
                    btn_start_flag = 0;
                    sensor_value = sensor_value_hold;
                    display_show_number(sensor_value_hold);
                    led_set_brightness(sensor_value_hold);
                    state = STATE_START;
                    // Reiniciar filtro ADC
                    adc_samples_ready = 0;
                    adc_sample_index = 0;
                    for(uint8_t i = 0; i < ADC_SAMPLES; i++) {
                        adc_samples[i] = 0;
                    }
                    adc_start_conversion();
                }
                break;
        }
    }

    return 0;
}

//=============== IMPLEMENTACIÓN ===============//

void display_init(void)
{
    DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGA) | (1<<DISPLAY_7SEG_SEGB) |
                             (1<<DISPLAY_7SEG_SEGC) | (1<<DISPLAY_7SEG_SEGD) |
                             (1<<DISPLAY_7SEG_SEGE) | (1<<DISPLAY_7SEG_SEGF);
    DISPLAY_7SEG_G_DDRX |= (1<<DISPLAY_7SEG_SEGG);
}

void dm_init(void)
{
    DM_DDRX |= (1<<DM_DIG1) | (1<<DM_DIG2) | (1<<DM_DIG3) | (1<<DM_DIG4);
}

void display_show_segment(uint8_t segment_code)
{
    DISPLAY_7SEG_AF_PORTX = (segment_code & 0b00111111) | 
                            (DISPLAY_7SEG_AF_PORTX & 0b11000000);
    DISPLAY_7SEG_G_PORTX = ((segment_code & 0b01000000) >> 6) | 
                           (DISPLAY_7SEG_G_PORTX & 0b11111110);
}

void display_show_number(uint16_t number)
{
    if (number > 9999) number = 9999;

    display_digits[DIG_1] = (number / 1000) % 10;
    display_digits[DIG_2] = (number / 100) % 10;
    display_digits[DIG_3] = (number / 10) % 10;
    display_digits[DIG_4] = number % 10;
}

void display_show_text_hola(void)
{
    display_digits[DIG_1] = 10;
    display_digits[DIG_2] = 11;
    display_digits[DIG_3] = 12;
    display_digits[DIG_4] = 13;
}

void display_show_text_rdy(void)
{
    display_digits[DIG_1] = 14;
    display_digits[DIG_2] = 15;
    display_digits[DIG_3] = 16;
    display_digits[DIG_4] = 17;
}

void display_show_text_hold(void)
{
    display_digits[DIG_1] = 10;
    display_digits[DIG_2] = 11;
    display_digits[DIG_3] = 12;
    display_digits[DIG_4] = 16;
}

void display_multiplex(void)
{
    uint8_t segment_code;
    uint8_t digit_value = display_digits[timer0_multiplex_counter];

    if (digit_value < 10)
    {
        segment_code = display_numbers[digit_value];
    }
    else
    {
        switch(digit_value)
        {
            case 10: segment_code = LETTER_H; break;
            case 11: segment_code = LETTER_O; break;
            case 12: segment_code = LETTER_L; break;
            case 13: segment_code = LETTER_A; break;
            case 14: segment_code = LETTER_SPACE; break;
            case 15: segment_code = LETTER_r; break;
            case 16: segment_code = LETTER_d; break;
            case 17: segment_code = LETTER_y; break;
            default: segment_code = 0; break;
        }
    }

    switch (timer0_multiplex_counter)
    {
        case DIG_1:
            DM_PORTX = (1<<DM_DIG1) | (DM_PORTX & 0b11100001);
            display_show_segment(segment_code);
            break;
        case DIG_2:
            DM_PORTX = (1<<DM_DIG2) | (DM_PORTX & 0b11100001);
            display_show_segment(segment_code);
            break;
        case DIG_3:
            DM_PORTX = (1<<DM_DIG3) | (DM_PORTX & 0b11100001);
            display_show_segment(segment_code);
            break;
        case DIG_4:
            DM_PORTX = (1<<DM_DIG4) | (DM_PORTX & 0b11100001);
            display_show_segment(segment_code);
            break;
    }
}

void buttons_init(void)
{
    BTN_START_DDRX &= ~(1<<BTN_START_PIN);
    BTN_START_PORTX |= (1<<BTN_START_PIN);

    BTN_HOLD_DDRX &= ~(1<<BTN_HOLD_PIN);
    BTN_HOLD_PORTX |= (1<<BTN_HOLD_PIN);

    BTN_RESET_DDRX &= ~(1<<BTN_RESET_PIN);
    BTN_RESET_PORTX |= (1<<BTN_RESET_PIN);

    EICRA |= (1<<ISC01) | (1<<ISC00);
    EICRA |= (1<<ISC11) | (1<<ISC10);

    EIMSK |= (1<<INT0) | (1<<INT1);
}

void check_reset_button(void)
{
    if ((BTN_RESET_PINX & (1<<BTN_RESET_PIN)) == 0)
    {
        _delay_ms(50);
        if ((BTN_RESET_PINX & (1<<BTN_RESET_PIN)) == 0)
        {
            state = STATE_IDLE;
            idle_showing_rdy = 0;
            idle_timer_counter = 0;
            idle_3sec_flag = 0;
            led_set_brightness(100);

            while ((BTN_RESET_PINX & (1<<BTN_RESET_PIN)) == 0);
        }
    }
}

void led_init(void)
{
    LED_DDRX |= (1<<LED_PIN);
}

void led_set_brightness(uint8_t brightness)
{
    if (brightness > 50)
        LED_PORTX |= (1<<LED_PIN);
    else
        LED_PORTX &= ~(1<<LED_PIN);
}

void adc_init(void)
{
    ADMUX = (1<<REFS0) | (SENSOR_ADC_CHANNEL & 0x0F);
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    ADCSRA |= (1<<ADIE);
}

void adc_start_conversion(void)
{
    ADCSRA |= (1<<ADSC);
}

uint16_t read_filtered_adc(void)
{
    uint32_t sum = 0;
    
    for(uint8_t i = 0; i < ADC_SAMPLES; i++) {
        sum += adc_samples[i];
    }
    
    return (uint16_t)(sum / ADC_SAMPLES);
}

uint8_t map_adc_to_scale(uint16_t adc_val)
{
    // CALIBRACIÓN PARA LDR CON RESISTOR DE 220?
    // Ajustado para que llegue exactamente a 100
    
    // Valores típicos con resistor de 220? - AJUSTAR SEGÚN TUS LECTURAS
    const uint16_t ADC_MIN = 50;      // Oscuridad total
    const uint16_t ADC_MAX = 550;     // Reducido para que llegue a 100 (ajustar según pruebas)
    
    // Si está por debajo del mínimo, retornar 0
    if (adc_val <= ADC_MIN)
        return 0;
    
    // Si está por encima del máximo, retornar 100
    if (adc_val >= ADC_MAX)
        return 100;
    
    // Mapear linealmente del rango completo a 0-100
    uint32_t rango = ADC_MAX - ADC_MIN;
    uint32_t diferencia = adc_val - ADC_MIN;
    uint32_t resultado = (diferencia * 100) / rango;
    
    // Asegurarnos de que esté en el rango 0-100
    if (resultado > 100) resultado = 100;
    
    return (uint8_t)resultado;
}

void timer0_init(void)
{
    TCCR0A = (1<<WGM01);
    TCCR0B = 0;

    OCR0A = 156;

    TIMSK0 = (1<<OCIE0A);

    TCCR0B = (1<<CS02) | (1<<CS00);
}

void timer0_pwm_init(void)
{
    // Vacío por ahora
}

void timer1_init(void)
{
    TCCR1A = 0;
    TCCR1B = (1<<WGM12);

    OCR1A = 2500;

    TIMSK1 = (1<<OCIE1A);

    TCCR1B |= (1<<CS11) | (1<<CS10);
}

ISR(TIMER0_COMPA_vect)
{
    display_multiplex();

    timer0_multiplex_counter++;
    if (timer0_multiplex_counter >= 4)
    {
        timer0_multiplex_counter = 0;
    }
}

ISR(TIMER1_COMPA_vect)
{
    timer1_second_counter++;
    if (timer1_second_counter >= 100)
    {
        timer1_second_counter = 0;
        second_flag = 1;
    }

    timer1_half_second_counter++;
    if (timer1_half_second_counter >= 50)
    {
        timer1_half_second_counter = 0;
        half_second_flag = 1;
    }

    if (state == STATE_IDLE && idle_showing_rdy == 0)
    {
        idle_timer_counter++;
        if (idle_timer_counter >= 300)
        {
            idle_timer_counter = 0;
            idle_3sec_flag = 1;
        }
    }
}

ISR(ADC_vect)
{
    adc_value = ADC;
    adc_ready = 1;
}

ISR(INT0_vect)
{
    _delay_ms(50);
    btn_start_flag = 1;
}

ISR(INT1_vect)
{
    _delay_ms(50);
    btn_hold_flag = 1;
}