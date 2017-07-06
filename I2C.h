#ifndef I2C_H
#define I2C_H

extern unsigned char time[7];

void I2C_Master_Init(const unsigned long c);
void I2C_Master_Write(unsigned d);
unsigned char I2C_Master_Read(unsigned char a);
void printRTC (void);

#endif /* I2C_H */
