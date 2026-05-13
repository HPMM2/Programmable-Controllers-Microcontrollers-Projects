/*************************************************************************
 *   Copyright (C) 2025 by Equipo 4 					                 *
 *   Heidi Pamela Martínez Martínez	           1952947	#6		         *
 *   Steven Neftalí Verdugo Figueroa	       2132366	#60		         *
 *   Ángel Armando Solís Jiménez	           2132088	#55		         *	
 *   Carlos Emiliano Mendoza Ramírez	       2048186	#35	             *
 *										                                 *
 *   Attribution-NonCommercial-ShareAlike 3.0(CC-BY-NC-SA 3.0)           *
 *   AFU05 - Estructuras de programación                                 *
 *   Dispositivo: atmega328p                                             *
 *   Lenguaje C                                                          *
 *   Rev: 1.0                                                            *
 *   Licencia:                                                           *
 *                                                     Fecha: 12/11/25   *
 ************************************************************************/


#include <avr/io.h>
#define F_CPU 16000000UL //16 Mhz
#include <util/delay.h>
#include <avr/interrupt.h>

//Display de 7 segmentos
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

//Botones con interrupciones externas
#define INT0_DDRX  DDRD
#define INT0_PORTX PORTD
#define INT0_BTN0  PORTD2  // Botón START/STOP

#define INT1_DDRX  DDRD
#define INT1_PORTX PORTD
#define INT1_BTN1  PORTD3  // Botón CONFIGURAR

//Display Multiplexado
#define DM_DDRX     DDRC
#define DM_PORTX    PORTC
#define DM_MILLARES PORTC1
#define DM_CENTENAS PORTC2
#define DM_DECENAS  PORTC3
#define DM_UNIDADES PORTC4

//Buzzer
#define BUZZER_DDRX  DDRD
#define BUZZER_PORTX PORTD
#define BUZZER_PIN   PORTD4

//Índices de dígitos
#define MIN_DEC  0
#define MIN_UNI  1
#define SEG_DEC  2
#define SEG_UNI  3

// Arrays de dígitos y tiempo
volatile uint8_t timedigits[4];
volatile uint8_t minutos = 59;
volatile uint8_t segundos = 59;

// Array de números para display de 7 segmentos
uint8_t display_numbers_array[10] =
{   //Xgfedcba
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

// Variables de control
volatile uint8_t timer0_barrido_flag = 0;  // Control de multiplexación
volatile uint16_t timer0_1seg_counter = 0;  // Contador para 1 segundo
volatile uint8_t int0_debounce_flag = 0;    // Antirrebote INT0
volatile uint8_t int1_debounce_flag = 0;    // Antirrebote INT1
volatile uint16_t buzzer_counter = 0;       // Contador para buzzer
volatile uint8_t buzzer_active = 0;         // Estado del buzzer

// Estados de la máquina de estados
enum states {
	CONFIG_INICIAL,      // Estado inicial
	CONFIG_SEGUNDOS,     // Configurando segundos
	CONFIG_MINUTOS,      // Configurando minutos
	LISTO,              // Esperando a presionar start
	CORRIENDO,          // Cronómetro corriendo
	ALARMA              // Tiempo terminado, sonando alarma
} state;

// Prototipos de funciones
void display_init(void);
void display_show_number_array(uint8_t number);
void int0_init(void);
void int1_init(void);
void dm_init(void);
void dm_show_time(uint8_t mins, uint8_t segs);
void timer0_init(void);
void timer0_run(void);
void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void update_time_digits(void);

///-----MAIN-------------------/////
int main(void)
{           
	cli(); // Deshabilitar interrupciones
	
	// Inicialización
	display_init();
	int0_init();
	int1_init();
	dm_init();
	buzzer_init();
	timer0_init();
	timer0_run();
	
	state = CORRIENDO; // Inicia directamente en cuenta regresiva
	timer0_1seg_counter = 0; // Reiniciar contador
	
	sei(); // Habilitar interrupciones

	while (1) 
	{   
		switch (state)
		{
			case CONFIG_INICIAL:
				// Este estado ya no se usa al inicio, solo al reiniciar
				// Mostrar tiempo inicial 59:59
				dm_show_time(minutos, segundos);
				
				// Transición: presionar CONFIGURAR para ajustar segundos
				if (int1_debounce_flag == 1)
				{
					state = CONFIG_SEGUNDOS;
					int1_debounce_flag = 0;
				}
				break;
			
			case CONFIG_SEGUNDOS:
				// Mostrar tiempo actual
				dm_show_time(minutos, segundos);
				
				// Transición: START/STOP incrementa segundos
				if (int0_debounce_flag == 1)
				{
					segundos++;
					if (segundos > 59) segundos = 0;
					int0_debounce_flag = 0;
				}
				
				// Transición: CONFIGURAR para ajustar minutos
				if (int1_debounce_flag == 1)
				{
					state = CONFIG_MINUTOS;
					int1_debounce_flag = 0;
				}
				break;
			
			case CONFIG_MINUTOS:
				// Mostrar tiempo actual
				dm_show_time(minutos, segundos);
				
				// Transición: START/STOP incrementa minutos
				if (int0_debounce_flag == 1)
				{
					minutos++;
					if (minutos > 59) minutos = 0;
					int0_debounce_flag = 0;
				}
				
				// Transición: CONFIGURAR para estar listo
				if (int1_debounce_flag == 1)
				{
					state = LISTO;
					int1_debounce_flag = 0;
				}
				break;
			
			case LISTO:
				// Mostrar tiempo configurado
				dm_show_time(minutos, segundos);
				
				// Transición: START/STOP para comenzar
				if (int0_debounce_flag == 1)
				{
					state = CORRIENDO;
					timer0_1seg_counter = 0;
					int0_debounce_flag = 0;
				}
				
				// Permite reconfigurar
				if (int1_debounce_flag == 1)
				{
					state = CONFIG_SEGUNDOS;
					int1_debounce_flag = 0;
				}
				break;
			
			case CORRIENDO:
				// Mostrar tiempo actual
				dm_show_time(minutos, segundos);
				
				// Transición: START/STOP para pausar
				if (int0_debounce_flag == 1)
				{
					state = LISTO;
					int0_debounce_flag = 0;
				}
				
				// Transición: CONFIGURAR para reconfigurar
				if (int1_debounce_flag == 1)
				{
					state = CONFIG_SEGUNDOS;
					int1_debounce_flag = 0;
				}
				
				// Verificar si llegó a 00:00
				if (minutos == 0 && segundos == 0)
				{
					state = ALARMA;
					buzzer_active = 1;
					buzzer_counter = 0;
				}
				break;
			
			case ALARMA:
				// Mostrar 00:00
				dm_show_time(0, 0);
				
				// Controlar buzzer (encender/apagar cada 0.5 seg para efecto intermitente)
				if (buzzer_active)
				{
					if (buzzer_counter < 50) // 0.5 seg ON
					{
						buzzer_on();
					}
					else if (buzzer_counter < 100) // 0.5 seg OFF
					{
						buzzer_off();
					}
					else
					{
						buzzer_counter = 0; // Reiniciar ciclo
					}
				}
				
				// Transición: cualquier botón detiene el buzzer y reinicia el sistema
				if (int0_debounce_flag == 1 || int1_debounce_flag == 1)
				{
					buzzer_off();          // Apagar buzzer inmediatamente
					buzzer_active = 0;     // Desactivar bandera del buzzer
					minutos = 59;          // Reiniciar tiempo a 59:59
					segundos = 59;
					state = CORRIENDO;     // Volver a contar automáticamente
					timer0_1seg_counter = 0; // Reiniciar contador de segundos
					int0_debounce_flag = 0;  // Limpiar banderas
					int1_debounce_flag = 0;
				}
				break;
		}
	}//fin while
}//fin main

//Definiciones de funciones
void display_init(void)
{
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGA);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGB);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGC);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGD);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGE);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGF);
	DISPLAY_7SEG_G_DDRX  |= (1<<DISPLAY_7SEG_SEGG);
}

void display_show_number_array(uint8_t number)
{
	if (number > 9) number = 0; // Protección
	
	DISPLAY_7SEG_AF_PORTX = (display_numbers_array[number] & 0b00111111) | 
	                         (DISPLAY_7SEG_AF_PORTX & 0b11000000);
	DISPLAY_7SEG_G_PORTX = ((display_numbers_array[number] & 0b01000000) >> 6) | 
	                        (DISPLAY_7SEG_G_PORTX & 0b11111110);
}

void int0_init(void)
{
	// Configurar botón como entrada
	INT0_DDRX &= ~(1<<INT0_BTN0);
	// Activar pull-up interno
	INT0_PORTX |= (1<<INT0_BTN0);
	
	// Configurar rising edge (flanco de subida)
	EICRA |= (1<<ISC01) | (1<<ISC00);
	
	// Habilitar interrupción
	EIMSK |= (1<<INT0);
}

void int1_init(void)
{
	// Configurar botón como entrada
	INT1_DDRX &= ~(1<<INT1_BTN1);
	// Activar pull-up interno
	INT1_PORTX |= (1<<INT1_BTN1);
	
	// Configurar rising edge (flanco de subida)
	EICRA |= (1<<ISC11) | (1<<ISC10);
	
	// Habilitar interrupción
	EIMSK |= (1<<INT1);
}

void dm_init(void)
{
	DM_DDRX |= (1<<DM_MILLARES) | (1<<DM_CENTENAS) | (1<<DM_DECENAS) | (1<<DM_UNIDADES);
}

void update_time_digits(void)
{
	timedigits[MIN_DEC] = (minutos / 10);
	timedigits[MIN_UNI] = (minutos % 10);
	timedigits[SEG_DEC] = (segundos / 10);
	timedigits[SEG_UNI] = (segundos % 10);
}

void dm_show_time(uint8_t mins, uint8_t segs)
{
	// Actualizar dígitos
	timedigits[MIN_DEC] = (mins / 10);
	timedigits[MIN_UNI] = (mins % 10);
	timedigits[SEG_DEC] = (segs / 10);
	timedigits[SEG_UNI] = (segs % 10);
	
	// Multiplexación del display
	switch (timer0_barrido_flag)
	{
		case MIN_DEC:
			DM_PORTX = (1<<DM_MILLARES) | (DM_PORTX & 0b11100001);
			display_show_number_array(timedigits[MIN_DEC]);
			break;
		case MIN_UNI:
			DM_PORTX = (1<<DM_CENTENAS) | (DM_PORTX & 0b11100001);
			display_show_number_array(timedigits[MIN_UNI]);
			break;
		case SEG_DEC:
			DM_PORTX = (1<<DM_DECENAS) | (DM_PORTX & 0b11100001);
			display_show_number_array(timedigits[SEG_DEC]);
			break;
		case SEG_UNI:
			DM_PORTX = (1<<DM_UNIDADES) | (DM_PORTX & 0b11100001);
			display_show_number_array(timedigits[SEG_UNI]);
			break;
	}
}

void buzzer_init(void)
{
	// Configurar pin del buzzer como salida
	BUZZER_DDRX |= (1<<BUZZER_PIN);
	buzzer_off();
}

void buzzer_on(void)
{
	BUZZER_PORTX |= (1<<BUZZER_PIN);
}

void buzzer_off(void)
{
	BUZZER_PORTX &= ~(1<<BUZZER_PIN);
}

void timer0_init(void)
{
	// MODO CTC
	TCCR0A &= ~(1<<WGM00);
	TCCR0A |=  (1<<WGM01);
	TCCR0B &= ~(1<<WGM02);
	
	// TOP para ~0.01 segundos con prescaler 1024
	OCR0A = 156;
	OCR0B = 156;
	
	// Habilitar interrupciones
	TIMSK0 |= (1<<OCIE0A);  // Para cuenta regresiva de 1 segundo
	TIMSK0 |= (1<<OCIE0B);  // Para multiplexación del display
}

void timer0_run(void)
{
	// Reiniciar contador
	TCNT0 = 0;
	
	// Prescaler 1024
	TCCR0B |= (1<<CS02) | (1<<CS00);
	TCCR0B &= ~(1<<CS01);
}

// ISR Timer0 Compare A - Control de cuenta regresiva (cada ~0.01 seg)
ISR(TIMER0_COMPA_vect)
{
	// Incrementar contador para buzzer
	if (buzzer_active)
	{
		buzzer_counter++;
	}
	
	// Solo decrementar si está en estado CORRIENDO
	if (state == CORRIENDO)
	{
		timer0_1seg_counter++;
		
		if (timer0_1seg_counter >= 100) // 100 x 0.01 seg = 1 segundo
		{
			timer0_1seg_counter = 0;
			
			// Decrementar segundos
			if (segundos > 0)
			{
				segundos--;
			}
			else
			{
				segundos = 59;
				if (minutos > 0)
				{
					minutos--;
				}
				else
				{
					// Llegó a 00:00
					segundos = 0;
					minutos = 0;
				}
			}
		}
	}
}

// ISR Timer0 Compare B - Multiplexación del display (cada ~0.01 seg)
ISR(TIMER0_COMPB_vect)
{
	timer0_barrido_flag++;
	if (timer0_barrido_flag >= 4) 
	{
		timer0_barrido_flag = 0;
	}
}

// ISR INT0 - Botón START/STOP
ISR(INT0_vect)
{
	_delay_ms(50); // Antirrebote simple
	int0_debounce_flag = 1;
}

// ISR INT1 - Botón CONFIGURAR
ISR(INT1_vect)
{
	_delay_ms(50); // Antirrebote simple
	int1_debounce_flag = 1;
}