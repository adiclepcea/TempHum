#include <stdlib.h>
#include <stdio.h>
#ifndef I2C_H
#define I2C_H

#define TRUE 1
#define FALSE 0

void I2CInit(void);
void I2CClose(void);

void I2CStart(void);
void I2CStop(void);

uint8_t I2C_IsBusy(void);

uint8_t GetErr(void);

uint8_t I2CWriteByte(uint8_t data);
uint8_t I2CReadByte(uint8_t *data,uint8_t ack);	

#endif



