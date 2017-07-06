/* 
 * File:                logs.h
 * Author:              Cindy Lian
 *
 * Created on:          February 25, 2017, 10:53 PM
 */

#ifndef LOGS_H
#define	LOGS_H

void displayLog(int eskaCap, int eskaNoCap, int yopCap, int yopNoCap, int time);
void showLogMenu (void);
unsigned short int Eeprom_ReadByte (unsigned short int address);
void Eeprom_WriteByte (unsigned short int address, unsigned short int data);


#endif	/* LOGS_H */
