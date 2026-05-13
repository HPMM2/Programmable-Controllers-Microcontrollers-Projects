

/*************************************************************************
 *   Copyright (C) 2025 by Equipo 4 									 *
 *	 Heidi Pamela Martínez Martínez	    1952947	#6					     *
 *   Steven Neftalí Verdugo Figueroa	2132366	#60					     *
 *   Ángel Armando Solís Jiménez	    2132088	#55					     *	
 *   Carlos Emiliano Mendoza Ramírez	2048186	#35						 *
 *																		 *
 *   Attribution-NonCommercial-ShareAlike 3.0(CC-BY-NC-SA 3.0)           *
 *   AFU04 - Arquitectura Internas                                       *
 *   Dispositivo: atmega328p                                             *
 *   Lenguaje C                                                          *
 *   Rev: 1.0                                                            *
 *   Licencia:                                                           *
 *                                                     Fecha: 18/09/25   *
 ************************************************************************/


#define F_CPU 16000000UL 
//16MHz


//Bibliotecas
#include <avr/io.h> 
#include <util/delay.h>

// Macros para los botones
#define BTN0 (!(PINC & (1<<PC2)))
#define BTN1 (!(PINC & (1<<PC3)))
#define BTN2 (!(PINC & (1<<PC6)))

// LEDs en PD2-PD7
#define LED_MASK 0xFC 
//1 1 1 1 1 1 0 0 

// Números para display 7 segmentos (PB0-PB5 = a-f, PC0 = g)
uint8_t numeros[4][7] = {
  // a,b,c,d,e,f,g
    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1},  // 3

};

// Declarar Funciones
void mostrarNumero(uint8_t n);
void efectoAutoFantastico(void);
void secuenciaLibre(void);
void todosEncendidos(void);
void todosApagados(void);


int main(void) {
    DDRB = 0x3F;        // Display
    DDRC |= (1<<PC0);   // g
    DDRC &= ~((1<<PC2)|(1<<PC3)|(1<<PC6)); // botones como entrada
    PORTC |= (1<<PC2)|(1<<PC3)|(1<<PC6);   // pull-ups
    DDRD |= LED_MASK;    // LEDs

    //uint8_t step = 0;    // Para la secuencia Auto Fantástico
  
    while(1){
        // Botones (1 si presionado)
        uint8_t btn0 = BTN0 ? 1 : 0;
        uint8_t btn1 = BTN1 ? 1 : 0;
        uint8_t btn2 = BTN2 ? 1 : 0;

        if(btn1==0 && btn0==0){
            mostrarNumero(0);
            efectoAutoFantastico();
        } else if(btn1==0 && btn0==1){
            mostrarNumero(1);
            todosApagados();
        } else if(btn1==1 && btn0==0){
            mostrarNumero(2);
            secuenciaLibre();
        } else if(btn1==1 && btn0==1){
            mostrarNumero(3);
            todosEncendidos();
        } else if(btn2==1 && btn0==0 && btn1==0){
            todosApagados();
        }
    }
}

// Mostrar número en el display 
void mostrarNumero(uint8_t n) {
    if(n>3) return;
    PORTB = 0x00;
    PORTC &= ~(1<<PC0);
    for(uint8_t i=0; i<6; i++){
        if(numeros[n][i])
            PORTB |= (1<<i);
    }
    if(numeros[n][6])
        PORTC |= (1<<PC0);
}

// Secuencia Auto Fantástico
void efectoAutoFantastico(void){
    static uint8_t dir = 1;   // dirección 1 = PD2?PD7, 0 = PD7?PD2
    static uint8_t pos = 2;   

    PORTD = (1<<pos);
    _delay_ms(500); // delay

  //secuencia 
    if(dir){
        pos++;
        if(pos>7) { pos=6; dir=0; }
    } else {
        pos--;
        if(pos<2) { pos=3; dir=1; }
    }
}


// Secuencia libre (3 si, 3 no)
void secuenciaLibre(void){
    static uint8_t state = 0;
    if(state == 0){
        PORTD = (1<<PD2) | (1<<PD3) | (1<<PD4); // Primeros 3 LEDs encendidos
        state = 1;
    } else {
        PORTD = (1<<PD5) | (1<<PD6) | (1<<PD7); // Últimos 3 LEDs encendidos
        state = 0;
    }
    _delay_ms(500); // Delay 
}


// Todos LEDs encendidos
void todosEncendidos(void){
    PORTD |= LED_MASK;
}

// Todos LEDs apagados
void todosApagados(void){
    PORTD &= ~LED_MASK;
}
