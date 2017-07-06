/* 
 * File:                logs.c
 * Author:              Cindy Lian
 *
 * Created on:          February 25, 2017, 10:53 PM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"

const char keys[] = "123A456B789C*0#D"; //list of keypad buttons
const char itemArray [] = "12345";

void displayLog (int eskaCap, int eskaNoCap, int yopCap, int yopNoCap, int time)
{
    int total = eskaCap + eskaNoCap + yopCap + yopNoCap;
    lcdInst (0b10000000);
    printf ("Total:%d Time:%d", total, time);
    lcdInst(0b11000000); //go to second line
    printf("ec:%den:%dyc:%dyn:%d", eskaCap, eskaNoCap, yopCap, yopNoCap);
    while (1){
        
    while(PORTBbits.RB1 == 0){ }//wait for press
    unsigned char press1 =(PORTB & 0xF0)>>4; // Read the 4 bit character code
    while(PORTBbits.RB1 == 1){} //wait for release
    if (keys[press1] == 'C') return;
    
    }
}

unsigned short int Eeprom_ReadByte(unsigned short int address)
{

    // Set address registers
    EEADRH = (unsigned short int)(address >> 8);
    EEADR = (unsigned short int)address;

    EECON1bits.EEPGD = 0;       // Select EEPROM Data Memory
    EECON1bits.CFGS = 0;        // Access flash/EEPROM NOT config. registers
    EECON1bits.RD = 1;          // Start a read cycle

    // A read should only take one cycle, and then the hardware will clear
    // the RD bit
    while(EECON1bits.RD == 1);

    return EEDATA;              // Return data

}

//! @brief      Writes a single byte of data to the EEPROM.
//! @param      address     The EEPROM address to write the data to (note that not all
//!                         16-bits of this variable may be supported).
//! @param      data        The data to write to EEPROM.
//! @warning    This function does not return until write operation is complete.
void Eeprom_WriteByte(unsigned short int address, unsigned short int data)
{    
    // Set address registers
    EEADRH = (unsigned int)(address >> 8);
    EEADR = (unsigned int)address;

    EEDATA = data;          // Write data we want to write to SFR
    EECON1bits.EEPGD = 0;   // Select EEPROM data memory
    EECON1bits.CFGS = 0;    // Access flash/EEPROM NOT config. registers
    EECON1bits.WREN = 1;    // Enable writing of EEPROM (this is disabled again after the write completes)

    // The next three lines of code perform the required operations to
    // initiate a EEPROM write
    EECON2 = 0x55;          // Part of required sequence for write to internal EEPROM
    EECON2 = 0xAA;          // Part of required sequence for write to internal EEPROM
    EECON1bits.WR = 1;      // Part of required sequence for write to internal EEPROM

    // Loop until write operation is complete
    while(PIR2bits.EEIF == 0)
    {
        continue;   // Do nothing, are just waiting
    }

    PIR2bits.EEIF = 0;      //Clearing EEIF bit (this MUST be cleared in software after each write)
    EECON1bits.WREN = 0;    // Disable write (for safety, it is re-enabled next time a EEPROM write is performed)
}


void showLogMenu ()
{
    int pointer = 0; //0 for top line, 1 for bottom line
    int topItem = 0; //whichever item (in the log) is at the top of the LCD

    while (1)
    {
        initLCD();
        lcdInst (0b10000000); // top line
        printf ("Item: %c", itemArray[topItem]);
        lcdInst (0b11000000); //bottom line 
        if (topItem != 3)
            printf ("Item: %c", itemArray[topItem+1]);
        else 
            printf ("Exit");
        
        if (pointer == 0)
            lcdInst (0b10001111); //last thingy on the top line
        else
            lcdInst (0b11001111); //last thingy on the bottom line 
                
        putch ('<');
                
        while(PORTBbits.RB1 == 0){ }//wait for press
        unsigned char press1 =(PORTB & 0xF0)>>4; // Read the 4 bit character code
        while(PORTBbits.RB1 == 1){} //wait for release
        Nop();  //Apply breakpoint here because of compiler optimizations
        Nop();
        unsigned char temp2= keys[press1];
                
        if (temp2 == 'A'){ //move up 
            if (pointer == 1){ pointer = 0;} //move pointer to top position
            else{
                if (topItem != 0) { topItem = topItem - 1; } //move list up
            }
        }
        else if (temp2 == 'B'){ //move down   
            if (pointer == 0){ pointer = 1; } //move pointer to bottom position
            else {
                if (topItem != 3) { topItem = topItem + 1; } //move list down
            }            
        }
        else if (temp2 == 'C'){ //select
            if (topItem == 3 & pointer == 1) return; //pointing @ exit option
            else displayLog (0,0,0,0,0);
        }
    }
}