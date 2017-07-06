/* 
 * File:                lcd.h
 * Taken from PICDevBugger sample code
 * Modified by:         Cindy Lian
 * Created on:          February 25, 2017, 10:53 PM
 */

#ifndef LCD_H
#define	LCD_H

void lcdInst(char data);
void lcdNibble(char data);
void putch(char data);
void initLCD(void);

#endif	/* LCD_H */

