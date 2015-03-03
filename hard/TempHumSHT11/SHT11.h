//library adapted after the arduino library for SHT11
//below you can find the copyright for the arduino library
/**Copyright 2009 Jonathan Oxer <jon@oxer.com.au> / <www.practicalarduino.com>
Copyright 2008 Maurice Ribble <ribblem@yahoo.com> / <www.glacialwanderer.com>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
http://www.gnu.org/licenses/

*/
//****************************************************
//and below the info of the arduino library
/**
 * SHT1x Library
 *
 * Copyright 2009 Jonathan Oxer <jon@oxer.com.au> / <www.practicalarduino.com>
 * Based on previous work by:
 *    Maurice Ribble: <www.glacialwanderer.com/hobbyrobotics/?p=5>
 *    Wayne ?: <ragingreality.blogspot.com/2008/01/ardunio-and-sht15.html>
 *
 * Manages communication with SHT1x series (SHT10, SHT11, SHT15)
 * temperature / humidity sensors from Sensirion (www.sensirion.com).
 */


#include <avr/io.h>
#include <util/delay.h>

#define DATA PIND2
#define CLOCK PIND3
#define DATAPORT PORTD
#define CLOCKPORT PORTD
#define DATAPIN PIND
#define DATADDR DDRD
#define CLOCKDDR DDRD


float readTemp(void);
float readHum(void);
float readTempRaw(void);
int shiftIn(int numBits);
void sendCommandSHT(int _command);
void waitForResultSHT(void);
int getData16SHT(void);
void skipCrcSHT(void);