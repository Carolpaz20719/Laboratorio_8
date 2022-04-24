/*
 * File:   Postlab.c
 * Author: Carolina Paz
 *
 * Created on 22 de abril de 2022, 07:57 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 4000000      // Frecuencia de cristal 
#define _tmr0_value 208       // 100ms = 4*1/250kHz*(256-x)(128)

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);
void obtener_num (void);
void set_display (void);
void mostrar_valor (void);

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
uint16_t valor;     // valor para obtener centenas/decenas/unidades
uint8_t w;           // variable para tabla
uint8_t banderas=0;  // banderas para el multiplexado
uint8_t num[3];      // guardar centenas/decenas/unidades
uint8_t display[3];  // guardar valor que ira a los displays
uint8_t tabla[10]= {0b00111111,  //0
                    0b00000110,  //1
                    0b01011011,  //2
                    0b01001111,  //3
                    0b01100110,  //4
                    0b01101101,  //5
                    0b01111101,  //6
                    0b00000111,  //7
                    0b01111111,  //8
                    0b01101111}; //9

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    
    if(PIR1bits.ADIF){             // Fue interrupción del ADC
        if(ADCON0bits.CHS == 2)    // Verificamos sea AN2 el canal seleccionado
            PORTB = ADRESH;         // Mostramos ADRESH en PORTB
        else 
            valor = (ADRESH*100/51);   // Conversión del ADRESH a 0-500 
        PIR1bits.ADIF = 0;          // Limpiamos bandera de interrupción 
    }
    
    if(INTCONbits.T0IF){ 
            mostrar_valor();         // llamar a mostrar valor
            TMR0 = _tmr0_value;      // Reset del timer
            INTCONbits.T0IF = 0;     // Limpiamos bandera de interrupción
        }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();                // Llamamos a la función de configuraciones
    ADCON0bits.GO = 1;      // Colocamos en 1 para que comience
    while(1){
        obtener_num();      // llamar a obtener_num
        set_display();      // llamar a set_display
        
        if(ADCON0bits.GO == 0){           // Revisar  
            if(ADCON0bits.CHS == 2)       // si el canal es 2
                ADCON0bits.CHS = 1;       // Cambio de canal al 1
            
            else 
                ADCON0bits.CHS = 2;       // Cambio de canal al 2
            __delay_us(50);               // Tiempo de adquisición
            
            ADCON0bits.GO = 1;                 // Iniciamos proceso de conversión
        }
        
    }
    return;
}


void obtener_num (void){
    num[0]=  valor/100;                         // Centenas
    num[1]= (valor - num[0]*100)/10;            // Decenas
    num[2]= (valor - num[0]*100 - num[1]*10);   // Unidades
}

void set_display (void){
    w = num[0];                     // guardar centenas en w
    display[0]= tabla[w];           // Display de centenas
    
    w = num[1];                     // guardar decenas en w
    display[1]= tabla[w];           // Display de decenas
    
    w = num[2];                     // guardar unidades en w
    display[2]= tabla[w];           // Display de unidades
}

void mostrar_valor (void){
    switch (banderas){
        case 0:
            PORTC = display[0];     // Puerto c dentra a display de centenas
            PORTDbits.RD0 =1;       // 1
            PORTDbits.RD1 =0;       // 0
            PORTDbits.RD2 =0;       // 0
            banderas = 1;           // colocar banderas en 1
            return; 
        case 1:
            PORTC = display[1];     // Puerto c dentra a display de decenas
            PORTDbits.RD0 =0;       // 0
            PORTDbits.RD1 =1;       // 1
            PORTDbits.RD2 =0;       // 0
            banderas = 2;           // colocar banderas en 2
            return;
        case 2:
            PORTC = display[2];     // Puerto c dentra a display de unidades
            PORTDbits.RD0 =0;       // 0
            PORTDbits.RD1 =0;       // 0
            PORTDbits.RD2 =1;       // 1
            banderas = 0;           // colcoar banderas en 0
            return;
        default:
            PORTC = 0;              // Puerto c sera 0
            banderas = 0;           // banderas sera 0
    }
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    
    // Configuracion de entradas y salidas
    ANSEL = 0b00000110;         // AN1 Y AN2 como entrada analógica;
    ANSELH = 0;                 // Usaremos I/O digitales
    
    TRISA = 0b0110;             // PORTA como entrada
    PORTA = 0x00;               // Limpiamos a PORTA
    
    TRISB = 0x00;               // PORTB como salida
    PORTB = 0x00;               // Limpiamos PORTB 
    TRISC = 0x00;               // PORTC como salida
    PORTC = 0x00;               // Limpiamos PORTC 
    TRISD = 0x00;               // PORTD como salida
    PORTD = 0x00;               // Limpiamos PORTD
    
               
    // Configuracion del reloj
    OSCCONbits.IRCF = 0b0110;   // 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuracion TMR0
    OPTION_REGbits.T0CS = 0;    // Timer0 como temporizador
    OPTION_REGbits.PSA = 0;     // Prescaler a TIMER0
    OPTION_REGbits.PS = 0b0110; // Prescaler de 1 : 128
    TMR0 = _tmr0_value;         // Retardo del timer
    
    // Configuración ADC
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON0bits.CHS = 2;         // Seleccionamos el AN2
    ADCON0bits.ADON = 1;        // Habilitamos modulo ADC
    __delay_us(50);             // Sample time
    
    // Configuracion de Interrupciones
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;        // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;         // Habilitamos int. globales
    INTCONbits.T0IE = 1;        // Habilitamos interrupcion TMR0
    INTCONbits.T0IF = 0;        // Limpiamos bandera de interrupción TMR0
    
    return;
}