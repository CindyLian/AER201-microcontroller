/* 
 * File:                constants.h
 *
 * Created on:          February 25, 2017, 10:53 PM
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H         //Prevent multiple inclusion 

//LCD Control Registers
#define RS              LATDbits.LATD2          
#define E               LATDbits.LATD3
#define	LCD_PORT        LATD   //On LATD[4,7] to be specific
#define LCD_DELAY       25

#define __delay_1s()    for(int i=0;i<100;i++){__delay_ms(10);}   //delays 1 second
#define __delay_2s()    for(int i=0;i<200;i++){__delay_ms(10);}   //delays 2 second
#define __delay_3s()    for(int i=0;i<300;i++){__delay_ms(10);}   //delays 3 second
#define __delay_4s()    for(int i=0;i<400;i++){__delay_ms(10);}   //delays 4 second 
#define __delay_5s()    for(int i=0;i<500;i++){__delay_ms(10);}   //delays 5 second

#define LINE_1          0b10000000
#define LINE_2          0b11000000

//input and output bits
#define DC_REF          LATCbits.LATC0
#define DC_FOR          LATCbits.LATC2    
#define DC_REV          LATCbits.LATC1

#define NON_CONT_SERVO  LATCbits.LATC5
#define CONT_SERVO      LATCbits.LATC6

#define TRIGGER         LATEbits.LATE0
#define ECHO            PORTEbits.RE1

#define AGITATE         LATCbits.LATC7

#endif	/* CONSTANTS_H */

