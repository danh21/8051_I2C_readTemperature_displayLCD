#include <reg51.h>
#include <absacc.h>	   	// defines XBYTE
#include "intrins.h"	// defines nop



#define REG0 XBYTE[0x8000]	// external data memory at address 8000h   
#define REG1 XBYTE[0x8001]   
#define REG2 XBYTE[0x8002]   

#define uchar unsigned char    

uchar result[16] = {" T = "};  
 
uchar bdata busyFlag;  			// bit-addressable
sbit busyFlag_7 = busyFlag^7; 	// bit 7 of busy flag

sbit SCL = P3^3;				// serial clock    
sbit SDA = P3^4;    			// serial data



///////////////////////////////////// START of I2C ////////////////////////////////////////////
void Start(void)	// high-to-low transition of SDA while SCL is high       
{
	SCL = 1;
	SDA = 1;
    _nop_();		// pause for 1 CPU cycle
    SDA = 0;
    _nop_();
}  



void Stop(void) 	// low-to-high transition of SDA while SCL is high
{
    SCL = 1;
	SDA = 0;
    _nop_();
    SDA = 1;
    _nop_();
}



void ACK(void)		// SDA is 0  
{
    SDA = 0;
    _nop_();
    SCL = 1;
    _nop_();
    SCL = 0;
}



void NACK(void)      
{
    SDA = 1;		// SDA is 1
    _nop_ ();
    SCL = 1;
    _nop_ ();
    SCL = 0;
} 



void Send(uchar Data)		 
{
	uchar i;
		
	for (i = 0; i < 8; i++) { 		// 8 bit
		SCL = 0;
		_nop_();

		if ((Data & 0x80) == 0x80)	// MSB first
		    SDA = 1;
		else
		    SDA = 0;

		Data <<= 1; 

		SCL = 1;
		_nop_();      
	} 
	     
    SCL = 0;
}



uchar Read(void) 
{
	uchar Data = 0, i;

    SDA = 1;
    for (i = 0; i < 8; i++) { 		// 8 bit
    	SCL = 0;
      	_nop_();
		SCL = 1;
		_nop_();

      	if (SDA)      
			Data |= 0x01;
					
	  	if (i < 7)
			Data <<= 1;	  			// MSB first
    }

    return Data;
}
///////////////////////////////////// END of I2C ////////////////////////////////////////////



/////////////////////////////////// Start of DS1621 ///////////////////////////////////////
void initSensor()
{
    Start();
    Send(0x98);		// 1001 - for read and write operations ; 100 - address (proteus) ; 0 - write
    ACK();
    Send(0xEE);		// begins a temperature conversion
    NACK();
    Stop();
}



void readTemp() 
{      
    uchar Data;

    Start(); 
    Send(0x98);
    ACK();
    Send(0xAA);		// reads the last temperature conversion result
    NACK();
	Stop();

    Start();
    Send(0x99);		// 1001 - for read and write operations ; 100 - address (proteus) ; 1 - write
    ACK();
    Data = Read();
    NACK();
    Stop();

    if((Data&0x80) != 0 && Data != 128) { 	// negative value -> MSB = 1
    	result[5] = '-';
        Data = ~(--Data);					// ex: 231 ---> -25
    }
    else									// positive value -> MSB = 0	
        result[5] = '+';

	result[6] = Data/100 + '0';				// hundreds ; '0' --> convert num to char
    result[7] = Data / 10 % 10 + '0';		// dozens
    result[8] = Data % 10 + '0';    		// units
	result[9] = 223; 						// degree
	result[10] = 'C';
}
/////////////////////////////////// END of DS1621 ///////////////////////////////////



/////////////////////////////////// START of LCD ///////////////////////////////////
void busy()        			// check status of LCD
{
	// busyFlag_7 = 1 ---> the system is now internally executing a previously received instruction
    // to read busy flag ---> RS = 0, R/W = 1
	// REG1 has address 01H
	do {
        busyFlag = REG1;
    } while (busyFlag_7);  	
}



void writeCMD(uchar cmd)   
{
	// to send CMD ---> RS = 0, R/W = 0
	// REG0 has address 00H
    busy();
    REG0 = cmd;
}



void initLCD()    	
{
    writeCMD(0x38);		// data length 8bits ; 2 lines ; (5×8 pixel) matrix
    writeCMD(0x01);		// clear screen
    writeCMD(0x06);		// increment cursor (shift cursor to right)
    writeCMD(0x0C);		// display on, cursor off
}



void writeChar(uchar ch)    //write data
{
	// to send data ---> RS = 1, R/W = 0
	// REG2 has address 10H
    busy();
    REG2 = ch;
}



void writeStr(uchar str[]) 
{
    uchar i;
    for(i=0; i<16; i++)
        writeChar(str[i]);
}
/////////////////////////////////// END of LCD ///////////////////////////////////
 


void main()
{
    initLCD();
	initSensor();
	    
    while(1) {       
        readTemp();
        writeCMD(0x80);		// Force cursor to beginning of first line
        writeStr(result);  
    }
}