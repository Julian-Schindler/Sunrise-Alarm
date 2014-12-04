/*
 * i2c_basic.h
 *
 * Created: 05.11.2013 20:42:10
 *  Author: Julian
 */ 


#ifndef I2C_BASIC_H_
#define I2C_BASIC_H_

#define FALSE 0
#define TRUE 1

// #define EEPROM_write 0b10101110
// #define EEPROM_read  0b10101111
// 
// #define SRAM_write 0b11011110
// #define SRAM_read  0b11011111

void I2CInit(void);

uint8_t I2CWriteByte(uint8_t address,uint8_t data);
uint8_t I2CReadByte(uint8_t address);

void I2CInit(void)
{
	//Set up TWI Module
		
	TWBR = 5;
	//TWBR = 32; //400KHz_SCL @8MHz_CPU
	TWSR &= (~((1<<TWPS1)|(1<<TWPS0)));//prescaler = 1

}

uint8_t I2CWriteByte(uint8_t address,uint8_t data)
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

		//Now write SLA+W
		//TWDR=0b10100000;	
		TWDR = address & 0b11111110; //clear LSB: write operation

		//Initiate Transfer
		TWCR=(1<<TWINT)|(1<<TWEN);

		//Poll Till Done
		while(!(TWCR & (1<<TWINT)));
	
	}while((TWSR & 0xF8) != 0x18);
	
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
uint8_t I2CReadByte(uint8_t address)	
{
	uint8_t data;
	
	//Put Start Condition on TWI Bus
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	//Poll Till Done
	while(!(TWCR & (1<<TWINT)));

	//Check status
	if((TWSR & 0xF8) != 0x10)
		return FALSE;

	//Now write SLA+R
	//TWDR=0b10100001;	
	TWDR = address | 0x01;	//set LSB: read operation

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












// uint8_t I2CStart(void)
// {
// 		//Put Start Condition on TWI Bus
// 		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 
// 		//Check status
// 		if((TWSR & 0xF8) != 0x08)
// 			return FALSE;	
// 		return TRUE;
// }
// void I2CStop(void)
// {
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Wait untill Writing is complete
// 	_delay_ms(12);
// }
// 
// 
// uint8_t I2CWriteByte(uint8_t address,uint8_t data)
// {
// do
// 	{
// 		//Now write SLA+W
// 		//TWDR=0b10100000;	
// 		TWDR = address & 0b11111110; //clear LSB: write operation
// 
// 		//Initiate Transfer
// 		TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 	
// 	}while((TWSR & 0xF8) != 0x18); //get ack
// 	
// 	//Now write DATA
// 	TWDR=(data);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 
// 	//Return TRUE
// 	return TRUE;
// }
// 
// uint8_t I2CReadByte(uint8_t address)	
// {
// 	uint8_t data;
// 	
// 	//Put Start Condition on TWI Bus
// 	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x10)
// 		return FALSE;
// 
// 	//Now write SLA+R
// 	//TWDR=0b10100001;	
// 	TWDR = address | 0x01;	//set LSB: read operation
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x40)
// 		return FALSE;
// 
// 	//Now enable Reception of data by clearing TWINT
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Wait till done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x58)
// 		return FALSE;
// 
// 	//Read the data
// 	data=TWDR;
// 
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Return TRUE
// 	return data;	
// }
// 
// uint8_t EEReadByte(uint16_t address)
// {
// 	uint8_t data;
// 
// 	//Initiate a Dummy Write Sequence to start Random Read
// 	do
// 	{
// 		//Put Start Condition on TWI Bus
// 		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 
// 		//Check status
// 		if((TWSR & 0xF8) != 0x08)
// 			return FALSE;
// 
// 		//Now write SLA+W
// 		//EEPROM @ 00h
// 		TWDR=0b10100000;	
// 
// 		//Initiate Transfer
// 		TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 	
// 	}while((TWSR & 0xF8) != 0x18);
// 		
// 
// // 	//Now write ADDRH
// // 	TWDR=(address>>8);
// // 
// // 	//Initiate Transfer
// // 	TWCR=(1<<TWINT)|(1<<TWEN);
// // 
// // 	//Poll Till Done
// // 	while(!(TWCR & (1<<TWINT)));
// // 
// // 	//Check status
// // 	if((TWSR & 0xF8) != 0x28)
// // 		return FALSE;
// 
// 	//Now write ADDRL
// 	TWDR=(address);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 	//*************************DUMMY WRITE SEQUENCE END **********************
// 
// 
// 	
// 	//Put Start Condition on TWI Bus
// 	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x10)
// 		return FALSE;
// 
// 	//Now write SLA+R
// 	//EEPROM @ 00h
// 	TWDR=0b10100001;	
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x40)
// 		return FALSE;
// 
// 	//Now enable Reception of data by clearing TWINT
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Wait till done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x58)
// 		return FALSE;
// 
// 	//Read the data
// 	data=TWDR;
// 
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Return TRUE
// 	return data;
// }
// 
// uint8_t EEWriteByte(uint16_t address,uint8_t data)
// {
// 	do
// 	{
// 		//Put Start Condition on TWI Bus
// 		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 
// 		//Check status
// 		if((TWSR & 0xF8) != 0x08)
// 			return FALSE;
// 
// 		//Now write SLA+W
// 		//EEPROM @ 00h
// 		TWDR=0b10100000;	
// 
// 		//Initiate Transfer
// 		TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 	
// 	}while((TWSR & 0xF8) != 0x18);
// 		
// 
// // 	//Now write ADDRH
// // 	TWDR=(address>>8);
// // 
// // 	//Initiate Transfer
// // 	TWCR=(1<<TWINT)|(1<<TWEN);
// // 
// // 	//Poll Till Done
// // 	while(!(TWCR & (1<<TWINT)));
// // 
// // 	//Check status
// // 	if((TWSR & 0xF8) != 0x28)
// // 		return FALSE;
// 
// 	//Now write ADDRL
// 	TWDR=(address);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 	//Now write DATA
// 	TWDR=(data);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Wait untill Writing is complete
// 	_delay_ms(12);
// 
// 	//Return TRUE
// 	return TRUE;
// 
// }



// uint8_t I2CWriteByte(uint8_t address,uint8_t data)
// {
// do
// 	{
// 		//Put Start Condition on TWI Bus
// 		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 
// 		//Check status
// 		if((TWSR & 0xF8) != 0x08)
// 			return FALSE;
// 
// 		//Now write SLA+W
// 		//TWDR=0b10100000;	
// 		//TWDR = address & 0b11111110; //clear LSB: write operation
// 		TWDR = EEPROM_write;
// 
// 		//Initiate Transfer
// 		TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 	
// 	}while((TWSR & 0xF8) != 0x18);	//ack received
// 	
// 	//Now write DATA
// 	TWDR=(data);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Wait untill Writing is complete
// 	_delay_ms(12);
// 
// 	//Return TRUE
// 	return TRUE;
// }
// uint8_t I2CReadByte(uint8_t address)	
// {
// 	uint8_t data;
// 	
// 	//Put Start Condition on TWI Bus
// 	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x10)
// 		return FALSE;
// 
// 	//Now write SLA+R
// 	//TWDR=0b10100001;	
// 	TWDR = address | 0x01;	//set LSB: read operation
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x40)
// 		return FALSE;
// 
// 	//Now enable Reception of data by clearing TWINT
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Wait till done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x58)
// 		return FALSE;
// 
// 	//Read the data
// 	data=TWDR;
// 
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Return TRUE
// 	return data;	
// }
// 
// uint8_t EEReadByte(uint8_t address)
// {
// 	
// 	/*	Start
// 		CTRL EEPROM_write
// 		->ack
// 		adress
// 		->ack
// 		Start
// 		CTRL EEPROM_read
// 		->receive byte
// 		->no ack
// 		Stop	
// 	*/
// 	uint8_t data;
// 
// 	do
// 	{
// 		//Put Start Condition on TWI Bus
// 		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 
// 		//Check status
// 		if((TWSR & 0xF8) != 0x08)
// 			return FALSE;
// 
// 		//Now write SLA+W
// 		//EEPROM @ 00h
// 		TWDR = SRAM_write;
// 		//TWDR = EEPROM_write;
// 		//TWDR=0b10100000;	
// 
// 		//Initiate Transfer
// 		TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 	
// 	}while((TWSR & 0xF8) != 0x18);
// 		
// 	//Now write ADDR
// 	TWDR=(address);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 	//Put Start Condition on TWI Bus
// 	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x10)
// 		return FALSE;
// 
// 	//Now write EEPROM+READ
// 	TWDR = SRAM_read;
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x40)
// 		return FALSE;
// 
// 	//Now enable Reception of data by clearing TWINT
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Wait till done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x58)
// 		return FALSE;
// 
// 	//Read the data
// 	data=TWDR;
// 
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Return TRUE
// 	return data;
// }
// uint8_t EEWriteByte(uint8_t address,uint8_t data)
// {
// 	do
// 	{
// 		//Put Start Condition on TWI Bus
// 		TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
// 		
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 
// 		//Check status
// 		if((TWSR & 0xF8) != 0x08)
// 			return FALSE;
// 
// 		//Now write SLA_EEPROM+WRITE
// 		//TWDR = EEPROM_write;
// 		TWDR = SRAM_write;
// 
// 		//Initiate Transfer
// 		TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 		//Poll Till Done
// 		while(!(TWCR & (1<<TWINT)));
// 	
// 	}while((TWSR & 0xF8) != 0x18);
// 		
// 
// 	//Now write ADDR
// 	TWDR=(address);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 	//Now write DATA
// 	TWDR=(data);
// 
// 	//Initiate Transfer
// 	TWCR=(1<<TWINT)|(1<<TWEN);
// 
// 	//Poll Till Done
// 	while(!(TWCR & (1<<TWINT)));
// 
// 	//Check status
// 	if((TWSR & 0xF8) != 0x28)
// 		return FALSE;
// 
// 	//Put Stop Condition on bus
// 	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
// 	
// 	//Wait for STOP to finish
// 	while(TWCR & (1<<TWSTO));
// 
// 	//Wait untill Writing is complete
// 	_delay_ms(12);
// 
// 	//Return TRUE
// 	return TRUE;
// 
// }

#endif /* I2C_BASIC_H_ */