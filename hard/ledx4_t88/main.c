#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <string.h>
#include "TWI_slave.h"

#define PORT_ADDR  4

#define MESSAGEBUF_SIZE       20

#define E PINB4
#define D PINC2
#define DP PINC0
#define C PINC1
#define G PIND3
#define B PINC3
#define F PINB0
#define A PIND7
#define COM1 PIND6
#define COM2 PINB1
#define COM3 PINB2
#define COM4 PINB3
#define Close_1() (PORTD &=~(1<<COM1))
#define Close_2() (PORTB &=~(1<<COM2))
#define Close_3() (PORTB &=~(1<<COM3))
#define Close_4() (PORTB &=~(1<<COM4))
#define Open_1() ({Close_2();Close_3();Close_4();PORTD |=(1<<COM1);})
#define Open_2() ({Close_1();Close_3();Close_4();PORTB |=(1<<COM2);})
#define Open_3() ({Close_1();Close_2();Close_4();PORTB |=(1<<COM3);})
#define Open_4() ({Close_1();Close_2();Close_3();PORTB |=(1<<COM4);})
#define Close_A() (PORTD |= 1<<A)
#define Close_B() (PORTC |= 1<<B)
#define Close_C() (PORTC |= 1<<C)
#define Close_D() (PORTC |= 1<<D)
#define Close_E() (PORTB |= 1<<E)
#define Close_F() (PORTB |= 1<<F)
#define Close_G() (PORTD |= 1<<G)
#define Close_DP() (PORTC |= 1<<DP)
#define Open_A() (PORTD &=~(1<<A))
#define Open_B() (PORTC &=~(1<<B))
#define Open_C() (PORTC &=~(1<<C))
#define Open_D() (PORTC &=~(1<<D))
#define Open_E() (PORTB &=~(1<<E))
#define Open_F() (PORTB &=~(1<<F))
#define Open_G() (PORTD &=~(1<<G))
#define Open_DP() (PORTC &=~(1<<DP))

char err[5];
void writeErr(char *cIn){
	strcpy(err,cIn);
}

void TWI_Act_On_Failure_In_Last_Transmission ( unsigned char TWIerrorMsg )
{
// A failure has occurred, use TWIerrorMsg to determine the nature of the failure
// and take appropriate actions.
// See header file for a list of possible failure messages.

// Error codes are just the state codes defined in TWI_Master.h. These tend to be
//	rather large to decode from a blinking LED. Since there are really only a few
//	that we care about, use a SWITCH/CASE to put out easy codes.
// Note that Receive Data NAck from Slave is expected so is not an error.
	switch (TWIerrorMsg & 0xf8)
	{	
		case TWI_MTX_ADR_NACK:		// No Transmit Address Ack from Slave
			writeErr("TANA");//"MTX ADR NACK\r\n");
		break;
		case TWI_MTX_DATA_NACK:		// No Transmit Data Ack from Slave
			writeErr("TDNA");//MTX DATA NACK\r\n");
		break;		
		case TWI_MRX_ADR_NACK:		// No Receive Address Ack from Slave
			writeErr("RANA");//MRX ADR NACK\r\n");
		break;		
		case TWI_ARB_LOST:			// Arbitration Lost -- How?
			writeErr("ARBL");//ARB LOST\r\n");
		break;
		case TWI_NO_STATE:			// No State -- What happened?
			writeErr("NSTA");//NO STATE");
		break;
		case TWI_BUS_ERROR:			// Bus Error
			writeErr("BERR");//BUS ERROR\r\n");
		break;
		default:					// Anything Else - further decoding is possible
			writeErr("ERR");//??\r\n");
	}
	TWI_Start_Transceiver();
}

void clearMessageBuffer(char *messageBuf ){
	for(int i=0;i<MESSAGEBUF_SIZE ;i++){
		*messageBuf++='\0';
	}
}

void CloseAllLetters(void){
	Close_A();
	Close_B();
	Close_C();
	Close_D();
	Close_E();
	Close_F();
	Close_G();
	Close_DP();
}

void WriteLetter(char c){
	CloseAllLetters();
	switch(c){
		case 'A':
			Open_E();
			Open_F();
			Open_A();
			Open_B();
			Open_C();
			Open_G();
			break;
		case 'B':
			Open_E();
			Open_F();
			Open_C();
			Open_G();
			Open_D();
			break;
		case 'C':
			Open_G();
			Open_E();
			Open_D();			
			break;
		case 'D':
			Open_B();
			Open_C();
			Open_D();
			Open_E();
			Open_G();
			break;
		case 'E':
			Open_A();
			Open_F();
			Open_E();
			Open_D();
			Open_G();
			break;
		case 'F':
			Open_A();
			Open_F();
			Open_E();
			Open_G();
			break;
		case 'G':
			Open_A();
			Open_C();
			Open_D();
			Open_E();
			Open_F();
			break;
		case 'H':
			Open_B();
			Open_C();
			Open_E();
			Open_F();
			Open_G();
			break;
		case 'I':
			Open_A();
			Open_B();			
			break;
		case 'J':
			Open_A();
			Open_B();
			Open_C();
			Open_D();
			Open_E();
			break;
		case 'K':
			Open_B();
			Open_C();
			Open_E();
			Open_F();
			Open_G();
			break;
		case 'L':
			Open_D();
			Open_E();
			Open_F();
			break;
		case 'M':
			Open_C();
			Open_E();
			Open_G();
			break;
		case 'N':
			Open_C();
			Open_E();
			Open_G();
			break;
		case 'O':
			Open_C();
			Open_D();
			Open_E();
			Open_G();
			break;
		case 'P':
			Open_A();
			Open_B();
			Open_E();
			Open_F();
			Open_G();
			break;
		case 'R':
			Open_E();
			Open_G();
			break;
		case 'S':
			Open_A();
			Open_C();
			Open_D();
			Open_F();
			Open_G();
			break;
		case 'T':			
			Open_D();
			Open_E();
			Open_F();
			Open_G();
			break;
		case 'U':
			Open_C();
			Open_D();
			Open_E();
			break;
		case 'V':
			Open_C();
			Open_D();
			Open_E();			
			break;
		case 'X':
			Open_B();
			Open_C();
			Open_E();
			Open_F();
			Open_G();
			break;
		case 'Y':
			Open_B();
			Open_C();
			Open_D();
			Open_F();
			Open_G();
			break;
		case 'Z':
			Open_A();
			Open_B();
			Open_D();
			Open_E();
			Open_G();
			break;
		case '1':
			Open_B();
			Open_C();			
			break;
		case '2':
			Open_A();
			Open_B();
			Open_D();
			Open_E();
			Open_G();
			break;
		case '3':
			Open_A();
			Open_B();
			Open_C();
			Open_D();
			Open_G();
			break;
		case '4':
			Open_B();
			Open_C();
			Open_F();
			Open_G();
			break;
		case '5':
			Open_A();
			Open_C();
			Open_D();
			Open_F();
			Open_G();
			break;
		case '6':
			Open_A();
			Open_C();
			Open_F();
			Open_D();
			Open_E();
			Open_G();
			break;
		case '7':
			Open_A();
			Open_B();
			Open_C();
			break;
		case '8':
			Open_A();
			Open_B();
			Open_C();
			Open_D();
			Open_E();
			Open_F();
			Open_G();
			break;
		case '9':
			Open_A();
			Open_B();
			Open_C();
			Open_D();
			Open_F();
			Open_G();
			break;
		case '0':
			Open_A();
			Open_B();
			Open_C();
			Open_D();
			Open_E();
			Open_F();
			break;
		default:
			break;
	}
}

void Open_Pos(uint8_t pos){
	switch(pos){
		case 0:
			Open_1();
			break;
		case 1:
			Open_2();
			break;
		case 2:
			Open_3();
			break;
		case 3:
			Open_4();
			break;
		default:
			break;
	}
}

void WriteString(char *cIn){
	uint8_t iPos=0;
	
	while(iPos<4 && *cIn){
		Open_Pos(iPos++);
		WriteLetter(*cIn++);
		if(*cIn=='.'){
			Open_DP();
			cIn++;
		}
		_delay_ms(4);		
	}
}

int main(void){
	
	DDRD |= 0b11111111;//(1<<G) | (1<<A) | (1<<COM1); //definim ca iesiri pinii folositi
	DDRB |= 0b11111111;//(1<<E) | (1<<F) | (1<<COM2) | (1<<COM3) | (1<<COM4);
	DDRC |= 0b11111111;//(1<<D) | (1<<DP) | (1<<C) | (1<<B);
	
	PORTC|=(1<<PINC4)|(1<<PINC5); // pull up pentru porturile pentru i2c
	char messageBuf[MESSAGEBUF_SIZE]; //bufferul pentru i2c
	unsigned char TWI_SlaveAddress; //adresa i2c
	
	clearMessageBuffer(messageBuf);
	
	TWI_SlaveAddress   = PORT_ADDR;	
  
	TWI_Slave_Initialise( (unsigned char)(0b00001001));  //adresa 4
	
	sei();
	
	TWI_Start_Transceiver();
	
	strcpy(messageBuf,"H234");
	
	while(1){
		if ( ! TWI_Transceiver_Busy() )	{
			if ( TWI_statusReg.lastTransOK ){
				if ( TWI_statusReg.RxDataInBuf ){
					for(int i=0;i<MESSAGEBUF_SIZE ;i++){
						messageBuf[i]='\0';
					}
					TWI_Get_Data_From_Transceiver(messageBuf, MESSAGEBUF_SIZE);
					
					TWI_Clear_Buffer();
					//WriteString(messageBuf);
					
				}
				//if ( ! TWI_Transceiver_Busy() )
				//{
				//	TWI_Start_Transceiver();
				//	for(int i=0;i<MESSAGEBUF_SIZE ;i++){
				//		messageBuf[i]='\0';
				//	}
				//	TWI_Clear_Buffer();
				//}
			}
			else{				
				//TWI_Act_On_Failure_In_Last_Transmission( TWI_Get_State_Info() );
				//strcpy(messageBuf,err);
				
			}			
			
		}
		WriteString(messageBuf);
		
	}
	
	return 0;
}