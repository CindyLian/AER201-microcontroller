/* 
 * File:                start.c
 * Author:              Cindy Lian
 *
 * Created on:          February 25, 2017, 10:53 PM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "logs.h"
#include "I2C.h"

int eskaCap, eskaNoCap, yopCap, yopNoCap, timeElapsed;
const char keys[] = "123A456B789C*0#D"; //list of keypad buttons

/*
 * Resets bottle count and timer to 0 for a new sorting operation
 */
void resetCounts() {
    eskaCap = 0;
    eskaNoCap = 0;
    yopCap = 0;
    yopNoCap = 0;
    timeElapsed = 0;
}

/*
 * Senses a bottle on the conveyor belt with the ultrasonic sensor.
 * Returns the distance of the bottle (in cm) if the bottle is between 1 and 
 * 5 cm away. Returns 0 if there is no bottle. 
 */
int ultrasonicFindBottle() {
    int a;
    TRIGGER = 1; //trigger high
    __delay_us(10); //delay for 10us
    TRIGGER = 0; //trigger low

    TMR1H = 0; //Sets the Initial Value of Timer
    TMR1L = 0; //Sets the Initial Value of Timer

    TMR1ON = 1; //Timer Starts
    while (!ECHO) //Waiting for Echo goes HIGH
    {
        a = (TMR1L | (TMR1H << 8)); //Reads Timer Value
        if (a > 50) return 0; //if there is no echo high after a certain time, send trigger again 
    }
    TMR1ON = 0;

    TMR1H = 0; //reset timer
    TMR1L = 0;

    TMR1ON = 1; //Timer Starts
    while (ECHO); //Waiting for Echo goes LOW
    TMR1ON = 0; //Timer Stops     

    a = (TMR1L | (TMR1H << 8)); //Reads Timer Value
    a = (int) a / 58.82; //Converts Time to Distance
    a = a + 1; //Distance Calibration

    if (a > 1 & a <= 5) {//in between 1 and 5 cm
        return a;
    }
    return 0;
}

/*
 * Rotates the continuous servo controlling the containers by 90 degrees 
 * clockwise. 
 */
void rotate90 () {
    unsigned int i;
    for (i = 0; i < 50; i++) {
        LATCbits.LATC6 = 1;
        __delay_us(2000);
        LATCbits.LATC6 = 0;
        __delay_us(18000);
    }
}

void rotate180 () {
    unsigned int i;
    for (i = 0; i < 100; i++) {
        LATCbits.LATC6 = 1;
        __delay_us(2000);
        LATCbits.LATC6 = 0;
        __delay_us(18000);
    }
}
void rotate270 () {
    unsigned int i;
    for (i = 0; i < 50; i++) {
        LATCbits.LATC6 = 1;
        __delay_us(1730);
        LATCbits.LATC6 = 0;
        __delay_us(18270);
    }
}

void dc_stop() {
    DC_FOR = 0;
    DC_REV = 0;
}

void dc_forward() {
    dc_stop();
    __delay_1s();
    DC_REV = 0;
    DC_FOR = 1;
}

void dc_reverse() {
    dc_stop();
    __delay_1s();
    DC_FOR = 0;
    DC_REV = 1;
}

/*
 * Reads in an analog signal and converts it into a digital signal. The character
 * parameter indicates which bit the analog signal is being read in from.
 */
void readADC(char channel) {
    // Select A2D channel to read
    ADCON0 = ((channel << 2));
    ADON = 1;
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO_NOT_DONE) {
        __delay_ms(5);
    }
}

/*
 * The entire sorting process for all bottles which are to be loaded into the 
 * loading bin. Sorting will continue until 10 bottles have been sorted, or 3 
 * minutes has passed. 
 */
void sort() {

    // <editor-fold defaultstate="collapsed" desc="Initialize Variables & Startup">
    int bottle; //type of bottle 0 = eskaCap, 1 = eskaNoCap, 2 = yopCap, 3 = yopNoCap
    int currentContainer = 0; //same as bottle
    timeElapsed = 0;
    resetCounts(); //resets bottle counts and timer

    for (int i = 0; i < 50; i++) { //close flap
        NON_CONT_SERVO = 1;
        __delay_us(3200);
        NON_CONT_SERVO = 0;
        __delay_us(18500);
    }
    //</editor-fold>

    T0CON = 0b00010111;
    TMR0H = 0b11011110;
    TMR0L = 0b11010000;
    T0CON = T0CON | 0b10000000;
    TMR0IE = 1; //enable timer 0 interrupts
    TMR0ON = 1;
    int done = 0;

    while (1) { //Each iteration of the loop will sort a single bottle

        /*
         * displays current bottle counts 
         */
        // <editor-fold defaultstate="collapsed" desc="Display Info">
        initLCD();
        lcdInst(LINE_2); //go to second line
        printf("ec:%den:%dyc:%dyn:%d", eskaCap, eskaNoCap, yopCap, yopNoCap);
        //</editor-fold>

        /*
         * The DC motor and ultrasonic sensor will find and isolate a single 
         * bottle which will then be sorted by the various other sensors. 
         * 
         * -> DC forward
         * -> If bottle (ultrasonic), delay 4 seconds to allow bottle to fall
         * -> If there is a bottle in the "chamber" (light sensor), continue
         *      -> If no bottle, DC reverse for 7 seconds, then repeat from top
         * -> DC stop, wait 2 seconds to allow for bottle to settle 
         * -> There should now be a bottle in the sorting chamber
         */
        // <editor-fold defaultstate="collapsed" desc="DC & Ultrasonic">

        int bottleFound = 0;
        int timeExceeded = 0;
        int timer = 0;
        while (1) {
            timer = timeElapsed;
            dc_forward();

            while (1) {
                if (ultrasonicFindBottle() != 0) {
                    bottleFound = 1;
                    break;
                }
                if ((timeElapsed-timer)>8) {
                    timeExceeded = 1;
                    break;
                }
                if (timeElapsed == 180){
                    done = 1;
                    break;
                }
            }
            
            if (done == 1){
                break;
            }

            if (bottleFound) {//if bottle has been found 
                __delay_4s();

                float lightVoltSum = 0; //take 1000 readings of light sensor + average 
                for (int i = 0; i < 100; i++) {
                    readADC(0);
                    float lightVolt = (float) ((ADRESH << 8) / 236)*5;
                    lightVoltSum += lightVolt;
                }
                lightVoltSum = lightVoltSum / 1000;
                //printf ("%f", lightVoltSum);
                if (lightVoltSum < 36) {
                    break;
                }
            }

            if (timeElapsed > 173){
                for (int x = 0; x <= (180-timeElapsed); x++){__delay_1s();}
                timeElapsed = 180;
                done = 1;
                break;
            }
            
            dc_reverse();
            AGITATE = 1;
            __delay_5s();
            __delay_2s();
            AGITATE = 0;

            float lightVoltSum = 0; //take 100 readings of light sensor + average 
            for (int i = 0; i < 100; i++) {
                readADC(0);
                float lightVolt = (float) ((ADRESH << 8) / 236)*5;
                lightVoltSum += lightVolt;
            }
            lightVoltSum = lightVoltSum / 1000;
            if (lightVoltSum < 36) {
                break;
            }
            

        }
        

        dc_stop();
        if (done == 1){
                break;
        }
        __delay_2s(); //wait for bottle to settle
        

        //</editor-fold>

        /*
         * Check all the sensors to sort the bottle. 
         * Proximity sensor, light sensor, IR sensor. 
         * 
         * All three are analog signals, and have been calibrated to sort the
         * bottles into the four categories.
         */
        // <editor-fold defaultstate="collapsed" desc="All Sensors + Sorting">
        readADC(2); //read IR sensor
        float IRVolt = (float) ((ADRESH << 8) / 236)*5;
        readADC(1); //read proximity sensor
        float proxVolt = (float) ((ADRESH << 8) / 236)*5;
        float lightVoltSum = 0; //take 1000 readings of light sensor + average 
        for (int i = 0; i < 100; i++) {
            readADC(0);
            float lightVolt = (float) ((ADRESH << 8) / 236)*5;
            lightVoltSum += lightVolt;
        }
        lightVoltSum = lightVoltSum / 1000;

        if (lightVoltSum > 34.5) { //eska
            if (IRVolt < -100 || proxVolt > 300) {
                eskaCap = eskaCap + 1; //0
                bottle = 0;
            } else {
                eskaNoCap = eskaNoCap + 1; //1
                bottle = 1;
            }
        } else { //yop
            if  (IRVolt > 100 && (proxVolt > 280 ||proxVolt < 220)) {
                yopCap = yopCap + 1; //2
                bottle = 2;
            } else {
                yopNoCap = yopNoCap + 1; //3 
                bottle = 3;
            }
        }

        //</editor-fold>

        /*
         * Rotates the continuous servo so the bottle will fall into the 
         * correct container. Keeps track of the container that is currently 
         * in the falling position.
         * 
         * Opens and closes flap with positional servo with delay to allow 
         * time for bottle to fall. 
         */
        // <editor-fold defaultstate="collapsed" desc="Servos">
        
        int numRotations = bottle - currentContainer;
        
        if (numRotations == 3 || numRotations == -1){
            rotate270 ();
        }
        else if (numRotations == 2 || numRotations == -2){
            rotate180 ();
        }
        else if (numRotations == 1 || numRotations == -3){
            rotate90();
        }
        
        currentContainer = bottle;
        
/*
        while (1) { //rotate until correct container
            if (currentContainer != bottle) {
                rotate90();
                if (currentContainer != 3)
                    currentContainer = currentContainer + 1;
                else
                    currentContainer = 0;
            } else {
                break;
            }
            //for(int i=0;i<100;i++){__delay_ms(10);}   //delays 0.5 second

        }
*/
        
        
        for(int i=0;i<50;i++){__delay_ms(10);}   //delays 0.5 second

        for (int i = 0; i < 50; i++) { //open flap
            NON_CONT_SERVO = 1;
            __delay_us(2000);
            NON_CONT_SERVO = 0;
            __delay_us(25000);
        }
        __delay_2s(); // wait for bottle to fall

        for (int i = 0; i < 50; i++) { //close flap
            NON_CONT_SERVO = 1;
            __delay_us(3200);
            NON_CONT_SERVO = 0;
            __delay_us(18500);
        }
        //</editor-fold>


        //done if 10 bottles have been sorted
        if ((eskaCap + eskaNoCap + yopCap + yopNoCap) == 10) break;

    }
    
    TMR0ON = 0;
    TMR0IE = 0;

    Eeprom_WriteByte(1, yopCap);

    displayLog(eskaCap, eskaNoCap, yopCap, yopNoCap, timeElapsed);

}