#include <stdio.h>
#include <util/twi.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"
#include "usart.c"
#include "I2C.h"
#include "SHT11.h"
#include <avr/eeprom.h>

//#define PORT_ADDR  4	
#define MESSAGEBUF_SIZE 20
#define ADDRESS 5	//adresa modbus
#define t1_5 3
#define t3_5 5 //max pentru 9600 - t3,5 = 3.64583 ms; pentru 4800 - t3,5 = 7.29166

//char buffer[10];
uint8_t rasp_g[10];
uint8_t rasp[10];
int temp_u=0,temp_d=0, hum_u=0,hum_d=0;
int sum_timp = 0;
char readParameter = 'C';
uint8_t temp_min=10,temp_max=35,umid_min=0,umid_max=100;

void readTempHum(void);

//calcularea CRC pentru modbus
unsigned int CRC16(unsigned char *buf, int len)
{  
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++)
  {
  crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

  for (int i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }

  return crc;
}

//raspundem cu eroare de adresa la modbus
void respondAddressError(void){
   rasp_g[1]=0x84; //function response in error
   rasp_g[3]=0x02; //illegal data address
   unsigned int rez = CRC16(rasp_g,3);
   rasp_g[4]=rez & 0xFF;;
   rez>>=8;
   rasp_g[5]=rez;
   for(uint8_t i=0;i<6;i++){
     UWriteData(rasp_g[i]);
   } 
}


//raspundem cu eroare de functie la modbus
void respondFunctionError(unsigned char inputFunction){
   rasp_g[1]=128+inputFunction; //function response in error
   rasp_g[3]=0x01; //illegal function
   unsigned int rez = CRC16(rasp_g,3);
   rasp_g[4]=rez & 0xFF;;
   rez>>=8;
   rasp_g[5]=rez;
   for(uint8_t i=0;i<6;i++){
     UWriteData(rasp_g[i]);
   }
}

void readTempLimits(void){
	temp_min=eeprom_read_byte((uint8_t*)1);
	temp_max=eeprom_read_byte((uint8_t*)2);
	if(temp_min==0xFF){
		temp_min=10;
	}
	if(temp_max==0xFF){
		temp_max=35;
	}
}
void readUmidLimits(void){
	umid_min=eeprom_read_byte((uint8_t*)11);
	umid_max=eeprom_read_byte((uint8_t*)12);
	if(umid_min==0xFF){
		umid_min=0;
	}
	if(umid_max==0xFF){
		umid_max=100;
	}
}

void writeTempLimits(void){
	eeprom_write_byte((uint8_t*)1,temp_min);
	eeprom_write_byte((uint8_t*)2,temp_max);
}

void writeUmidLimits(void){
	eeprom_write_byte((uint8_t*)11,umid_min);
	eeprom_write_byte((uint8_t*)12,umid_max);
}

void listen485(void){ //spunem la chipul 485 ca ascultam
 	_delay_ms(10);
	PORTC&=~(1<<PINC3);	
}

//raspundem la modbus in functie de cerere
void respondModbus(unsigned char *buff){
   PORTC^=(1<<PINC3);
   _delay_ms(10);
   uint8_t pos = 2;
	if(buff[0]!=ADDRESS){ //mesajul nu este pentru adresa noastra  
		return; 
    }else if(buff[1]!=0x04){
		if(buff[1]==0x06){ //ni se cere sa setam limite
			if(buff[3]==0x01){//scriem temp 
				temp_min = buff[4];
				temp_max= buff[5];
				writeTempLimits();
				for(uint8_t i=0;i<8;i++){
					UWriteData(buff[i]);
				}
				listen485();
				return;
			}else if(buff[3]==0x02){//scriem umiditatea
				umid_min = buff[4];
				umid_max= buff[5];
				writeUmidLimits();
				for(uint8_t i=0;i<8;i++){
					UWriteData(buff[i]);
				}
				listen485();
				return;
			}else{ //nu stim ce se doreste
				respondFunctionError(buff[1]);
				listen485();
				return;
			}
		}else if(buff[1]==0x03){//ni se cer valorile setate
			//rasp[0]=ADDRESS;
			rasp[1]=0x03;
			rasp[2]=0x02; //urmeaza 2 bytes
			if(buff[3]==0x01){//temperatura
				rasp[3]=temp_min;
				rasp[4]=temp_max;
			}else{				//umiditatea
				rasp[3]=umid_min;
				rasp[4]=umid_max;
			}
			unsigned int crc=CRC16(rasp,5);
			rasp[5]=crc & 0xFF;
			rasp[6]=crc>>8;
			for(uint8_t i=0;i<7;i++){
				UWriteData(rasp[i]);	
			}
			rasp[1]=0x04;
			listen485();
			return;
							
		}else{
			respondFunctionError(buff[1]);
			listen485();     
			return;//nu, vrem sa citim registrii de input
		}
	}else if(buff[2]!=0){
		//_delay_ms(t3_5);
        
		respondAddressError(); 
		PORTC&=~(1<<PINC3);
		_delay_ms(10);
		return;// noi stim doar de registrii 1 si 2 
	}else if(buff[3]==1 || buff[3]==2){         
		 if(buff[3]==0x2){//citim incepand cu umiditatea
            if(buff[4]!=0x00 || buff[5]!=1){               
				respondAddressError();
				listen485();
				return; //ni se cer 2 registrii incepand cu umiditatea si noi stim doar atat 
            }else{
               rasp[2]=0x02;
               rasp[3]=hum_u>>8;
               rasp[4]=hum_u & 0xFF;
               unsigned int rez = CRC16(rasp,5);
               rasp[5]=rez & 0xFF;
               rez>>=8;
               rasp[6]=rez;
               pos=7;
            }
         }
         else{
            if(buff[4]!=0x00 || buff[5]>2){              
				respondAddressError();  
				listen485();
				return; //ni se cer mai mult de 2 registrii
            }else{
               if(buff[5]==0x01){
                  rasp[2]=0x02; //vin 2 bytes pentru temperatura 
                  rasp[3]=temp_u>>8;
                  rasp[4]=temp_u & 0xFF;
                  unsigned int rez = CRC16(rasp,5);
                  rasp[5]=rez & 0xFF;
                  rez>>=8;
                  rasp[6]=rez;
                  pos=7;
               }else{
                  rasp[2]=0x04; //vin 2 bytes pentru temperatura si doi pentru umiditate
                  rasp[3]=temp_u>>8;
                  rasp[4]=temp_u & 0xFF;
                  rasp[5]=hum_u>>8;
                  rasp[6]=hum_u & 0xFF;
                  unsigned int rez = CRC16(rasp,7);
                  rasp[7]=rez & 0xFF;
                  rez>>=8;
                  rasp[8]=rez;
                  pos=9;
               }
            }
         }
      }
      //_delay_ms(t1_5);  //pentru 9600 - t3,5 = 3.64583 ms; pentru 4800 - t3,5 = 7.29166
      for(uint8_t i=0;i<pos;i++){
         UWriteData(rasp[i]);
      } 
	  listen485();
}

void clearBuffer(char *buffer){
	for(int i=0;i<10;i++){
		buffer[i]='\0';
	}
}

void readTempHum(void){
	float temp_c;
	float humidity;	
	
	// Read values from the sensor
	uint8_t count=0;


	char buffer[10];
	char buffer2[4];
	
	clearBuffer(buffer);
	
	strcpy(buffer,"ERR");
	temp_c = readTemp();
	humidity = readHum();
	temp_u = (int)temp_c;
	temp_d = ((int)(temp_c*100) % (temp_u*100))/10;
	hum_u = (int)humidity;
	hum_d = ((int)(humidity*100) % (hum_u*100))/10;
	if(readParameter=='C'){
		itoa((int)(temp_u),buffer,10);
		itoa((int)(temp_d),buffer2,10);
		strcat(buffer,".");
		strcat(buffer,buffer2);
	}else{
		itoa((int)(hum_u),buffer,10);
		itoa((int)(hum_d),buffer2,10);
		strcat(buffer,".");
		strcat(buffer,buffer2);
	}
	PORTB|=(1<<PINB2);
	
	I2CStart();
	
	if(!I2C_IsBusy()){
	
		count++;		
		uint8_t res=I2CWriteByte(0b00001000);
	
		if(res!=1){			
			//itoa(res,buffer,10);
			//PORTC |=(1<<PINC3);
			//_delay_ms(10);
			//UWriteString("\r\nNu am putut scrie la adresa mentionata");
			//PORTC &=~(1<<PINC3);
			//writeString(buffer);			
		}else{}
		//itoa(count,buffer,10);
		res=I2CWriteByte(readParameter);				
		if(readParameter=='C' && strstr(buffer,"-42.")){
			strcpy(buffer,"ERR");
		}else if(readParameter=='U' && strstr(buffer,"-13.")){
			strcpy(buffer,"ERR");
		}
		for(int i=0;i<4;i++){
			res=I2CWriteByte(buffer[i]);				
		}
		buffer[5]='\0';
		res=I2CWriteByte('\0');
		
		
		//PORTC |=(1<<PINC3);
		//_delay_ms(5);		
		//UWriteData(readParameter);
		//UWriteString(buffer);
		//UWriteString("\r\n\0");
		//_delay_ms(10);
		//PORTC &=~(1<<PINC3);
		
		if(readParameter=='C'){
			readParameter='U';
		}else{
			readParameter='C';
		}
		I2CStop();
		
		clearBuffer(buffer);
		//I2CClose();
	}else{
		PORTC |=(1<<PINC3);
		_delay_ms(5);
		//UWriteString("\r\nNu am putut scrie adresa");
		PORTC &=~(1<<PINC3);
		_delay_ms(5);
	}
	PORTB^=(1<<PINB2);
}


int main(void)
 {
	
	rasp_g[0]=ADDRESS;
	rasp[0]=ADDRESS; //adresa
	rasp[1]=0x04; //citire registrii de input
		
	USARTInit(103);
	
	PORTC|=(1<<PINC4)|(1<<PINC5);
	DDRC|=(1<<PINC3);
	PORTC &=~(1<<PINC3);
	
	DDRB |= (1<<PINB1) | (1<<PINB2); //PINB1 pentru alarma si PINB2 pentru led
	PORTB &= ~(1<<PINB1);
	PORTB &= ~(1<<PINB2);
	
	readUmidLimits();
	readTempLimits();

	I2CInit();
	
	unsigned char *b;
	unsigned char buff[10];
	
	while(1){		
		_delay_us(100);
		
		if(UDataAvailable()){
			b = &buff[0];
			while(UDataAvailable()){
				*b=UReadData();
				
				*b++;    
				if(b==&buff[0]+1 && buff[0]>0xF0){
					*b--;
				}
				if(b>(&buff[0]+8)){ //avem destule caractere in mesaj
					UFlushBuffer();
					break;
				}
				
				//UWriteData(*b);
				_delay_ms(t1_5); //timpul inter caracter
			}
			
			
			if(b<(&buff[0]+8)){ //nu avem destule caractere in mesaj
				
			}else{
				unsigned int rez = CRC16(buff,6);
				if((rez & 0xFF)!=(int)buff[6]){ //daca CRC-ul nu este corect, ignoram mesajul
					
				}else{
					
					if((rez >> 8)!=buff[7]){	//daca CRC-ul nu este corect, ignoram mesajul
						
					}else{			
						if(buff[0]==ADDRESS){
							//readTempHum();
							//sum_timp=0;
							respondModbus(buff);
						}
					}
				}
			}
			
			
		}
		UFlushBuffer();
		if(++sum_timp>=20000){
			readTempHum();
			if((temp_u<temp_min) || (temp_u>temp_max) || (hum_u<umid_min) || (hum_u>umid_max)){//daca sunt depasite limitele
				PORTB|=(1<<PINB1);
				PORTB|=(1<<PINB2);
				//for(uint8_t i=0;i<5;i++){
				//	PORTB|=(1<<PINB2);
				//	_delay_ms(200);
				//	PORTB&=~(1<<PINB2);
				//	_delay_ms(200);
				//}
			}else{
				PORTB&=~(1<<PINB1);
				PORTB&=~(1<<PINB2);
			}
			sum_timp=0;
		}	
    }
 }