#include <reg51.h>
#include "intrins.h"	// defines nop


  
#define uchar unsigned char    

uchar result[16] = {" T = "};  

sbit SCL = P3^3;		// serial clock    
sbit SDA = P3^4;   		// serial data

#define LCD P2
sbit RS = P1^2;			// register select
sbit E 	= P1^3; 		// enable



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
    Send(0x90);		// 1001 - for read and write operations ; 000 - address (proteus) ; 0 - write
    ACK();
    Send(0xEE);		// begins a temperature conversion
    NACK();
    Stop();
}



void readTemp() 
{      
    uchar Data;

    Start(); 
    Send(0x90);
    ACK();
    Send(0xAA);		// reads the last temperature conversion result
    NACK();
	Stop();

    Start();
    Send(0x91);		// 1001 - for read and write operations ; 000 - address (proteus) ; 1 - write
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
void msdelay(unsigned int time)	
{										   
    unsigned int i, j;
    for (i = 0; i < time; i++)    
    	for (j = 0; j < 1275; j++);
} 



void writeCMD(uchar cmd)   
{
    RS = 0;
	LCD = cmd;
	E = 1;
	E = 0;
	msdelay(1);
}



void initLCD()    	
{
    writeCMD(0x38);		// data length 8bits ; 2 lines ; (5×8 pixel) matrix
    writeCMD(0x01);		// clear screen
    writeCMD(0x06);		// increment cursor (shift cursor to right)
    writeCMD(0x0C);		// display on, cursor off
}



void writeData(uchar Data)  
{
    RS = 1;
	LCD = Data;
	E = 1;
	E = 0;
	msdelay(1);
}



void writeStr(uchar *str) 
{
    while (*str != '\0')							
		writeData(*str++); 
}
/////////////////////////////////// END of LCD ///////////////////////////////////



void main()
{
    initLCD();
	initSensor();
	
    while(1) {       
        readTemp();
        writeCMD(0x80);	// Force cursor to beginning of first line
        writeStr(result);  
    }
}