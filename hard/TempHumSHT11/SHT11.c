#include "SHT11.h"

float readTemp(void)
{
  int val;                // Raw value returned from sensor
  float temperature;      // Temperature derived from raw value

  // Conversion coefficients from SHT15 datasheet
  const float D1 = -40.0;  // for 14 Bit @ 5V
  const float D2 =   0.01; // for 14 Bit DEGC

  // Fetch raw value
  val = readTempRaw();

  // Convert raw value to degrees Celsius
  temperature = (val * D2) + D1;

  return (temperature);
}

float readHum(void)
{
  int val;                    // Raw humidity value returned from sensor
  float linearHumidity;       // Humidity with linear correction applied
  float correctedHumidity;    // Temperature-corrected humidity
  float temperature;          // Raw temperature value

  // Conversion coefficients from SHT15 datasheet
  const float C1 = -4.0;       // for 12 Bit
  const float C2 =  0.0405;    // for 12 Bit
  const float C3 = -0.0000028; // for 12 Bit
  const float T1 =  0.01;      // for 14 Bit @ 5V
  const float T2 =  0.00008;   // for 14 Bit @ 5V

  // Command to send to the SHT1x to request humidity
  int _gHumidCmd = 0b00000101;

  // Fetch the value from the sensor
  sendCommandSHT(_gHumidCmd);
  waitForResultSHT();
  val = getData16SHT();
  skipCrcSHT();

  // Apply linear conversion to raw value
  linearHumidity = C1 + C2 * val + C3 * val * val;

  // Get current temperature for humidity correction
  temperature = readTemp();

  // Correct humidity value for current temperature
  correctedHumidity = (temperature - 25.0 ) * (T1 + T2 * val) + linearHumidity;

  return (correctedHumidity);
}


/* ================  Private methods ================ */

/**
 * Reads the current raw temperature value
 */
float readTempRaw(void)
{
  int val;

  // Command to send to the SHT1x to request Temperature
  int _gTempCmd  = 0b00000011;

  sendCommandSHT(_gTempCmd);
  waitForResultSHT();
  val = getData16SHT();
  skipCrcSHT();

  return (val);
}

/**
 */
 
 
int shiftIn(int numBits)
{
  int ret = 0;
  int i;

  for (i=0; i<numBits; ++i)
  {
	CLOCKPORT |= (1<<CLOCK);
    _delay_ms(10);  // I don't know why I need this, but without it I don't get my 8 lsb of temp
    ret = ret*2 + ((DATAPIN & (1 << DATA))?1:0);
    CLOCKPORT &=~(1<<CLOCK);
  }

  return(ret);
}

void sendCommandSHT(int _command)
{
	int ack;
	
	DATADDR |= (1<<DATA);
	CLOCKDDR |=(1<<CLOCK);
  // Transmission Start
	DATAPORT |= (1<<DATA);
	CLOCKPORT |= (1<<CLOCK);
	DATAPORT &=~(1<<DATA);
	CLOCKPORT &=~(1<<CLOCK);
	CLOCKPORT |= (1<<CLOCK);
	DATAPORT |= (1<<DATA);
	CLOCKPORT &=~(1<<CLOCK);

  // The command (3 msb are address and must be 000, and last 5 bits are command)
//  shiftOut(_dataPin, _clockPin, MSBFIRST, _command);
  for (int i=0x80;i>0;i/=2)             //shift bit for masking
  { 
    if (i & _command) 
		DATAPORT |= (1<<DATA);
    else 
		DATAPORT &=~(1<<DATA);
	
	CLOCKPORT |= (1<<CLOCK);
    
	CLOCKPORT &=~(1<<CLOCK);    
  }


  // Verify we get the correct ack
  CLOCKPORT |= (1<<CLOCK);
  DATADDR &=~(1<<DATA); //we put data pin in input mode
  ack = (DATAPIN & (1 << DATA));//bit_is_set(DATAPORT,DATA);
  if (ack) {
    //Serial.println("Ack Error 0");
  }
  CLOCKPORT &=~(1<<CLOCK);
  
  ack = (DATAPIN & (1 << DATA)); //bit_is_set(DATAPORT,DATA);
  if (!ack) {
    //Serial.println("Ack Error 1");
  }
  
}

/**
 */
void waitForResultSHT(void)
{
  int i;
  int ack;

  DATADDR &=~(1<<DATA); //we put data pin in input mode

  for(i= 0; i < 100; ++i)
  {
    _delay_ms(10);
    ack = (DATAPIN & (1 << DATA));//bit_is_set(DATAPORT,DATA);

    if (!ack) {
      break;
    }
  }

  if (ack) {
    //Serial.println("Ack Error 2"); // Can't do serial stuff here, need another way of reporting errors
  }
}

int getData16SHT(void)
{
  int val;

  // Get the most significant bits
  DATADDR &=~(1<<DATA); //we put data pin in input mode
  CLOCKDDR |= (1<<CLOCK); //we put clock pin in output mode
  
  val = shiftIn(8);
  val *= 256;

  // Send the required ack
  DATADDR |=(1<<DATA); //we put data pin in output mode
  DATAPORT |= (1<<DATA);
  DATAPORT &=~(1<<DATA);
  CLOCKPORT |= (1<<CLOCK);  
  CLOCKPORT &=~(1<<CLOCK);  

  // Get the least significant bits
  DATADDR &=~(1<<DATA); //we put data pin in input mode
  val |= shiftIn(8);

  return val;
}

void skipCrcSHT(void)
{
  // Skip acknowledge to end trans (no CRC)
  DATADDR |= (1<<DATA); //we put data pin in output mode
  CLOCKDDR |= (1<<CLOCK); //we put clock pin in output mode

  DATAPORT |= (1<<DATA);
  CLOCKPORT |= (1<<CLOCK);
  CLOCKPORT &=~(1<<CLOCK);  
}

