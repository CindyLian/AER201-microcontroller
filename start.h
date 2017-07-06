/* 
 * File:                start.h
 * Author:              Cindy Lian
 *
 * Created on:          February 25, 2017, 10:53 PM
 */

#ifndef START_H
#define	START_H

extern int eskaCap;
extern int eskaNoCap;
extern int yopCap;
extern int yopNoCap;
extern int timeElapsed;

void resetCounts(void);
void sort (void);
int ultrasonicFindBottle (void);
void rotate90 (void);
void readADC(char channel);
void dc_stop (void);
void dc_forward (void);
void dc_reverse (void);

#endif	/* START_H */
