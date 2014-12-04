/*
 * MCP79410.h
 *
 * Created: 06.11.2013 18:52:57
 *  Author: Julian
 *
 *	Ansteuerung der RealTimeClock MCP79410 von Microchip
 *	Kommunikation über I2C
 *	Entworfen und getestet mit ATMega328P @8MHz	
 *
 *	Werte sind BCD kodiert
 *
 *	  Funktion	     SRAM ADRESSEN
 
 *	Zeit + Datum	: 0x00 - 0x06
 *	Cotrol Reg		: 0x07 - 0x09
 *	Alarm0 Reg		: 0x0A - 0x10
 *	Alarm1 Reg		: 0x11 - 0x13
 *	
 */ 


#ifndef MCP79410_H_
#define MCP79410_H_

/*	Device & controll adress */
#define EEPROM_write 0b10101110
#define EEPROM_read  0b10101111

#define SRAM_write 0b11011110
#define SRAM_read  0b11011111

/* data adress	*/

#define ADD_time_sec 0x00
#define ADD_time_min 0x01
#define ADD_time_hour 0x02
#define ADD_time_day 0x03	//weekday (1-7)
#define ADD_time_date 0x04	//day (1-31)
#define ADD_time_month 0x05
#define ADD_time_year 0x06	//0-99

#define ADD_control 0x07

/*	alarm 0 address	*/
#define ALARM0_sec 0x0A
#define ALARM0_min 0x0B
#define ALARM0_weekday 0x0D	//!and config register!
#define ALARM0_hour 0x0C
#define ALARM0_date 0x0E
#define ALARM0_month 0x0F

/*  0x0D	*/
#define ALM0IF 3
#define ALM0POL 7
/*	0x07	*/
#define ALM0 4


void RTC_setTime(uint8_t hour,uint8_t minute,uint8_t second);
void RTC_setDate(uint8_t date, uint8_t month, uint8_t year);	//year: 13 not 2013!
void RTC_setDay(uint8_t day);	//1-7	1=Monday 7=Sunday

uint8_t RTC_getTime_Second();
uint8_t RTC_getTime_Minute();
uint8_t RTC_getTime_Hour();

uint8_t RTC_getTime_WeekDay();
uint8_t RTC_getTime_Day();
uint8_t RTC_getTime_Month();
uint8_t RTC_getTime_Year();

void RTC_enableBat(uint8_t batEn);	//1=enabled 0=disabled
uint8_t RTC_start(uint8_t start);

uint8_t decodeBCD(uint8_t BCDvalue,uint8_t mask);
uint8_t toBCD(uint8_t zahl);

uint8_t SRAMWriteByte(uint8_t adress,uint8_t data);
uint8_t SRAMReadByte(uint8_t adress);

void RTC_setAlert0(void);

/*	funktions	*/

void RTC_setAlert0()
{
	SRAMWriteByte(0x0B,0x10); //minute
	SRAMWriteByte(0x0C,0x18); //hour
	SRAMWriteByte(0x0E,0x07); //date
	SRAMWriteByte(0x0F,0x11); //month 	
 	
	
	SRAMWriteByte(0x0D,0b11110011); //match at matchAll day=3
	 //SRAMWriteByte(0x0D,0b00010011); //alarm0 configure (pol=1 match:minutes)
	 
	SRAMWriteByte(0x07,0b00010000); //alarm0 ON
}

void RTC_setTime(uint8_t hour,uint8_t minute,uint8_t second)
{
	SRAMWriteByte(0x00,toBCD(second));
	SRAMWriteByte(0x01,toBCD(minute));	//minute
	SRAMWriteByte(0x02,toBCD(hour));	//hour
	//SRAMWriteByte(0x03,0x01); //day
	
	
	//SRAMWriteByte(0x00,(toBCD(second) & (1<<7)) );	//seconds,0	and start counting!

	
}
void RTC_setDate(uint8_t date, uint8_t month, uint8_t year)
{
	SRAMWriteByte(ADD_time_date,toBCD(date));	// tag
	SRAMWriteByte(ADD_time_month,toBCD(month)); //	monat
	SRAMWriteByte(ADD_time_year,toBCD(year));	//	jahr
}
void RTC_setDay(uint8_t day)
{
	uint8_t configByte;
	configByte = SRAMReadByte(0x03);
	configByte &= 0xF8; //clear firt 3 bits
	configByte |= (day & 0x07); //mask the first 3 bit
	
	SRAMWriteByte(0x03,configByte);
}

uint8_t RTC_getTime_Second()
{
	return decodeBCD(SRAMReadByte(ADD_time_sec),0x7F);
}
uint8_t RTC_getTime_Minute()
{
	return decodeBCD(SRAMReadByte(ADD_time_min),0x7F);
}
uint8_t RTC_getTime_Hour()
{
	return decodeBCD(SRAMReadByte(ADD_time_hour),0x3F);
}

uint8_t RTC_getTime_WeekDay()
{
	return decodeBCD(SRAMReadByte(ADD_time_day),0x07);	
}
uint8_t RTC_getTime_Day()
{
	return decodeBCD(SRAMReadByte(ADD_time_date),0x3F);	
}
uint8_t RTC_getTime_Month()
{
	return decodeBCD(SRAMReadByte(ADD_time_month),0x1F);
}
uint8_t RTC_getTime_Year()
{
	return decodeBCD(SRAMReadByte(ADD_time_year),0xFF);
}

void RTC_enableBat(uint8_t batEn)	//1=enabled 0=disabled
{
	uint8_t configByte;
	configByte = SRAMReadByte(0x03);
	if(batEn)
		configByte |= (1<<3);
	else
		configByte &= ~(1<<3);
		
	SRAMWriteByte(0x03,configByte);
}
uint8_t RTC_start(uint8_t start)
{
	uint8_t configByte;
	configByte = SRAMReadByte(0x00);
	if(start)
		configByte |= (1<<7);
	else
		configByte &= ~(1<<7);
		
	SRAMWriteByte(0x00,configByte);
	return TRUE;
}

uint8_t decodeBCD(uint8_t BCDvalue,uint8_t mask)
{
	uint8_t retValue;
	
	char one;
	char ten;
	
	BCDvalue = BCDvalue & mask;
	
	one = BCDvalue & 0x0F;	//lower 4 bits
	ten = (BCDvalue>>4) & 0x0F;		//higher bits
	
	retValue = ten * 10 + one;
	
	return retValue;
}
uint8_t toBCD(uint8_t zahl)
{
	uint8_t zehner,einer,bcd_zahl;
	zehner = 0;
	einer = 0;
	bcd_zahl = 0;
	
	if(zahl < 60)
	{
		while(zahl>9)
		{
			zahl = zahl - 10;
			zehner ++;
		}
		einer = zahl;
	
		bcd_zahl = (zehner<<4);
		bcd_zahl |= einer;
	
		return bcd_zahl;
	}	
	else
		return 0;
}

uint8_t SRAMReadByte(uint8_t address)
{
	
	/*	Start
		CTRL EEPROM_write
		->ack
		adress
		->ack
		Start
		CTRL EEPROM_read
		->receive byte
		->no ack
		Stop	
	*/
	uint8_t data;

	do
	{
		//Put Start Condition on TWI Bus
		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

		//Poll Till Done
		while(!(TWCR & (1<<TWINT)));

		//Check status
		if((TWSR & 0xF8) != 0x08)
			return FALSE;

		//Now write SLA+W
		//EEPROM @ 00h
		TWDR = SRAM_write;
		//TWDR = EEPROM_write;
		//TWDR=0b10100000;	

		//Initiate Transfer
		TWCR=(1<<TWINT)|(1<<TWEN);

		//Poll Till Done
		while(!(TWCR & (1<<TWINT)));
	
	}while((TWSR & 0xF8) != 0x18);
		
	//Now write ADDR
	TWDR=(address);

	//Initiate Transfer
	TWCR=(1<<TWINT)|(1<<TWEN);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) != 0x28)
		return FALSE;

	//Put Start Condition on TWI Bus
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) != 0x10)
		return FALSE;

	//Now write EEPROM+READ
	TWDR = SRAM_read;

	//Initiate Transfer
	TWCR=(1<<TWINT)|(1<<TWEN);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) != 0x40)
		return FALSE;

	//Now enable Reception of data by clearing TWINT
	TWCR=(1<<TWINT)|(1<<TWEN);

	//Wait till done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) != 0x58)
		return FALSE;

	//Read the data
	data=TWDR;

	//Put Stop Condition on bus
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	
	//Wait for STOP to finish
	while(TWCR & (1<<TWSTO));

	//Return TRUE
	return data;
}
uint8_t SRAMWriteByte(uint8_t address,uint8_t data)
{
	do
	{
		//Put Start Condition on TWI Bus
		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
		
		//Poll Till Done
		while(!(TWCR & (1<<TWINT)));

		//Check status
		if((TWSR & 0xF8) != 0x08)
			return FALSE;

		//Now write RTC_SRAM+WRITE
		TWDR = SRAM_write;

		//Initiate Transfer
		TWCR=(1<<TWINT)|(1<<TWEN);

		//Poll Till Done
		while(!(TWCR & (1<<TWINT)));
	
	}while((TWSR & 0xF8) != 0x18);
		

	//Now write ADDR
	TWDR=(address);

	//Initiate Transfer
	TWCR=(1<<TWINT)|(1<<TWEN);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) != 0x28)
		return FALSE;

	//Now write DATA
	TWDR=(data);

	//Initiate Transfer
	TWCR=(1<<TWINT)|(1<<TWEN);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) != 0x28)
		return FALSE;

	//Put Stop Condition on bus
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	
	//Wait for STOP to finish
	while(TWCR & (1<<TWSTO));

	//Wait untill Writing is complete
	_delay_ms(12);

	//Return TRUE
	return TRUE;

}


#endif /* MCP79410_H_ */