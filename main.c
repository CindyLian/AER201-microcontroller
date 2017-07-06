/* 
 * File:                main.c
 * Author:              Cindy Lian
 *
 * Created on:          February 25, 2017, 10:53 PM
 * 
 * Main file controlling overall program flow + interface. Code for RTC taken 
 * from main.c of PICDevBugger PIC_I2C_RTC MPLABX sample code 
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "start.h"
#include "logs.h"
#include "I2C.h"

int sorting = 0;
int logs = 0;
int counter = 0;

// <editor-fold defaultstate="collapsed" desc="DECLARATIONS">
const char keys[] = "123A456B789C*0#D"; //list of keypad buttons
const char happynewyear[7] = {0x00, //45 Seconds 
    0x30, //59 Minutes
    0x00, //24 hour mode, set to 23:00
    0x0, //Saturday 
    0x28, //31st
    0x03, //December
    0x17}; //2016
//</editor-fold>

/* Sets the time based on the constant defined at the top of this file. 
 * Is not called during a normal program run.
 */
void set_time(void) {
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for (char i = 0; i < 7; i++) {
        I2C_Master_Write(happynewyear[i]);
    }
    I2C_Master_Stop(); //Stop condition    
}

void main(void) {

    // <editor-fold defaultstate="collapsed" desc=" STARTUP SEQUENCE ">
    TRISA = 0xFF; //All input (A0, A1)
    TRISB = 0xFF; //All input mode
    TRISC = 0x00; //All output (C0, C1, C2, C5, C6, C7)
    TRISD = 0x00; //All output mode
    TRISE = 0b00000010; //E0 out, E1 in 

    T1CON = 0x10; //Initialize Timer Module
    T0CON = 0x80;

    ADCON0 = 0x00; //Enable ADC and clock selection
    ADCON1 = 0x0D; //AN0, AN1 analog
    INT1IE = 1; //enables int1 external interrupt

    //</editor-fold>

    while (1) //run program; display RTC
    {
        // <editor-fold defaultstate="collapsed" desc="INITIALIZE BITS">
        DC_REF = 1; //H-bridge reference set to 5V
        AGITATE = 0;
        dc_stop();
        //</editor-fold>

        initLCD();
        I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock

        ei(); //enable (keypad) interrupts 
        while (1) {

            lcdInst(LINE_1); //first line
            printf("1:Start; 2:Logs"); //print menu
            printRTC();
            if (sorting == 1) {
                sort();
                sorting = 0;
            }
            if (logs == 1) {
                int ec = Eeprom_ReadByte(1);
                printf("%d", ec);
                //showLogMenu();
                logs = 0;
            }
        }
        di(); //disable (keypad) interrupts
    }
    return;
}

/* Keypad button pressed interrupt: 
 * 1 to 2: normal robot function
 * 3 to D: sensor and motor testing
 * 
 * 0 = RESET
 * 1 = Sort
 * 2 = Logs
 * 3 = Ultrasonic
 * 4 = Light Sensor
 * 5 = Non-continuous Servo
 * 6 = Continuous Servo
 * 7 = DC forward
 * 8 = DC reverse
 * 9 = DC stop
 * A = Proximity Sensor
 * B = IR Sensor
 * D = All Sensors
 * * = rotate 180
 * # = agitatel 
 */
void interrupt keypressed(void) {
    if (INT1IF) { //external interrupt occurred
        unsigned char keypress = (PORTB & 0xF0) >> 4;
        initLCD();
        if (keys[keypress] == '0') {
            printf("RESET");
            __delay_2s();
#asm 
            RESET
#endasm 
        } else if (keys[keypress] == '1') {
            sorting = 1;
        } else if (keys[keypress] == '2') {
            printf("Logs");
            __delay_1s();
            logs = 1;
        }// <editor-fold defaultstate="collapsed" desc=" TESTING ">
        else if (keys[keypress] == '3') { //ultrasonic
            while (1)//wait for ultrasonic to find bottle
            {
                int i = ultrasonicFindBottle();
                if (i != 0) {
                    printf("%d", i);
                    break;
                }
            }
            __delay_3s();
        } else if (keys[keypress] == '4') { //light sensor

            float lightVoltSum = 0;
            for (int i = 0; i < 100; i++) {
                readADC(0);
                float lightVolt = (float) ((ADRESH << 8) / 236)*5;
                lightVoltSum += lightVolt;
            }
            lightVoltSum = lightVoltSum / 1000;

            lcdInst(LINE_1); //first line
            printf("lightSum %f", lightVoltSum);
            __delay_3s();
        } else if (keys[keypress] == '5') { //noncontinuous servo

            for (int i = 0; i < 50; i++) { //open flap
                NON_CONT_SERVO = 1;
                __delay_us(2000);
                NON_CONT_SERVO = 0;
                __delay_us(25000);
            }
            for (int i = 0; i < 50; i++) { //close flap
                NON_CONT_SERVO = 1;
                __delay_us(3200);
                NON_CONT_SERVO = 0;
                __delay_us(18500);
            }

        } else if (keys[keypress] == '6') { //continuous servo
            rotate90();
        } else if (keys[keypress] == '7') { //dc reverse
            dc_forward();
        } else if (keys[keypress] == '8') { //dc forward                      
            dc_reverse();
        } else if (keys[keypress] == '9') { //dc stop
            dc_stop();
        } else if (keys[keypress] == 'A') {//prox sensor

            //check proximity sensor
            readADC(1);
            float proxVolt = (float) ((ADRESH << 8) / 236)*5;
            lcdInst(LINE_1); //first line
            printf("proxVolt %f", proxVolt);
            __delay_3s();

        } else if (keys[keypress] == 'B') { //IR sensor
            readADC(2);
            float IRSensor = (float) ((ADRESH << 8) / 236)*5;
            lcdInst(LINE_1); //first line
            printf("IR %f", IRSensor);
            __delay_3s();
        } else if (keys[keypress] == 'D') {
            readADC(2); //IR
            float IRVolt = (float) ((ADRESH << 8) / 236)*5;

            readADC(1); //Proximity
            float proxVolt = (float) ((ADRESH << 8) / 236)*5;

            //check ambient light sensor
            float lightVoltSum = 0;

            for (int i = 0; i < 1000; i++) { //Light sensor
                readADC(0);
                float lightVolt = (float) ((ADRESH << 8) / 236)*5;
                lightVoltSum += lightVolt;
            }

            lightVoltSum = lightVoltSum / 10000;

            lcdInst(LINE_2); //go to second line
            int prox = (int) proxVolt;
            int light = (int) (lightVoltSum * 10);
            int IR = (int) IRVolt;
            printf("%d %d %d", light, prox, IR);

            lcdInst(LINE_1); //first line


            if (lightVoltSum > 39) { //eska
                if (IRVolt < -100 || proxVolt > 350) {
                    printf("eska cap");
                } else {
                    printf("eska no cap");
                }
            } else { //yop
                if (IRVolt > 100 && proxVolt >= 300) {
                    printf("yop cap");
                } else {
                    printf("yop no cap");
                }
            }

            __delay_3s();

        }
        else if (keys[keypress] == '*'){
            rotate180 ();
        }
        else if (keys[keypress] == '#'){
            rotate270();
        }
        
        
        //</editor-fold>

        INT1IF = 0; //Clear flag bit
    } else if (TMR0IF) {
        
        TMR0IF = 0;
        timeElapsed ++;
        TMR0ON = 0;
        lcdInst(LINE_1);
        printf("Time: %d", timeElapsed);
        
        if (timeElapsed < 180){
            T0CON = 0b00010111;
            TMR0H = 0b11011010;
            TMR0L = 0b00010000;
            T0CON = T0CON | 0b10000000;
        }

    }

}
