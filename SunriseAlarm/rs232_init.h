/*
 * rs232_init.h
 *
 * Created: 03.05.2012 18:41:05
 *  Author: Julian Hauser
 *
 *	Funktionen:
 *		USARTReadChar()  // return char
 *		USARTWriteChar(char data) // char
 *
 *		sende(const char *s)	//sende array von char
 *		sendeInt(int)			//sende array von int
 *
 *	 im Hauptprogramm ausführen:    
 *
	#include <inttypes.h>
	#include <stdlib.h>

	#include "rs232_init.h"
	
	//in main:
	USARTInit(MYUBRR);  //15 == UBRR
	
	#define BAUD 9600
	#define MYUBRR F_CPU/16/BAUD-1
 * 
*/

#define NL "\r\n"  //NewLine

char rx_data; // wird gesendet


   
//initialize the USART
void USARTInit(uint16_t ubrr_value)
{
   //Set Baud rate
   UBRR0L = ubrr_value;
   UBRR0H = (ubrr_value>>8);

   /*Set Frame Format
   >> Asynchronous mode
   >> No Parity
   >> 1 StopBit
   >> char size 8
   */
   UCSR0C=(1<<USBS0)|(3<<UCSZ00);

   //Enable The receiver and transmitter
   UCSR0B=(1<<RXEN0)|(1<<TXEN0);
}


//gebe empfangenes Zeichen zurück (char)
//rx_data = USARTReadChar();
char USARTReadChar()
{
   //Wait untill a data is available
   while(!(UCSR0A & (1<<RXC0)))
   {	/*Do nothing*/   }

   //Now USART has got data from host
   return UDR0;
}


//sende einzeknen char
void USARTWriteChar(char rx_data)
{
   //Wait untill the transmitter is ready

   while(!(UCSR0A & (1<<UDRE0)))
   {	/*Do nothing*/	}

   //Now write the data to USART buffer
   UDR0=rx_data;
}


// sende ein Array von Chars
// sende("hello world");
void sende (const char *s)
{
    do
    {
        USARTWriteChar(*s);
		*s++;
    }
    while (*s);
}

//sende Int als char (ASCII)
void sendeInt(const int *i)
{
	char Buffer[20]; // in diesem {} lokal
	itoa(i, Buffer, 10 ); 
	sende(Buffer);
}