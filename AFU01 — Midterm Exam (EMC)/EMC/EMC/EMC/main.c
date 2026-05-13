/*************************************************************************
 *   Copyright (C) 2025 by Equipo 4                                      *
 *   Heidi Pamela Martínez Martínez    1952947   #6                      *
 *   Steven Neftalí Verdugo Figueroa   2132366   #60                     *
 *   Ángel Armando Solís Jiménez       2132088   #55                     *    
 *   Carlos Emiliano Mendoza Ramírez   2048186   #35                     *
 *                                                                       *
 *   Attribution-NonCommercial-ShareAlike 3.0(CC-BY-NC-SA 3.0)           *
 *   AFU01 - Examen de Medio Curso - Práctico                            *
 *   Dispositivo: atmega328p                                             *
 *   Lenguaje C                                                          *
 *   Rev: 1.0                                                            *
 *   Fecha: 09/10/25                                                     *
 ************************************************************************/
#define F_CPU 16000000UL  // 16 MHz - Frecuencia del cristal

//LIBRERIAS
#include <avr/io.h>
#include <util/delay.h>

#define RETARDO 200  // Tiempo base para delays en milisegundos

//LEDS - Configuración de pines para los 4 LEDs en PORTD
#define LEDS_DDRX DDRD
#define LEDS_PORTX PORTD
#define LEDS_LED0 PORTD2
#define LEDS_LED1 PORTD3
#define LEDS_LED2 PORTD4
#define LEDS_LED3 PORTD5

// Array para manejar fácilmente los LEDs en secuencias
uint8_t LEDS[4] = { LEDS_LED0, //0
	LEDS_LED1, //1
	LEDS_LED2, //2
LEDS_LED3}; //3

//BOTONES - Configuración de pines para botones en PORTC
#define BUTTONS_DDRX   DDRC
#define BUTTONS_PORTX  PORTC
#define BUTTONS_PINX   PINC
#define BUTTONS_BTN0   PINC2
#define BUTTONS_BTN1   PINC3
#define BUTTONS_BTN2   PINC6

//MACRO BOTONES - Lectura de estado de botones (0=presionado, 1=no presionado)
#define BUTTONS_BTN0_READ (!(BUTTONS_PINX & (1<<BUTTONS_BTN0)))
#define BUTTONS_BTN1_READ (!(BUTTONS_PINX & (1<<BUTTONS_BTN1)))
#define BUTTONS_BTN2_READ (!(BUTTONS_PINX & (1<<BUTTONS_BTN2)))

//DISPLAY ANODO COMUN - Configuración de pines para display 7 segmentos
#define DISPLAY_7SEG_AF_DDRX DDRB
#define DISPLAY_7SEG_AF_PORTX PORTB
#define DISPLAY_7SEG_G_DDRX DDRC
#define DISPLAY_7SEG_G_PORTX PORTC
#define DISPLAY_7SEG_SEGA PORTB0
#define DISPLAY_7SEG_SEGB PORTB1
#define DISPLAY_7SEG_SEGC PORTB2
#define DISPLAY_7SEG_SEGD PORTB3
#define DISPLAY_7SEG_SEGE PORTB4
#define DISPLAY_7SEG_SEGF PORTB5
#define DISPLAY_7SEG_SEGG PORTC0

//L293D - MOTOR DC - Configuración de pines para control del motor
#define L293D_DDRX DDRC
#define L293D_PORTX PORTC
#define L293D_IN1 PORTC5
#define L293D_IN2 PORTC4
#define L293D_EN1 PORTC1

//DECLARACIONES DE FUNCIONES
void leds_init(void);
void leds_blink_all(void);
void leds_center_to_edges(void);
void leds_shift_left_to_right(void);
void leds_shift_right_to_left(void);
void leds_all_on(void);
void leds_all_off(void);
void buttons_init(void);
void display_init(void);
void display_show_number(uint8_t number);
void l293d_init(void);
void l293d_motor_fast_stop(void);
void l293d_motor_free_wheel(void);
void l293d_motor_turn_right(void);
void l293d_motor_turn_left(void);

//// ------------------------ MAIN ------------------------
//------------------------------------------------------------
//------------------------------------------------------------
int main(void)
{
	//llamar funciones de inicialización
	leds_init();
	buttons_init();
	display_init();
	l293d_init();
	uint8_t i = 0;
	
	// Loop principal infinito
	while (1)
	{
		//00 
		if (!(BUTTONS_BTN0_READ) && !(BUTTONS_BTN1_READ))
		{
			display_show_number('E');  // Mostrar 'E' en display
			leds_center_to_edges();    // Efecto de LEDs centro-orillas
			l293d_motor_fast_stop();   // Parada rápida del motor
		}
		
		//01 
		if (!(BUTTONS_BTN0_READ) && (BUTTONS_BTN1_READ))
		{
			display_show_number(1);           // Mostrar '1' en display
			leds_shift_left_to_right();       // Efecto LEDs izquierda-derecha
			l293d_motor_turn_left();          // Motor gira a la izquierda
		}
		
		//10 
		if ((BUTTONS_BTN0_READ) && !(BUTTONS_BTN1_READ))
		{
			display_show_number(2);           // Mostrar '2' en display
			leds_shift_right_to_left();       // Efecto LEDs derecha-izquierda
			l293d_motor_turn_right();         // Motor gira a la derecha
		}
		
		//11 
		if ((BUTTONS_BTN0_READ) && (BUTTONS_BTN1_READ))
		{
			display_show_number('P');         // Mostrar 'P' en display
			leds_blink_all();                 // Todos los LEDs parpadeando
			l293d_motor_free_wheel();         // Motor en modo libre
		}
	}//fin while
}//fin main

//------------------------------------------------------------
//------------------------------------------------------------


// Inicialización de LEDs - Configura pines como salidas
void leds_init(void)
{
	LEDS_DDRX |= (1<<LEDS_LED0); // Configura LED0 como salida
	LEDS_DDRX |= (1<<LEDS_LED1); // Configura LED1 como salida
	LEDS_DDRX |= (1<<LEDS_LED2); // Configura LED2 como salida
	LEDS_DDRX |= (1<<LEDS_LED3); // Configura LED3 como salida
}

// Efecto: Parpadeo de todos los LEDs simultáneamente
void leds_blink_all(void)
{
	// Encender todos los LEDs
	leds_all_on();
	_delay_ms(RETARDO);
	
	// Apagar todos los LEDs
	leds_all_off();
	_delay_ms(RETARDO);
}

// Efecto: Primero se encienden los LEDs centrales, luego los de las orillas
void leds_center_to_edges(void)
{
	// 1. Encender solo los del centro (LED1 y LED2)
	leds_all_off();
	LEDS_PORTX |= (1 << LEDS_LED1) | (1 << LEDS_LED2);
	_delay_ms(RETARDO);
	
	// 2. Encender solo las orillas (LED0 y LED3)
	leds_all_off();
	LEDS_PORTX |= (1 << LEDS_LED0) | (1 << LEDS_LED3);
	_delay_ms(RETARDO);
}

// Efecto: Un LED se desplaza de izquierda a derecha
void leds_shift_left_to_right(void)
{
	// Orden de LEDs: LED0 ? LED1 ? LED2 ? LED3 (de izquierda a derecha)
	for(uint8_t i = 0; i < 4; i++)
	{
		leds_all_off();  // Apagar todos
		LEDS_PORTX |= (1 << LEDS[i]);  // Encender solo el LED actual
		_delay_ms(RETARDO);
	}
}

// Efecto: Un LED se desplaza de derecha a izquierda
void leds_shift_right_to_left(void)
{
	// Orden de LEDs: LED3 ? LED2 ? LED1 ? LED0 (de derecha a izquierda)
	for(int8_t i = 3; i >= 0; i--)
	{
		leds_all_off();  // Apagar todos
		LEDS_PORTX |= (1 << LEDS[i]);  // Encender solo el LED actual
		_delay_ms(RETARDO);
	}
}

// Encender todos los LEDs simultáneamente
void leds_all_on(void)
{
	LEDS_PORTX |= (1<<LEDS_LED0); // Encender LED0
	LEDS_PORTX |= (1<<LEDS_LED1); // Encender LED1
	LEDS_PORTX |= (1<<LEDS_LED2); // Encender LED2
	LEDS_PORTX |= (1<<LEDS_LED3); // Encender LED3
}

// Apagar todos los LEDs simultáneamente
void leds_all_off(void)
{
	LEDS_PORTX &= ~(1<<LEDS_LED0); // Apagar LED0
	LEDS_PORTX &= ~(1<<LEDS_LED1); // Apagar LED1
	LEDS_PORTX &= ~(1<<LEDS_LED2); // Apagar LED2
	LEDS_PORTX &= ~(1<<LEDS_LED3); // Apagar LED3
}

// Inicialización de botones - Configura pines como entradas con pull-up
void buttons_init(void)
{
	// Botones 0 y 1 con pull-up INTERNO
	BUTTONS_DDRX &= ~(1<<BUTTONS_BTN0);  // Configurar como entrada
	BUTTONS_DDRX &= ~(1<<BUTTONS_BTN1);  // Configurar como entrada
	BUTTONS_PORTX |= (1<<BUTTONS_BTN0);  // Activar pull-up interno
	BUTTONS_PORTX |= (1<<BUTTONS_BTN1);  // Activar pull-up interno
	
	// Botón 2 en PC6 - usar pull-up EXTERNO (RESET del Arduino)
	BUTTONS_DDRX &= ~(1<<BUTTONS_BTN2);     // Configurar como entrada
	BUTTONS_PORTX &= ~(1<<BUTTONS_BTN2);    // Desactivar pull-up interno
}

// Inicialización del display - Configura todos los segmentos como salidas
void display_init(void)
{
	// Configurar segmentos A-F como salidas en PORTB
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGA);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGB);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGC);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGD);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGE);
	DISPLAY_7SEG_AF_DDRX |= (1<<DISPLAY_7SEG_SEGF);
	// Configurar segmento G como salida en PORTC
	DISPLAY_7SEG_G_DDRX |= (1<<DISPLAY_7SEG_SEGG);
}

// Muestra caracteres en display 7 segmentos (ánodo común)
void display_show_number(uint8_t number)
{
	switch (number)
	{
		case 1:  // Mostrar número 1
		DISPLAY_7SEG_AF_PORTX |= (1<<DISPLAY_7SEG_SEGA);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGB);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGC);
		DISPLAY_7SEG_AF_PORTX |= (1<<DISPLAY_7SEG_SEGD);
		DISPLAY_7SEG_AF_PORTX |= (1<<DISPLAY_7SEG_SEGE);
		DISPLAY_7SEG_AF_PORTX |= (1<<DISPLAY_7SEG_SEGF);
		DISPLAY_7SEG_G_PORTX  |= (1<<DISPLAY_7SEG_SEGG);
		break;
		
		case 2:  // Mostrar número 2
		DISPLAY_7SEG_AF_PORTX &= ~ (1<<DISPLAY_7SEG_SEGA);
		DISPLAY_7SEG_AF_PORTX &= ~ (1<<DISPLAY_7SEG_SEGB);
		DISPLAY_7SEG_AF_PORTX |= (1<<DISPLAY_7SEG_SEGC);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGD);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGE);
		DISPLAY_7SEG_AF_PORTX |= (1<<DISPLAY_7SEG_SEGF);
		DISPLAY_7SEG_G_PORTX  &= ~ (1<<DISPLAY_7SEG_SEGG);
		break;
		
		case 'E':  // Mostrar letra E 
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGA);
		DISPLAY_7SEG_AF_PORTX |=(1<<DISPLAY_7SEG_SEGB);
		DISPLAY_7SEG_AF_PORTX |=(1<<DISPLAY_7SEG_SEGC);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGD);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGE);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGF);
		DISPLAY_7SEG_G_PORTX &= ~(1<<DISPLAY_7SEG_SEGG);
		break;
		
		case 'P':  // Mostrar letra P 
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGA);
		DISPLAY_7SEG_AF_PORTX &= ~ (1<<DISPLAY_7SEG_SEGB);
		DISPLAY_7SEG_AF_PORTX |=(1<<DISPLAY_7SEG_SEGC);
		DISPLAY_7SEG_AF_PORTX |=(1<<DISPLAY_7SEG_SEGD);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGE);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGF);
		DISPLAY_7SEG_G_PORTX &= ~(1<<DISPLAY_7SEG_SEGG);
		break;

		case 'F':  // Mostrar letra F 
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGA);
		DISPLAY_7SEG_AF_PORTX |= (1<<DISPLAY_7SEG_SEGB);
		DISPLAY_7SEG_AF_PORTX |=(1<<DISPLAY_7SEG_SEGC);
		DISPLAY_7SEG_AF_PORTX |=(1<<DISPLAY_7SEG_SEGD);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGE);
		DISPLAY_7SEG_AF_PORTX &= ~(1<<DISPLAY_7SEG_SEGF);
		DISPLAY_7SEG_G_PORTX &= ~(1<<DISPLAY_7SEG_SEGG);
		break;
	}
}

// Inicialización del controlador L293D - Configura pines como salidas
void l293d_init(void)
{
	L293D_DDRX |= (1<<L293D_EN1);  // Enable como salida
	L293D_DDRX |= (1<<L293D_IN1);  // IN1 como salida
	L293D_DDRX |= (1<<L293D_IN2);  // IN2 como salida
}

// Parada rápida del motor 
void l293d_motor_fast_stop(void)
{
	L293D_PORTX &= ~ (1<<L293D_EN1);   // Deshabilitar el motor
	L293D_PORTX &= ~ (1<<L293D_IN1);   // IN1 = LOW
	L293D_PORTX &= ~ (1<<L293D_IN2);   // IN2 = LOW ? Freno activo
}

// Motor en modo libre 
void l293d_motor_free_wheel(void)
{
	L293D_PORTX &= ~(1<<L293D_IN1);  // IN1 = LOW
	L293D_PORTX &= ~(1<<L293D_IN2);  // IN2 = LOW
	L293D_PORTX &= ~(1<<L293D_EN1);  // Apagar enable
}

// Motor gira en sentido DERECHA
void l293d_motor_turn_right(void)
{
	L293D_PORTX |= (1<<L293D_EN1);   // Habilitar el motor
	L293D_PORTX |= (1<<L293D_IN1);   // IN1 = HIGH
	L293D_PORTX &= ~ (1<<L293D_IN2); // IN2 = LOW
}

// Motor gira en sentido IZQUIERDA
void l293d_motor_turn_left(void)
{
	L293D_PORTX |= (1<<L293D_EN1);   // Habilita el motor
	L293D_PORTX &= ~(1<<L293D_IN1);  // IN1 = LOW
	L293D_PORTX |= (1<<L293D_IN2);   // IN2 = HIGH
}